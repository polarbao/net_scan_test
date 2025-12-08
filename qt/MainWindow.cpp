#include "MainWindow.h"
#include "PrintInfoTreeWidget.h"
#include "ImagePreviewWidget.h"
#include "PrintThread.h"
#include "MonitorThread.h"
#include "ParameterManager.h"
#include "PrintJobDialog.h"
#include "SysParamDialog.h"
#include "VTSetDialog.h"
#include "ReloadWaveDialog.h"
#include "MoveCtlDialog.h"
#include "AirInkDialog.h"
#include "AdjParamDialog.h"
#include "AdibCtrlDialog.h"
#include "ImgLayerSetDialog.h"
#include "../Inc/RYPrtCtler.h"
#include <QElapsedTimer>
#include <QThread>
#include <QMessageBox>
#include <QFileDialog>
#include <QApplication>
#include <QDir>
#include <QDebug>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <windows.h> // For BITMAPFILEHEADER etc.

// 外部全局变量声明（在main.cpp中定义）
extern RYUSR_PARAM g_sysParam;
extern PRTJOB_ITEM g_testJob;
extern PRTIMG_LAYER g_PrtImgLayer;
extern RYCalbrationParam g_Calbration;
extern MOV_Config g_movConfig;
extern LPPRINTER_INFO g_pSysInfo;
extern bool g_bPHValid[MAX_PH_CNT];
extern HANDLE g_PrtMutex;
extern UINT g_nPHType;
extern bool g_IsRoladWave;
extern int PrtBuffNum;
extern QByteArray g_prtData[MAX_COLORS]; // Global image data buffer
extern int g_prtDataSize; // Global image data size

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_printThread(nullptr)
    , m_monitorThread(nullptr)
    , m_paramManager(new ParameterManager(this))
    , m_jobStarted(false)
    , m_printing(false)
    , m_imageLoaded(false)
    , m_stopMonitor(false)
    , m_enableInkWatch(false)
    , m_closeAutoCheck(false)
    , m_jobImgLayerCounts(1)
    , m_prtRevColumn(0)
    , m_prtEncoderVal(0)
{
    ui->setupUi(this);
    
    // 初始化全局参数
    InitImgLayerInfo();
    InitJobInfo();
    InitCalibrationInfo();
    
    // 创建自定义控件
    m_printInfoTree = new PrintInfoTreeWidget(this);
    m_drvInfoTree = new PrintInfoTreeWidget(this);
    m_imagePreview = new ImagePreviewWidget(this);
    
    // 布局控件（如果UI文件中没有布局，则手动创建）
    // 注意：实际使用时应该在UI文件中设置布局
    // 这里只是示例代码
    if (ui->verticalLayoutPrintInfo) {
        ui->verticalLayoutPrintInfo->addWidget(m_printInfoTree);
    }
    if (ui->verticalLayoutDrvInfo) {
        ui->verticalLayoutDrvInfo->addWidget(m_drvInfoTree);
    }
    if (ui->verticalLayoutPreview) {
        ui->verticalLayoutPreview->addWidget(m_imagePreview);
    }
    
    // 连接信号槽
    connect(ui->buttonStartJob, &QPushButton::clicked,
            this, &MainWindow::onButtonStartJobClicked);
    connect(ui->buttonLoadImage, &QPushButton::clicked,
            this, &MainWindow::onButtonLoadImageClicked);
    connect(ui->buttonReloadWave, &QPushButton::clicked,
            this, &MainWindow::onButtonReloadWaveClicked);
    connect(ui->buttonVTSet, &QPushButton::clicked,
            this, &MainWindow::onButtonVTSetClicked);
    connect(ui->buttonCali, &QPushButton::clicked,
            this, &MainWindow::onButtonCaliClicked);
    connect(ui->buttonSysParam, &QPushButton::clicked,
            this, &MainWindow::onButtonSysParamClicked);
    connect(ui->buttonFlash, &QPushButton::clicked,
            this, &MainWindow::onButtonFlashClicked);
    connect(ui->buttonResetEncoder, &QPushButton::clicked,
            this, &MainWindow::onButtonResetEncoderClicked);
    connect(ui->buttonAdibCtl, &QPushButton::clicked,
            this, &MainWindow::onButtonAdibCtlClicked);
    
    connect(ui->checkBoxInkWatch, &QCheckBox::toggled,
            this, &MainWindow::onCheckInkWatchToggled);
    connect(ui->checkBoxCloseAutoCheck, &QCheckBox::toggled,
            this, &MainWindow::onCheckCloseAutoCheckToggled);
    
    connect(ui->lineEditFilePath, &QLineEdit::textChanged,
            this, &MainWindow::onFilePathChanged);
    
    // 初始化系统
    initSystem();
    
    // 启动监控线程
    m_monitorThread = new MonitorThread(&m_printMutex, this);
    m_monitorThread->setCloseAutoCheck(m_closeAutoCheck);
    connect(m_monitorThread, &MonitorThread::dataUpdated,
            this, &MainWindow::onMonitorDataUpdated);
    connect(m_monitorThread, &MonitorThread::deviceConnectedChanged,
            this, [this](bool connected) {
                // 设备连接状态变化处理
                if (!connected) {
                    QMessageBox::warning(this, "警告", "设备已断开连接！");
                }
            });
    connect(m_monitorThread, &MonitorThread::errorOccurred,
            this, [this](const QString &error) {
                QMessageBox::critical(this, "错误", error);
            });
    m_monitorThread->start();
}

