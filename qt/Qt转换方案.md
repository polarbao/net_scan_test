# NetScanDemo MFC 到 Qt 转换方案

## 一、转换概述

### 1.1 转换目标
将基于 MFC 的 NetScanDemo 项目转换为基于 Qt 框架的项目，保持原有功能不变，提升代码可维护性和跨平台能力。

### 1.2 转换原则
1. **功能等价**: 保持所有原有功能
2. **架构优化**: 利用Qt特性优化代码结构
3. **渐进式迁移**: 可以分模块逐步迁移
4. **兼容性**: 保持与RYPrtCtler库的兼容性

### 1.3 转换难点
1. 消息映射 → 信号槽
2. DDX/DDV → Qt属性系统或手动同步
3. MFC控件 → Qt控件
4. 线程模型转换
5. 文件操作转换
6. 自定义控件重写

---

## 二、核心模块转换方案

### 2.1 应用程序类转换

#### MFC 方式
```cpp
// NetScanDemo.h
class CNetScanDemoApp : public CWinApp
{
public:
    virtual BOOL InitInstance();
    CString m_szAppPath;
};

// NetScanDemo.cpp
CNetScanDemoApp theApp;  // 全局对象

BOOL CNetScanDemoApp::InitInstance()
{
    // 初始化代码
    CNetScanDemoDlg dlg;
    dlg.DoModal();
    return FALSE;
}
```

#### Qt 方式
```cpp
// main.cpp
#include <QApplication>
#include "MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // 设置应用程序信息
    app.setApplicationName("NetScanDemo");
    app.setOrganizationName("RongYue");
    
    MainWindow window;
    window.show();
    
    return app.exec();
}
```

**转换要点**:
- `CWinApp` → `QApplication`
- `InitInstance()` → `main()` 函数
- 全局对象 `theApp` → `QApplication` 实例
- `DoModal()` → `show()` + `exec()`

---

### 2.2 主对话框转换

#### MFC 方式
```cpp
// NetScanDemoDlg.h
class CNetScanDemoDlg : public CDialogEx
{
    DECLARE_MESSAGE_MAP()
    enum { IDD = IDD_NETSCANDEMO_DIALOG };
    
protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    virtual BOOL OnInitDialog();
    
    afx_msg void OnBnClickedButtonStartjob();
    
    CSWListTreeCtrl m_ListPrtInfo;
    CPicScreen m_ImgPreView;
    CString m_szPrtFile;
};

// NetScanDemoDlg.cpp
BEGIN_MESSAGE_MAP(CNetScanDemoDlg, CDialogEx)
    ON_BN_CLICKED(IDC_BUTTON_STARTJOB, &CNetScanDemoDlg::OnBnClickedButtonStartjob)
END_MESSAGE_MAP()

void CNetScanDemoDlg::DoDataExchange(CDataExchange* pDX)
{
    DDX_Control(pDX, IDC_STATIC_PRT_INFO, m_ListPrtInfo);
    DDX_Text(pDX, IDC_MFC_PRT_FILE_PATH, m_szPrtFile);
}
```

#### Qt 方式
```cpp
// MainWindow.h
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include "ui_MainWindow.h"

class PrintInfoTreeWidget;
class ImagePreviewWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onButtonStartJobClicked();
    void onButtonLoadImageClicked();
    void onFilePathChanged();

private:
    void initSystem();
    void updateButtonState();
    void updatePrtState(uint state);
    
    Ui::MainWindow *ui;
    PrintInfoTreeWidget *m_printInfoTree;
    ImagePreviewWidget *m_imagePreview;
    QString m_prtFilePath;
    bool m_jobStarted;
    bool m_printing;
};

#endif // MAINWINDOW_H
```

