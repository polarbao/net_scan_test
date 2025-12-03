#include "ImagePreviewWidget.h"
#include <QPaintEvent>
#include <QPainter>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QFileInfo>
#include <QDebug>

ImagePreviewWidget::ImagePreviewWidget(QWidget *parent)
    : QWidget(parent)
    , m_loading(false)
    , m_failedLoad(false)
    , m_bkColor(palette().color(QPalette::Base))
{
    setMinimumSize(200, 200);
    setMouseTracking(true);
}

ImagePreviewWidget::~ImagePreviewWidget()
{
}

void ImagePreviewWidget::setPixmap(const QPixmap &pixmap)
{
    m_pixmap = pixmap;
    m_loading = false;
    m_failedLoad = false;
    updatePixmapRect();
    update();
}

void ImagePreviewWidget::showPreview(const QString &filePath)
{
    if (filePath.isEmpty()) {
        clear();
        return;
    }
    
    // 如果文件路径相同，不重复加载
    if (m_ripFile == filePath) {
        return;
    }
    
    m_ripFile = filePath;
    m_loading = true;
    m_failedLoad = false;
    update();
    
    // 生成预览
    generatePreview(filePath);
}

void ImagePreviewWidget::clear()
{
    m_pixmap = QPixmap();
    m_ripFile.clear();
    m_previewFile.clear();
    m_loading = false;
    m_failedLoad = false;
    update();
}

void ImagePreviewWidget::setBkColor(const QColor &color)
{
    m_bkColor = color;
    update();
}

void ImagePreviewWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // 填充背景
    painter.fillRect(rect(), m_bkColor);
    
    if (m_loading) {
        painter.drawText(rect(), Qt::AlignCenter, "Loading...");
        return;
    }
    
    if (m_failedLoad) {
        painter.drawText(rect(), Qt::AlignCenter, "Failed to load image");
        return;
    }
    
    if (m_pixmap.isNull()) {
        return;
    }
    
    // 绘制图像
    drawPixmap(painter);
    
    // 绘制边框
    painter.setPen(QPen(palette().color(QPalette::Dark), 1));
    painter.drawRect(rect().adjusted(0, 0, -1, -1));
}

void ImagePreviewWidget::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event)
    updatePixmapRect();
    update();
}

void ImagePreviewWidget::mouseMoveEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
    // 可以添加鼠标移动事件处理
    QWidget::mouseMoveEvent(event);
}

void ImagePreviewWidget::updatePixmapRect()
{
    if (m_pixmap.isNull()) {
        m_pixmapRect = QRect();
        return;
    }
    
    QSize widgetSize = size();
    QSize pixmapSize = m_pixmap.size();
    
    // 计算缩放比例（保持宽高比）
    qreal scaleX = static_cast<qreal>(widgetSize.width()) / pixmapSize.width();
    qreal scaleY = static_cast<qreal>(widgetSize.height()) / pixmapSize.height();
    qreal scale = qMin(scaleX, scaleY);
    
    QSize scaledSize = pixmapSize * scale;
    QPoint topLeft((widgetSize.width() - scaledSize.width()) / 2,
                   (widgetSize.height() - scaledSize.height()) / 2);
    
    m_pixmapRect = QRect(topLeft, scaledSize);
}

void ImagePreviewWidget::drawPixmap(QPainter &painter)
{
    if (m_pixmapRect.isValid()) {
        painter.setRenderHint(QPainter::SmoothPixmapTransform);
        painter.drawPixmap(m_pixmapRect, m_pixmap);
    }
}

void ImagePreviewWidget::generatePreview(const QString &filePath)
{
    // 根据文件扩展名判断格式
    QFileInfo fileInfo(filePath);
    QString suffix = fileInfo.suffix().toLower();
    
    if (suffix == "bmp") {
        // 直接加载BMP
        QPixmap pixmap;
        if (pixmap.load(filePath)) {
            m_pixmap = pixmap;
            m_loading = false;
            m_failedLoad = false;
            updatePixmapRect();
            update();
            emit previewGenerated(filePath);
        } else {
            m_loading = false;
            m_failedLoad = true;
            update();
        }
    } else if (suffix == "prt" || suffix == "cli") {
        // PRT和CLI格式需要特殊处理
        // 这里可以调用相应的解析函数
        // 暂时标记为加载失败
        m_loading = false;
        m_failedLoad = true;
        update();
    } else {
        // 尝试直接加载
        QPixmap pixmap;
        if (pixmap.load(filePath)) {
            m_pixmap = pixmap;
            m_loading = false;
            m_failedLoad = false;
            updatePixmapRect();
            update();
            emit previewGenerated(filePath);
        } else {
            m_loading = false;
            m_failedLoad = true;
            update();
        }
    }
}