MainWindow::~MainWindow()
{
    // Stop threads gracefully
    if (m_monitorThread) {
        m_monitorThread->stop();
    }
    if (m_printThread) {
        // Stop any hardware-level printing first
        IDP_StopPrintJob();
        m_printThread->stop();
    }

    // Wait for threads to finish
    // The monitor thread is a child of MainWindow, so it will be deleted automatically.
    // We still wait for it to ensure it finishes before the mutex is destroyed.
    if (m_monitorThread && !m_monitorThread->wait(5000)) {
        qWarning() << "MonitorThread did not finish in 5 seconds.";
    }
    if (m_printThread && !m_printThread->wait(5000)) {
        qWarning() << "PrintThread did not finish in 5 seconds.";
    }
    
    // m_printThread is managed manually, so delete it.
    delete m_printThread;
    m_printThread = nullptr;

    // Save parameters
    if (m_paramManager) {
        m_paramManager->saveUserParam(g_sysParam);
        m_paramManager->saveCalibrationParam(g_Calbration);
#ifdef RY_MOVE_CTL
        m_paramManager->saveMoveParam(g_movConfig);
#endif
    }
    
    delete ui;
}

void MainWindow::initSystem()
{
    // 加载参数
    if (m_paramManager) {
        m_paramManager->loadUserParam(g_sysParam);
        m_paramManager->loadLayerParam(g_PrtImgLayer);
        m_paramManager->loadJobParam(g_testJob);
        m_paramManager->loadCalibrationParam(g_Calbration);
#ifdef RY_MOVE_CTL
        if (!m_paramManager->loadMoveParam(g_movConfig)) {
            // Optionally initialize g_movConfig with defaults if loading fails
            // For now, just log a warning or show a message
            qWarning() << "Failed to load move parameters. Using default/uninitialized values.";
        }
#endif
    }

    // 应用程序路径
    QString appPath = QApplication::applicationDirPath();
    QByteArray appPathBytes = appPath.toLocal8Bit();

    // 禁用窗口，防止用户操作
    this->setEnabled(false);

    // 打开设备
    DEV_OpenDevice(reinterpret_cast<HWND>(winId()), (unsigned char*)appPathBytes.constData());

    QElapsedTimer timer;
    timer.start();
    const int TIMEOUT_MS = 10 * 1000; // 10 seconds timeout

    bool connected = false;
    while (!DEV_DeviceIsConnected()) {
        QThread::msleep(500); // Wait for 500ms
        if (timer.elapsed() >= TIMEOUT_MS) {
            QMessageBox::critical(this, tr("错误"), tr("设备连接超时，请检查设备连接！"));
            this->close(); // Close application on connection failure
            return;
        }
        // Process events to keep UI responsive (optional, as it's blocking anyway)
        QCoreApplication::processEvents();
    }
    connected = true; // Device connected

    // 更新系统参数
    DEV_UpdateParam(&g_sysParam);

    // 初始化设备
    int nResult = DEV_InitDevice(ZERO_POSITION);
    if (nResult < 0) {
        QMessageBox::critical(this, tr("错误"), tr("初始化设备失败，错误码：%1").arg(nResult));
        this->close(); // Close application on initialization failure
        return;
    } else if (nResult > 0) {
        QMessageBox::information(this, tr("成功"), tr("设备初始化成功！"));
    }
    
    // 重新启用窗口
    this->setEnabled(true);

    // 初始化设备信息
    initDeviceInfo();
    
    // 移除 initSysParam()，因为DEV_UpdateParam已使用g_sysParam
    // initSysParam();
    
    // 更新按钮状态
    updateButtonState();
}