```cpp
// MainWindow.cpp
#include "MainWindow.h"
#include "PrintInfoTreeWidget.h"
#include "ImagePreviewWidget.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_jobStarted(false)
    , m_printing(false)
{
    ui->setupUi(this);
    
    // 初始化控件
    m_printInfoTree = new PrintInfoTreeWidget(this);
    m_imagePreview = new ImagePreviewWidget(this);
    
    // 布局控件
    ui->verticalLayout->addWidget(m_printInfoTree);
    ui->verticalLayout->addWidget(m_imagePreview);
    
    // 连接信号槽
    connect(ui->buttonStartJob, &QPushButton::clicked, 
            this, &MainWindow::onButtonStartJobClicked);
    connect(ui->buttonLoadImage, &QPushButton::clicked,
            this, &MainWindow::onButtonLoadImageClicked);
    connect(ui->lineEditFilePath, &QLineEdit::textChanged,
            this, &MainWindow::onFilePathChanged);
    
    // 初始化系统
    initSystem();
}

void MainWindow::onButtonStartJobClicked()
{
    if (!m_jobStarted) {
        // 启动打印作业
        m_jobStarted = true;
        ui->buttonStartJob->setText("停止打印");
        // 启动打印线程
    } else {
        // 停止打印作业
        m_jobStarted = false;
        ui->buttonStartJob->setText("开始打印");
    }
}
```

**转换要点**:
- `CDialogEx` → `QMainWindow` 或 `QDialog`
- 消息映射 → 信号槽连接
- `DoDataExchange()` → 手动同步或使用Qt属性系统
- `OnInitDialog()` → 构造函数
- `CString` → `QString`
- 控件ID访问 → `ui->控件名` 访问

---

### 2.3 参数配置对话框转换

#### MFC 方式
```cpp
// CPrtJobDlg.h
class CPrtJobDlg : public CDialogEx
{
    DECLARE_MESSAGE_MAP()
    enum { IDD = IDD_PRT_JOB_SET };
    
protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    
public:
    float m_fXOutDPI;
    float m_fYOutDPI;
    float m_fClipWidth;
    float m_fClipHeight;
    afx_msg void OnBnClickedOk();
};
```

#### Qt 方式
```cpp
// PrintJobDialog.h
#ifndef PRINTJOBDIALOG_H
#define PRINTJOBDIALOG_H

#include <QDialog>
#include "ui_PrintJobDialog.h"

class PrintJobDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PrintJobDialog(QWidget *parent = nullptr);
    ~PrintJobDialog();
    
    float xOutDPI() const { return m_xOutDPI; }
    float yOutDPI() const { return m_yOutDPI; }
    float clipWidth() const { return m_clipWidth; }
    float clipHeight() const { return m_clipHeight; }
    
    void setXOutDPI(float dpi) { m_xOutDPI = dpi; }
    void setYOutDPI(float dpi) { m_yOutDPI = dpi; }
    void setClipWidth(float width) { m_clipWidth = width; }
    void setClipHeight(float height) { m_clipHeight = height; }

private slots:
    void onAccepted();

private:
    Ui::PrintJobDialog *ui;
    float m_xOutDPI;
    float m_yOutDPI;
    float m_clipWidth;
    float m_clipHeight;
};

#endif // PRINTJOBDIALOG_H
```

```cpp
// PrintJobDialog.cpp
#include "PrintJobDialog.h"

PrintJobDialog::PrintJobDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::PrintJobDialog)
    , m_xOutDPI(600.0f)
    , m_yOutDPI(600.0f)
    , m_clipWidth(0.0f)
    , m_clipHeight(0.0f)
{
    ui->setupUi(this);
    
    // 初始化控件值
    ui->spinBoxXDPI->setValue(m_xOutDPI);
    ui->spinBoxYDPI->setValue(m_yOutDPI);
    ui->spinBoxClipWidth->setValue(m_clipWidth);
    ui->spinBoxClipHeight->setValue(m_clipHeight);
    
    connect(ui->buttonBox, &QDialogButtonBox::accepted, 
            this, &PrintJobDialog::onAccepted);
}

void PrintJobDialog::onAccepted()
{
    // 从控件读取数据
    m_xOutDPI = ui->spinBoxXDPI->value();
    m_yOutDPI = ui->spinBoxYDPI->value();
    m_clipWidth = ui->spinBoxClipWidth->value();
    m_clipHeight = ui->spinBoxClipHeight->value();
    
    accept();
}
```

**转换要点**:
- `DoDataExchange()` → 手动在 `accept()` 时读取数据
- 成员变量 → 私有成员变量 + getter/setter
- `OnBnClickedOk()` → `onAccepted()` 槽函数

---

### 2.4 线程管理转换

