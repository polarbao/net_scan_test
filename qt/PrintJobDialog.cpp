#include "PrintJobDialog.h"
#include "ui_PrintJobDialog.h"
#include "../Inc/RYPrtCtler.h"

// 外部变量
extern PRTJOB_ITEM g_testJob;

PrintJobDialog::PrintJobDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::PrintJobDialog)
    , m_grayBits(1)
    , m_xOutDPI(600.0f)
    , m_yOutDPI(600.0f)
    , m_clipWidth(0.0f)
    , m_clipHeight(0.0f)
    , m_yPrtPos(0.0f)
    , m_xPrtPos(100.0f)
    , m_fileType(0)
    , m_jobID(0)
    , m_imageLayerCount(1)
    , m_whiteJump(false)
    , m_cycleOff(false)
    , m_radomJetOff(false)
    , m_xMirror(false)
    , m_yMirror(false)
    , m_doubleYDpi(false)
    , m_screenshotPrt(false)
{
    ui->setupUi(this);
    
    // 从全局变量加载数据
    loadFromGlobal();
    
    // 初始化UI
    initUI();
    
    // 连接信号槽
    connect(ui->buttonBox, &QDialogButtonBox::accepted,
            this, &PrintJobDialog::onAccepted);
}

PrintJobDialog::~PrintJobDialog()
{
    delete ui;
}

void PrintJobDialog::initUI()
{
    // 设置控件值
    if (ui->spinBoxGrayBits) {
        ui->spinBoxGrayBits->setValue(m_grayBits);
    }
    if (ui->doubleSpinBoxXDPI) {
        ui->doubleSpinBoxXDPI->setValue(m_xOutDPI);
    }
    if (ui->doubleSpinBoxYDPI) {
        ui->doubleSpinBoxYDPI->setValue(m_yOutDPI);
    }
    if (ui->doubleSpinBoxClipWidth) {
        ui->doubleSpinBoxClipWidth->setValue(m_clipWidth);
    }
    if (ui->doubleSpinBoxClipHeight) {
        ui->doubleSpinBoxClipHeight->setValue(m_clipHeight);
    }
    if (ui->doubleSpinBoxXPrtPos) {
        ui->doubleSpinBoxXPrtPos->setValue(m_xPrtPos);
    }
    if (ui->doubleSpinBoxYPrtPos) {
        ui->doubleSpinBoxYPrtPos->setValue(m_yPrtPos);
    }
    if (ui->comboBoxFileType) {
        ui->comboBoxFileType->setCurrentIndex(m_fileType);
    }
    if (ui->lineEditJobFilePath) {
        ui->lineEditJobFilePath->setText(m_jobFilePath);
    }
    if (ui->lineEditJobName) {
        ui->lineEditJobName->setText(m_jobName);
    }
    if (ui->spinBoxJobID) {
        ui->spinBoxJobID->setValue(m_jobID);
    }
    if (ui->spinBoxImageLayerCount) {
        ui->spinBoxImageLayerCount->setValue(m_imageLayerCount);
    }
    if (ui->checkBoxWhiteJump) {
        ui->checkBoxWhiteJump->setChecked(m_whiteJump);
    }
    if (ui->checkBoxCycleOff) {
        ui->checkBoxCycleOff->setChecked(m_cycleOff);
    }
    if (ui->checkBoxRadomJetOff) {
        ui->checkBoxRadomJetOff->setChecked(m_radomJetOff);
    }
    if (ui->checkBoxXMirror) {
        ui->checkBoxXMirror->setChecked(m_xMirror);
    }
    if (ui->checkBoxYMirror) {
        ui->checkBoxYMirror->setChecked(m_yMirror);
    }
    if (ui->checkBoxDoubleYDpi) {
        ui->checkBoxDoubleYDpi->setChecked(m_doubleYDpi);
    }
    if (ui->checkBoxScreenshotPrt) {
        ui->checkBoxScreenshotPrt->setChecked(m_screenshotPrt);
    }
}