void MainWindow::initDeviceInfo()
{
    // 初始化设备信息
    g_pSysInfo = DEV_GetDeviceInfo();
    if (g_pSysInfo) {
        if (m_printInfoTree) {
            m_printInfoTree->updateDeviceInfo();
        }
        if (m_drvInfoTree) {
            m_drvInfoTree->updateVTListInfo();
        }
    }
}

void MainWindow::initSysParam()
{
    // 初始化系统参数
    memset(&g_sysParam, 0, sizeof(RYUSR_PARAM));
}

void MainWindow::initMoveSys()
{
    // 初始化运动系统（如果启用）
#ifdef RY_MOVE_CTL
    // 运动系统初始化代码
#endif
}

void MainWindow::updateButtonState()
{
    // 更新按钮状态
    ui->buttonStartJob->setEnabled(m_imageLoaded);
    if (m_jobStarted) {
        ui->buttonStartJob->setText("停止打印");
    } else {
        ui->buttonStartJob->setText("开始打印");
    }
}

void MainWindow::updatePrtState(uint state)
{
    // 更新打印状态显示
    QString stateText;
    switch (state) {
    case 0:
        stateText = "空闲";
        break;
    case 1:
        stateText = "打印中";
        break;
    case 2:
        stateText = "暂停";
        break;
    case 3:
        stateText = "完成";
        break;
    default:
        stateText = "未知";
        break;
    }
    
    ui->labelPrintState->setText(QString("打印状态: %1").arg(stateText));
}



void MainWindow::onButtonStartJobClicked()
{
    if (!m_imageLoaded) {
        QMessageBox::warning(this, "警告", "请先加载图像文件！");
        return;
    }
    
    if (!m_jobStarted) {
        // 弹出作业配置对话框
        PrintJobDialog dlg(this);
        if (dlg.exec() == QDialog::Accepted) {
            // 获取作业参数
            g_testJob.fOutXdpi = dlg.xOutDPI();
            g_testJob.fOutYdpi = dlg.yOutDPI();
            g_testJob.fClipWidth = dlg.clipWidth();
            g_testJob.fClipHeight = dlg.clipHeight();
            // ... 其他参数
            
            // 如果已有打印线程，先停止
            if (m_printThread) {
                m_printThread->stop();
                m_printThread->wait(3000);
                delete m_printThread;
                m_printThread = nullptr;
            }
            
            // 创建打印线程，传递图层数量和共享互斥锁
            m_printThread = new PrintThread(m_jobImgLayerCounts, &m_printMutex, this);
            connect(m_printThread, &PrintThread::printStateChanged,
                    this, &MainWindow::onPrintStateChanged);
            connect(m_printThread, &PrintThread::printProgress,
                    this, &MainWindow::onPrintProgress);
            connect(m_printThread, &PrintThread::printFinished,
                    this, &MainWindow::onPrintFinished);
            connect(m_printThread, &PrintThread::errorOccurred,
                    this, &MainWindow::onPrintError);
            
            // 启动打印线程
            m_printThread->start();
            m_jobStarted = true;
            m_printing = true;
            updateButtonState();
        }
    } else {
        // 停止打印作业
        if (m_printThread) {
            IDP_StopPrintJob();
            m_printThread->stop();
            if (!m_printThread->wait(5000)) {
                qWarning() << "PrintThread did not finish in time when stopping";
                m_printThread->terminate();
                m_printThread->wait(1000);
            }
            delete m_printThread;
            m_printThread = nullptr;
        }
        m_jobStarted = false;
        m_printing = false;
        updateButtonState();
    }
}

void MainWindow::onButtonLoadImageClicked()
{
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "选择图像文件",
        "",
        "图像文件 (*.bmp *.prt *.cli);;所有文件 (*.*)"
    );
    
    if (!fileName.isEmpty()) {
        int result = getSrcData(fileName);
        if (result == 0) {
            m_prtFilePath = fileName;
            ui->lineEditFilePath->setText(fileName);
            m_imagePreview->showPreview(fileName);
            m_imageLoaded = true;
            updateButtonState();

            ImgLayerSetDialog dlg(this);
            dlg.exec();
        } else {
            QString errorMessage;
            if (result == -2) {
                errorMessage = "图像文件未找到或无法打开！"; // File not found or couldn't be opened
            } else if (result == -3) {
                errorMessage = "图像格式无效或不支持的位深（仅支持1位BMP）！"; // Invalid BMP format or unsupported bit-depth
            } else {
                errorMessage = QString("加载图像文件失败，错误码：%1").arg(result); // Generic error
            }
            QMessageBox::critical(this, "错误", errorMessage);
        }
    }
}