#### MFC 方式
```cpp
// 静态线程函数
static UINT PrintThread(LPVOID pvoid)
{
    CNetScanDemoDlg* pDlg = (CNetScanDemoDlg*)pvoid;
    // 工作代码
    return 0;
}

// 启动线程
void CNetScanDemoDlg::OnBnClickedButtonStartjob()
{
    g_PrtMutex = CreateMutex(NULL, FALSE, "Mutex");
    AfxBeginThread(PrintThread, this);
}
```

#### Qt 方式
```cpp
// PrintThread.h
#ifndef PRINTTHREAD_H
#define PRINTTHREAD_H

#include <QThread>
#include <QMutex>

class PrintThread : public QThread
{
    Q_OBJECT

public:
    explicit PrintThread(QObject *parent = nullptr);
    ~PrintThread();
    
    void stop();

signals:
    void printStateChanged(uint state);
    void printProgress(int progress);
    void printFinished();
    void errorOccurred(const QString &error);

protected:
    void run() override;

private:
    bool m_stop;
    QMutex m_mutex;
};

#endif // PRINTTHREAD_H
```

```cpp
// PrintThread.cpp
#include "PrintThread.h"
#include "../Inc/RYPrtCtler.h"

PrintThread::PrintThread(QObject *parent)
    : QThread(parent)
    , m_stop(false)
{
}

PrintThread::~PrintThread()
{
    stop();
    wait();
}

void PrintThread::stop()
{
    QMutexLocker locker(&m_mutex);
    m_stop = true;
}

void PrintThread::run()
{
    PrtRunInfo prtInfo;
    int nPassCount;
    
    m_mutex.lock();
    bool shouldStop = m_stop;
    m_mutex.unlock();
    
    while (!shouldStop) {
        // 等待缓冲区就绪
        while (PrtBuffNum <= 0 && !shouldStop) {
            msleep(10);
            m_mutex.lock();
            shouldStop = m_stop;
            m_mutex.unlock();
        }
        
        if (shouldStop) break;
        
        // 启动图层打印
        nPassCount = IDP_StartLayerPrint(0);
        
        if (nPassCount > 0) {
            // 处理PASS循环
            for (int i = 0; i < nPassCount; i++) {
                LPPassDataItem pPrtPassDes = IDP_GetPassItem(0, i);
                if (pPrtPassDes) {
                    if (IDP_DoPassPrint(pPrtPassDes) < 0) {
                        emit errorOccurred("打印失败");
                        break;
                    }
                    
                    // 监控打印状态
                    do {
                        IDP_GetPrintState(&prtInfo);
                        emit printStateChanged(prtInfo.nPrtState);
                        msleep(1);
                    } while (prtInfo.nPrtState == 1 && !shouldStop);
                }
            }
        }
        
        m_mutex.lock();
        shouldStop = m_stop;
        m_mutex.unlock();
    }
    
    emit printFinished();
}
```

**使用方式**:
```cpp
// MainWindow.cpp
void MainWindow::onButtonStartJobClicked()
{
    if (!m_jobStarted) {
        m_printThread = new PrintThread(this);
        connect(m_printThread, &PrintThread::printStateChanged,
                this, &MainWindow::onPrintStateChanged);
        connect(m_printThread, &PrintThread::printFinished,
                this, &MainWindow::onPrintFinished);
        connect(m_printThread, &PrintThread::errorOccurred,
                this, &MainWindow::onPrintError);
        
        m_printThread->start();
        m_jobStarted = true;
    } else {
        if (m_printThread) {
            m_printThread->stop();
            m_printThread->wait();
            delete m_printThread;
            m_printThread = nullptr;
        }
        m_jobStarted = false;
    }
}
```

**转换要点**:
- 静态线程函数 → `QThread` 子类
- `AfxBeginThread()` → `QThread::start()`
- `CreateMutex()` → `QMutex`
- `WaitForSingleObject()` → `QMutex::lock()`
- `ReleaseMutex()` → `QMutex::unlock()`
- Windows消息通信 → Qt信号槽

---

### 2.5 文件操作转换

#### MFC 方式
```cpp
int CNetScanDemoApp::SaveParam(LPRYUSR_PARAM lparam)
{
    CFile file;
    CString szParamName = m_szAppPath + _T("\\userparam.dat");
    if (!file.Open(szParamName, CFile::modeCreate | CFile::modeWrite))
        return -1;
    file.Write(&g_sysParam, sizeof(RYUSR_PARAM));
    file.Close();
    return 0;
}
```