void PrintJobDialog::loadFromGlobal()
{
    // 从全局变量g_testJob加载数据
    m_grayBits = g_testJob.nOutPixelBits;
    m_xOutDPI = g_testJob.fOutXdpi;
    m_yOutDPI = g_testJob.fOutYdpi;
    m_clipWidth = g_testJob.fClipWidth;
    m_clipHeight = g_testJob.fClipHeight;
    m_xPrtPos = g_testJob.fPrtXPos;
    m_yPrtPos = g_testJob.fPrtYPos;
    m_fileType = g_testJob.nFileType;
    m_jobFilePath = QString::fromLocal8Bit(g_testJob.szJobFilePath);
    m_jobName = QString::fromLocal8Bit(g_testJob.szJobName);
    m_jobID = g_testJob.nJobID;
    m_imageLayerCount = g_testJob.nImageLayerCount;
    
    // 标志位（需要根据实际结构体定义）
    // m_whiteJump = ...
    // m_cycleOff = ...
    // ...
}

void PrintJobDialog::onAccepted()
{
    // 从控件读取数据
    if (ui->spinBoxGrayBits) {
        m_grayBits = ui->spinBoxGrayBits->value();
    }
    if (ui->doubleSpinBoxXDPI) {
        m_xOutDPI = ui->doubleSpinBoxXDPI->value();
    }
    if (ui->doubleSpinBoxYDPI) {
        m_yOutDPI = ui->doubleSpinBoxYDPI->value();
    }
    if (ui->doubleSpinBoxClipWidth) {
        m_clipWidth = ui->doubleSpinBoxClipWidth->value();
    }
    if (ui->doubleSpinBoxClipHeight) {
        m_clipHeight = ui->doubleSpinBoxClipHeight->value();
    }
    if (ui->doubleSpinBoxXPrtPos) {
        m_xPrtPos = ui->doubleSpinBoxXPrtPos->value();
    }
    if (ui->doubleSpinBoxYPrtPos) {
        m_yPrtPos = ui->doubleSpinBoxYPrtPos->value();
    }
    if (ui->comboBoxFileType) {
        m_fileType = ui->comboBoxFileType->currentIndex();
    }
    if (ui->lineEditJobFilePath) {
        m_jobFilePath = ui->lineEditJobFilePath->text();
    }
    if (ui->lineEditJobName) {
        m_jobName = ui->lineEditJobName->text();
    }
    if (ui->spinBoxJobID) {
        m_jobID = ui->spinBoxJobID->value();
    }
    if (ui->spinBoxImageLayerCount) {
        m_imageLayerCount = ui->spinBoxImageLayerCount->value();
    }
    if (ui->checkBoxWhiteJump) {
        m_whiteJump = ui->checkBoxWhiteJump->isChecked();
    }
    if (ui->checkBoxCycleOff) {
        m_cycleOff = ui->checkBoxCycleOff->isChecked();
    }
    if (ui->checkBoxRadomJetOff) {
        m_radomJetOff = ui->checkBoxRadomJetOff->isChecked();
    }
    if (ui->checkBoxXMirror) {
        m_xMirror = ui->checkBoxXMirror->isChecked();
    }
    if (ui->checkBoxYMirror) {
        m_yMirror = ui->checkBoxYMirror->isChecked();
    }
    if (ui->checkBoxDoubleYDpi) {
        m_doubleYDpi = ui->checkBoxDoubleYDpi->isChecked();
    }
    if (ui->checkBoxScreenshotPrt) {
        m_screenshotPrt = ui->checkBoxScreenshotPrt->isChecked();
    }
    
    // 保存到全局变量
    saveToGlobal();
    
    accept();
}

void PrintJobDialog::saveToGlobal()
{
    // 保存到全局变量g_testJob
    g_testJob.nOutPixelBits = m_grayBits;
    g_testJob.fOutXdpi = m_xOutDPI;
    g_testJob.fOutYdpi = m_yOutDPI;
    g_testJob.fClipWidth = m_clipWidth;
    g_testJob.fClipHeight = m_clipHeight;
    g_testJob.fPrtXPos = m_xPrtPos;
    g_testJob.fPrtYPos = m_yPrtPos;
    g_testJob.nFileType = m_fileType;
    
    QByteArray jobFilePathBytes = m_jobFilePath.toLocal8Bit();
    strncpy_s(g_testJob.szJobFilePath, jobFilePathBytes.constData(), 
              sizeof(g_testJob.szJobFilePath) - 1);
    
    QByteArray jobNameBytes = m_jobName.toLocal8Bit();
    strncpy_s(g_testJob.szJobName, jobNameBytes.constData(),
              sizeof(g_testJob.szJobName) - 1);
    
    g_testJob.nJobID = m_jobID;
    g_testJob.nImageLayerCount = m_imageLayerCount;
}

