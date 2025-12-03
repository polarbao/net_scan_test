#ifndef IMAGEPREVIEWWIDGET_H
#define IMAGEPREVIEWWIDGET_H

#include <QWidget>
#include <QPixmap>
#include <QString>

/**
 * @brief 图像预览控件
 * 
 * 对应MFC版本的CPicScreen
 * 负责图像预览显示和位图绘制
 */
class ImagePreviewWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ImagePreviewWidget(QWidget *parent = nullptr);
    ~ImagePreviewWidget();
    
    void showPreview(const QString &filePath);
    void setPixmap(const QPixmap &pixmap);
    void clear();
    void setBkColor(const QColor &color);

signals:
    void previewGenerated(const QString &filePath);

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    QPixmap m_pixmap;
    QRect m_pixmapRect;
    bool m_loading;
    bool m_failedLoad;
    QColor m_bkColor;
    QString m_ripFile;
    QString m_previewFile;
    
    void updatePixmapRect();
    void drawPixmap(QPainter &painter);
    void generatePreview(const QString &filePath);
};

#endif // IMAGEPREVIEWWIDGET_H