#### Qt 方式
```cpp
// ParameterManager.h
#ifndef PARAMETERMANAGER_H
#define PARAMETERMANAGER_H

#include <QObject>
#include <QString>
#include "../Inc/RYPrtCtler.h"

class ParameterManager : public QObject
{
    Q_OBJECT

public:
    explicit ParameterManager(QObject *parent = nullptr);
    
    bool saveUserParam(const RYUSR_PARAM &param);
    bool loadUserParam(RYUSR_PARAM &param);
    
    bool saveLayerParam(const PRTIMG_LAYER &layer);
    bool loadLayerParam(PRTIMG_LAYER &layer);
    
    bool saveJobParam(const PRTJOB_ITEM &job);
    bool loadJobParam(PRTJOB_ITEM &job);
    
    QString applicationPath() const { return m_appPath; }

private:
    QString m_appPath;
    QString getParamFilePath(const QString &filename) const;
};

#endif // PARAMETERMANAGER_H
```

```cpp
// ParameterManager.cpp
#include "ParameterManager.h"
#include <QApplication>
#include <QDir>
#include <QFile>
#include <QDataStream>

ParameterManager::ParameterManager(QObject *parent)
    : QObject(parent)
{
    // 获取应用程序路径
    m_appPath = QApplication::applicationDirPath();
}

QString ParameterManager::getParamFilePath(const QString &filename) const
{
    return QDir(m_appPath).filePath(filename);
}

bool ParameterManager::saveUserParam(const RYUSR_PARAM &param)
{
    QString filePath = getParamFilePath("userparam.dat");
    QFile file(filePath);
    
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }
    
    QDataStream out(&file);
    out.writeRawData(reinterpret_cast<const char*>(&param), sizeof(RYUSR_PARAM));
    
    file.close();
    return true;
}

bool ParameterManager::loadUserParam(RYUSR_PARAM &param)
{
    QString filePath = getParamFilePath("userparam.dat");
    QFile file(filePath);
    
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }
    
    QDataStream in(&file);
    in.readRawData(reinterpret_cast<char*>(&param), sizeof(RYUSR_PARAM));
    
    file.close();
    return true;
}
```

**转换要点**:
- `CFile` → `QFile`
- `CString` → `QString`
- `CFile::Write()` → `QDataStream::writeRawData()`
- `CFile::Read()` → `QDataStream::readRawData()`
- `GetModuleFileName()` → `QApplication::applicationDirPath()`

---

### 2.6 自定义控件转换

#### MFC 方式 - CPicScreen
```cpp
// PicScreen.h
class CPicScreen : public CStatic
{
    DECLARE_MESSAGE_MAP()
protected:
    afx_msg void OnPaint();
    HBITMAP m_hBmp;
};
```

#### Qt 方式
```cpp
// ImagePreviewWidget.h
#ifndef IMAGEPREVIEWWIDGET_H
#define IMAGEPREVIEWWIDGET_H

#include <QWidget>
#include <QPixmap>

class ImagePreviewWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ImagePreviewWidget(QWidget *parent = nullptr);
    ~ImagePreviewWidget();
    
    void showPreview(const QString &filePath);
    void setPixmap(const QPixmap &pixmap);
    void clear();

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    QPixmap m_pixmap;
    QRect m_pixmapRect;
    bool m_loading;
    
    void updatePixmapRect();
    void drawPixmap(QPainter &painter);
};

#endif // IMAGEPREVIEWWIDGET_H
```

```cpp
// ImagePreviewWidget.cpp
#include "ImagePreviewWidget.h"
#include <QPaintEvent>
#include <QPainter>
#include <QResizeEvent>

ImagePreviewWidget::ImagePreviewWidget(QWidget *parent)
    : QWidget(parent)
    , m_loading(false)
{
    setMinimumSize(200, 200);
}

ImagePreviewWidget::~ImagePreviewWidget()
{
}

void ImagePreviewWidget::setPixmap(const QPixmap &pixmap)
{
    m_pixmap = pixmap;
    updatePixmapRect();
    update();
}

void ImagePreviewWidget::showPreview(const QString &filePath)
{
    m_loading = true;
    update();
    
    // 异步加载图像
    QPixmap pixmap;
    if (pixmap.load(filePath)) {
        m_pixmap = pixmap;
        m_loading = false;
        updatePixmapRect();
        update();
    } else {
        m_loading = false;
        update();
    }
}

void ImagePreviewWidget::clear()
{
    m_pixmap = QPixmap();
    update();
}

void ImagePreviewWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // 填充背景
    painter.fillRect(rect(), palette().color(QPalette::Base));
    
    if (m_loading) {
        painter.drawText(rect(), Qt::AlignCenter, "Loading...");
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
```

