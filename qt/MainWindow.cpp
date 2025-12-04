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
#include "../Inc/RYPrtCtler.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QApplication>
#include <QDir>
#include <QDebug>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <windows.h> // For BITMAPFILEHEADER etc.

// 全局变量定义（与MFC版本保持一致）
RYUSR_PARAM g_sysParam;
PRTJOB_ITEM g_testJob;
PRTIMG_LAYER g_PrtImgLayer;
RYCalbrationParam g_Calbration;
LPPRINTER_INFO g_pSysInfo = nullptr;
bool g_bPHValid[MAX_PH_CNT] = { false };

/**
 * @brief 初始化图像图层信息
 */
void InitImgLayerInfo()
{
    memset(&g_PrtImgLayer, 0, sizeof(PRTIMG_LAYER));
    g_PrtImgLayer.nLayerIndex = -1;
    g_PrtImgLayer.nXDPI = 720;
    g_PrtImgLayer.nYDPI = 600;
    g_PrtImgLayer.nColorCnts = 1;
    g_PrtImgLayer.nGrayBits = 1;
    g_PrtImgLayer.nPrtDir = 1;
    g_PrtImgLayer.nValidClrMask = 0xFFFF;
    g_PrtImgLayer.fLayerDensity = 1.0f;
    g_PrtImgLayer.nPrtFlag = 1;
}

/**
 * @brief 初始化作业信息
 */
void InitJobInfo()
{
    memset(&g_testJob, 0, sizeof(PRTJOB_ITEM));
    g_testJob.nJobID = 0;
    strncpy_s(g_testJob.szJobName, "Scan Demo", sizeof(g_testJob.szJobName));
    g_testJob.fPrtXPos = 100.0f;
    g_testJob.fClipHeight = 0.0f;
    g_testJob.fClipWidth = 0.0f;
    g_testJob.fOutXdpi = 600.0f;
    g_testJob.fOutYdpi = 600.0f;
    g_testJob.nFileType = 0;
    g_testJob.nOutPixelBits = 1;
}

/**
 * @brief 初始化校准信息
 */
void InitCalibrationInfo()
{
    memset(&g_Calbration, 0, sizeof(RYCalbrationParam));
    g_Calbration.fPrtXPos = 10.0f;
    g_Calbration.fPrtYPos = 0.0f;
    g_Calbration.fStpSize = 0.0f;
    g_Calbration.fxadjdpi = 600.0f;
    g_Calbration.fXMaxPrtWidth = 600.0f;
    g_Calbration.fXRunSpd = 10.0f;
    g_Calbration.fYMaxPrtHeight = 600.0f;
    g_Calbration.nAdjType = 0;
    g_Calbration.nCtrlValue = 0;
    g_Calbration.nGrayBits = 1;
    g_Calbration.nPrtDir = 1;
}

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
    , m_prtDataSize(0)
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
    }
    
    // 初始化设备信息
    initDeviceInfo();
    
    // 初始化系统参数
    initSysParam();
    
    // 更新按钮状态
    updateButtonState();
}

void MainWindow::initDeviceInfo()
{
    // 初始化设备信息
    g_pSysInfo = DEV_GetDeviceInfo();
    if (g_pSysInfo) {
        updateVTListInfo();
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

void MainWindow::updateVTListInfo()
{
    if (!g_pSysInfo) {
        return;
    }
    
    // 更新VT列表信息（温度、电压等）
    // 这里需要根据具体的树形控件API来实现
    // m_drvInfoTree->updateDeviceInfo(g_pSysInfo);
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
        if (getSrcData(fileName) == 0) {
            m_prtFilePath = fileName;
            ui->lineEditFilePath->setText(fileName);
            m_imagePreview->showPreview(fileName);
            m_imageLoaded = true;
            updateButtonState();
        } else {
            QMessageBox::critical(this, "错误", "加载图像文件失败！");
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
    // 闪喷功能
    // ...
}

void MainWindow::onButtonResetEncoderClicked()
{
    // 重置编码器
    // DEV_ResetEncoder();
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
    // ADIB控制对话框
    // AdibCtrlDialog dlg(this);
    // dlg.exec();
}

void MainWindow::onCheckInkWatchToggled(bool checked)
{
    m_enableInkWatch = checked;
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
        m_prtData[c].clear();

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
        m_prtData[c] = pixelData;
        m_prtDataSize = dataSize; // Set the global size for the print thread

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

