#include "MainWindow.h"
#include "PrintInfoTreeWidget.h"
#include "ImagePreviewWidget.h"
#include "PrintThread.h"
#include "MonitorThread.h"
#include "ParameterManager.h"
#include "PrintJobDialog.h"
#include "SysParamDialog.h"
#include "../Inc/RYPrtCtler.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QApplication>
#include <QDir>
#include <QDebug>
#include <QVBoxLayout>
#include <QHBoxLayout>

// 全局变量定义（与MFC版本保持一致）
RYUSR_PARAM g_sysParam;
PRTJOB_ITEM g_testJob;
PRTIMG_LAYER g_PrtImgLayer;
RYCalbrationParam g_Calbration;
LPPRINTER_INFO g_pSysInfo = nullptr;
bool g_bPHValid[MAX_PH_CNT] = { false };
HANDLE g_PrtMutex = INVALID_HANDLE_VALUE;

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
    m_monitorThread = new MonitorThread(this);
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
    // 先停止打印线程（可能正在使用设备）
    if (m_printThread) {
        IDP_StopPrintJob();  // 先停止打印作业
        m_printThread->stop();
        if (!m_printThread->wait(5000)) {
            qWarning() << "PrintThread did not finish in time";
            m_printThread->terminate();
            m_printThread->wait(1000);
        }
        delete m_printThread;
        m_printThread = nullptr;
    }
    
    // 停止监控线程
    if (m_monitorThread) {
        m_monitorThread->stop();
        if (!m_monitorThread->wait(5000)) {
            qWarning() << "MonitorThread did not finish in time";
            m_monitorThread->terminate();
            m_monitorThread->wait(1000);
        }
        // MonitorThread是MainWindow的子对象，会自动删除，不需要手动delete
    }
    
    // 关闭互斥锁
    if (g_PrtMutex != INVALID_HANDLE_VALUE) {
        CloseHandle(g_PrtMutex);
        g_PrtMutex = INVALID_HANDLE_VALUE;
    }
    
    // 保存参数
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
            
            // 创建打印线程，传递图层数量
            m_printThread = new PrintThread(m_jobImgLayerCounts, this);
            connect(m_printThread, &PrintThread::printStateChanged,
                    this, &MainWindow::onPrintStateChanged);
            connect(m_printThread, &PrintThread::printProgress,
                    this, &MainWindow::onPrintProgress);
            connect(m_printThread, &PrintThread::printFinished,
                    this, &MainWindow::onPrintFinished);
            connect(m_printThread, &PrintThread::errorOccurred,
                    this, &MainWindow::onPrintError);
            
            // 创建互斥锁
            if (g_PrtMutex == INVALID_HANDLE_VALUE) {
                g_PrtMutex = CreateMutex(NULL, FALSE, "PrintMutex");
            }
            
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
    // 重载波形对话框
    // ReloadWaveDialog dlg(this);
    // dlg.exec();
}

void MainWindow::onButtonVTSetClicked()
{
    // VT设置对话框
    // VTSetDialog dlg(this);
    // dlg.exec();
}

void MainWindow::onButtonCaliClicked()
{
    // 校准对话框
    // CalibrationDialog dlg(this);
    // dlg.exec();
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
    // 运动控制对话框
    // MoveCtlDialog dlg(this);
    // dlg.exec();
}

void MainWindow::onButtonAirInkClicked()
{
    // 气墨控制对话框
    // AirInkDialog dlg(this);
    // dlg.exec();
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
    // 加载图像数据
    // 这里需要根据具体的图像格式来实现
    // 对应MFC版本的GetSrcData()函数
    
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return -1;
    }
    
    // 根据文件扩展名判断格式
    QString suffix = QFileInfo(filePath).suffix().toLower();
    if (suffix == "bmp") {
        // 加载BMP格式
        // ...
    } else if (suffix == "prt") {
        // 加载PRT格式
        // ...
    } else if (suffix == "cli") {
        // 加载CLI格式
        // ...
    }
    
    return 0;
}

QString MainWindow::getApplicationPath() const
{
    return QApplication::applicationDirPath();
}