**转换要点**:
- `CStatic` → `QWidget`
- `OnPaint()` → `paintEvent()`
- `CPaintDC` → `QPainter`
- `HBITMAP` → `QPixmap`
- `StretchBlt()` → `QPainter::drawPixmap()`
- `GetClientRect()` → `rect()`

---

## 三、转换步骤

### 3.1 准备阶段
1. 安装Qt开发环境（Qt 5.15+ 或 Qt 6.x）
2. 创建Qt项目结构
3. 准备UI文件（.ui文件）

### 3.2 核心模块转换
1. **应用程序类** → `main.cpp`
2. **主对话框** → `MainWindow`
3. **参数管理** → `ParameterManager`
4. **线程管理** → `PrintThread`, `MonitorThread`
5. **自定义控件** → Qt Widget子类

### 3.3 UI设计
1. 使用Qt Designer设计界面
2. 转换资源文件（.rc → .qrc）
3. 调整布局和样式

### 3.4 功能实现
1. 实现信号槽连接
2. 实现数据同步
3. 实现线程通信
4. 实现文件操作

### 3.5 测试和优化
1. 功能测试
2. 性能优化
3. 跨平台测试（如需要）

---

## 四、注意事项

### 4.1 兼容性
- RYPrtCtler库是Windows专用，需要保持Windows平台
- 某些Windows API调用需要保留或使用Qt的Windows特定功能

### 4.2 线程安全
- Qt的信号槽是线程安全的
- 使用 `QMetaObject::invokeMethod()` 进行跨线程调用
- 避免在工作线程中直接操作UI

### 4.3 内存管理
- Qt使用父子对象自动管理内存
- 使用智能指针（`QSharedPointer`, `QScopedPointer`）
- 注意避免循环引用

### 4.4 字符串编码
- Qt使用UTF-8编码
- 与Windows API交互时注意编码转换
- 使用 `QString::toLocal8Bit()` 或 `QString::toUtf8()`

---

## 五、项目结构

```
NetScanDemoQt/
├── CMakeLists.txt          # CMake构建文件
├── NetScanDemoQt.pro      # qmake项目文件（可选）
├── main.cpp               # 应用程序入口
├── MainWindow.h/cpp       # 主窗口
├── PrintJobDialog.h/cpp   # 打印作业对话框
├── SysParamDialog.h/cpp   # 系统参数对话框
├── ParameterManager.h/cpp # 参数管理
├── PrintThread.h/cpp      # 打印线程
├── MonitorThread.h/cpp    # 监控线程
├── widgets/               # 自定义控件
│   ├── ImagePreviewWidget.h/cpp
│   ├── PrintInfoTreeWidget.h/cpp
│   └── ...
├── ui/                    # UI文件
│   ├── MainWindow.ui
│   ├── PrintJobDialog.ui
│   └── ...
├── resources/             # 资源文件
│   ├── images/
│   └── resources.qrc
└── Inc/                   # 外部库头文件
    └── RYPrtCtler.h
```

---

## 六、总结

### 6.1 转换优势
1. **跨平台能力**: Qt支持多平台（虽然本项目受限于Windows库）
2. **现代化**: 使用现代C++特性和Qt框架
3. **可维护性**: 代码结构更清晰，易于维护
4. **扩展性**: 更容易添加新功能和模块

### 6.2 转换挑战
1. **学习曲线**: 需要学习Qt框架
2. **工作量**: 需要重写大部分代码
3. **兼容性**: 需要保持与现有库的兼容
4. **测试**: 需要全面测试确保功能一致

### 6.3 建议
1. **渐进式迁移**: 可以先转换核心模块，逐步迁移
2. **保持接口**: 保持与RYPrtCtler库的接口不变
3. **充分测试**: 每个模块转换后都要充分测试
4. **文档更新**: 及时更新项目文档