void MainWindow::onButtonReloadWaveClicked()
{
    if (!g_pSysInfo) {
        QMessageBox::warning(this, "错误", "未获取到设备信息，无法加载波形。");
        return;
    }

    ReloadWaveDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        int colorIndex = dlg.selectedColorIndex();
        int groupIndex = dlg.selectedGroupIndex();
        QString wavePath = dlg.selectedWavePath();

        // The original code has a potential typo `g_nGroupIndex` which is not a member of the dialog
        // Here we use the correct value from the dialog.
        // Also, the original code uses T2A for conversion, we'll use Qt's built-in methods.
        QByteArray wavePathBytes = wavePath.toLocal8Bit();
        int nResult = DEV_ReloadWaveForm((1 << colorIndex), (1 << groupIndex), (const char*)wavePathBytes.constData());

        if (nResult < 0) {
            // Re-implementing the customer ID check from the original
            QFile file(wavePath);
            if (file.open(QIODevice::ReadOnly)) {
                unsigned int headerData[8];
                qint64 bytesRead = file.read(reinterpret_cast<char*>(headerData), sizeof(headerData));
                file.close();

                if (bytesRead >= sizeof(headerData) && headerData[5] != g_pSysInfo->nCustomerID) {
                     QMessageBox::critical(this, "加载失败", "波形文件中的客户ID与设备不匹配。");
                     return;
                }
            }

            QMessageBox::critical(this, "加载失败", QString("加载波形文件失败，错误码: %1").arg(nResult));
        } else {
            QMessageBox::information(this, "成功", "波形文件加载成功。");
        }
    }
}

void MainWindow::onButtonVTSetClicked()
{
    // VT设置对话框
    VTSetDialog dlg(this);
    dlg.exec();
}

void MainWindow::onButtonCaliClicked()
{
    AdjParamDialog dlg(&m_printMutex, this);
    if (dlg.exec() == QDialog::Accepted) {
        // The dialog now handles its own parameter saving,
        // but we might need to sync the main window's copy if necessary.
        // For now, we just update the global param like the MFC version.
        if (!DEV_UpdateParam(&g_sysParam))
            QMessageBox::warning(this, "警告", "更新校准参数失败。");
        else
            QMessageBox::information(this, "成功", "更新校准参数成功。");
    }
}

void MainWindow::onButtonSysParamClicked()
{
    SysParamDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        // 保存系统参数
        // ...
    }
}

void MainWindow::onButtonFlashClicked()
{
    PrtRunInfo runInfo;
    if (IDP_GetPrintState(&runInfo)) {
        bool bFlashState = (runInfo.nPrtState == 3); // 3 is Flash state
        if (!IDP_FlashPrtCtl(!bFlashState)) { // Toggle flash state
            QMessageBox::critical(this, "错误", "切换闪喷状态失败！");
        } else {
            if (bFlashState) {
                ui->buttonFlash->setText("开启闪喷"); // Assuming original text was "关闭闪喷"
            } else {
                ui->buttonFlash->setText("关闭闪喷"); // Assuming original text was "开启闪喷"
            }
        }
    } else {
        QMessageBox::critical(this, "错误", "获取打印状态失败，无法切换闪喷！");
    }
}

void MainWindow::onButtonResetEncoderClicked()
{
    if (!DEV_ResetPrinterEncValue(ZERO_POSITION)) {
        QMessageBox::critical(this, "错误", "重置编码器失败！");
    } else {
        QMessageBox::information(this, "成功", "编码器已重置。");
    }
}

void MainWindow::onButtonMoveCtlClicked()
{
#ifdef RY_MOVE_CTL
    MoveCtlDialog dlg(this);
    dlg.exec();
#else
    QMessageBox::information(this, "不支持", "此版本未编译运动控制模块。");
#endif
}

void MainWindow::onButtonAirInkClicked()
{
    AirInkDialog dlg(this);
    dlg.exec();
}

void MainWindow::onButtonAdibCtlClicked()
{
    AdibCtrlDialog dlg(this);
    dlg.exec();
}

void MainWindow::onCheckInkWatchToggled(bool checked)
{
    m_enableInkWatch = checked;
    if (!DEV_EnableInkWatch(checked, 0, 0)) { // Assuming 0,0 are default values or not critical
        QMessageBox::critical(this, "错误", QString("墨水监测%1失败！").arg(checked ? "开启" : "关闭"));
    } else {
        QMessageBox::information(this, "成功", QString("墨水监测已%1。").arg(checked ? "开启" : "关闭"));
    }
}

void MainWindow::onCheckCloseAutoCheckToggled(bool checked)
{
    m_closeAutoCheck = checked;
    // 更新监控线程的设置
    if (m_monitorThread) {
        m_monitorThread->setCloseAutoCheck(checked);
    }
}

void MainWindow::onFilePathChanged(const QString &text)
{
    m_prtFilePath = text;
}

void MainWindow::onPrintStateChanged(uint state)
{
    updatePrtState(state);
}

void MainWindow::onPrintProgress(int progress)
{
    ui->progressBar->setValue(progress);
}

void MainWindow::onPrintFinished()
{
    m_jobStarted = false;
    m_printing = false;
    updateButtonState();
    QMessageBox::information(this, "提示", "打印完成！");
}

void MainWindow::onPrintError(const QString &error)
{
    QMessageBox::critical(this, "错误", error);
    m_jobStarted = false;
    m_printing = false;
    updateButtonState();
}

void MainWindow::onMonitorDataUpdated()
{
    updateVTListInfo();
    if (ui->labelEncoderValue) {
        ui->labelEncoderValue->setText(QString::number(m_prtEncoderVal));
    }
    
    // 更新打印信息树
    if (m_printInfoTree) {
        m_printInfoTree->updateDeviceInfo();
    }
    
    // 更新驱动信息树
    if (m_drvInfoTree) {
        m_drvInfoTree->updateVTListInfo();
    }
}

int MainWindow::getSrcData(const QString &filePath)
{
    // This function replicates the unusual logic from the MFC version,
    // which ignores the given file's extension and instead looks for
    // numbered BMP files in the same directory.

    QFileInfo originalFileInfo(filePath);
    QDir baseDir = originalFileInfo.dir();

    // The number of color channels to load is determined by the global parameter.
    for (int c = 0; c < g_PrtImgLayer.nColorCnts; ++c)
    {
        QString channelFileName = QString("%1.bmp").arg(c + 1);
        QString channelFilePath = baseDir.filePath(channelFileName);

        QFile file(channelFilePath);
        if (!file.open(QIODevice::ReadOnly)) {
            qWarning() << "Could not open channel file:" << channelFilePath;
            return -2; // File not found
        }

        QByteArray fileData = file.readAll();
        file.close();
        
        if (fileData.size() < sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER)) {
            qWarning() << "Invalid BMP file (too small):" << channelFilePath;
            return -3; // Invalid format
        }

        BITMAPFILEHEADER* pfheader = reinterpret_cast<BITMAPFILEHEADER*>(fileData.data());
        BITMAPINFOHEADER* pbmpheader = reinterpret_cast<BITMAPINFOHEADER*>(fileData.data() + sizeof(BITMAPFILEHEADER));

        if (pbmpheader->biBitCount != 1) {
            qWarning() << "Unsupported BMP format (not 1-bit):" << channelFilePath;
            return -3; // Not 1-bit monochrome
        }
        
        // Clear previous data
        g_prtData[c].clear();

        int dataOffset = pfheader->bfOffBits;
        int dataSize = fileData.size() - dataOffset;
        
        // Get the raw pixel data
        QByteArray pixelData = fileData.right(dataSize);

        // Check palette and invert if necessary (as per MFC logic)
        RGBQUAD* pcolor = reinterpret_cast<RGBQUAD*>(fileData.data() + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER));
        COLORREF clrZeroColor = RGB(pcolor[0].rgbRed, pcolor[0].rgbGreen, pcolor[0].rgbBlue);

        if (clrZeroColor == 0) { // If palette color 0 is black, invert data
            for (int i = 0; i < pixelData.size(); ++i) {
                pixelData[i] = ~pixelData[i];
            }
        }
        
        // Store the (potentially inverted) pixel data
        g_prtData[c] = pixelData;
        g_prtDataSize = dataSize; // Set the global size for the print thread

        // Update global layer info from the first valid BMP file
        if (c == 0) {
            if(pbmpheader->biSizeImage == 0) {
                g_PrtImgLayer.nBytesPerLine = (pbmpheader->biWidth + 31) / 32 * 4;
            } else {
                g_PrtImgLayer.nBytesPerLine = pbmpheader->biSizeImage / pbmpheader->biHeight;
            }
            g_PrtImgLayer.nHeight = pbmpheader->biHeight;
            g_PrtImgLayer.nWidth = pbmpheader->biWidth / g_PrtImgLayer.nGrayBits;
        }
    }

    g_PrtImgLayer.nLayerIndex = 0;
    m_imageLoaded = true;
    return 0; // Success
}

QString MainWindow::getApplicationPath() const
{
    return QApplication::applicationDirPath();
}

