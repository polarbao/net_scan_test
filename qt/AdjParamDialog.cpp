#include "AdjParamDialog.h"
#include "ui_AdjParamDialog.h"
#include "LackJetDialog.h"
#include "CalibrationPrintThread.h"
#include "../Inc/RYPrtCtler.h"
#include <QMessageBox>

// Reference global variables
extern RYUSR_PARAM g_sysParam;
extern RYCalbrationParam g_Calbration;
extern LPPRINTER_INFO g_pSysInfo;

AdjParamDialog::AdjParamDialog(QMutex* hwMutex, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AdjParamDialog),
    m_hwMutex(hwMutex),
    m_printThread(nullptr)
{
    ui->setupUi(this);
    initDialog();

    // Connect signals and slots
    connect(ui->buttonGroupOffset, &QPushButton::clicked, this, &AdjParamDialog::on_buttonGroupOffset_clicked);
    connect(ui->buttonJetOffset, &QPushButton::clicked, this, &AdjParamDialog::on_buttonJetOffset_clicked);
    connect(ui->comboGroupDir, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &AdjParamDialog::on_comboGroupDir_currentIndexChanged);
    connect(ui->comboJetDir, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &AdjParamDialog::on_comboJetDir_currentIndexChanged);

    connect(ui->buttonPrintStatus, &QPushButton::clicked, this, &AdjParamDialog::on_buttonPrintStatus_clicked);
    connect(ui->buttonPrintVertical, &QPushButton::clicked, this, &AdjParamDialog::on_buttonPrintVertical_clicked);
    connect(ui->buttonPrintStep, &QPushButton::clicked, this, &AdjParamDialog::on_buttonPrintStep_clicked);
    connect(ui->buttonPrintXOffsetGroup, &QPushButton::clicked, this, &AdjParamDialog::on_buttonPrintXOffsetGroup_clicked);
    connect(ui->buttonPrintXOffsetJet, &QPushButton::clicked, this, &AdjParamDialog::on_buttonPrintXOffsetJet_clicked);
    connect(ui->buttonPrintBidirectional, &QPushButton::clicked, this, &AdjParamDialog::on_buttonPrintBidirectional_clicked);
    connect(ui->buttonStopPrint, &QPushButton::clicked, this, &AdjParamDialog::on_buttonStopPrint_clicked);

    connect(ui->buttonLackJet, &QPushButton::clicked, this, &AdjParamDialog::on_buttonLackJet_clicked);
    connect(ui->buttonApplyAndClose, &QPushButton::clicked, this, &AdjParamDialog::on_buttonApplyAndClose_clicked);
}

AdjParamDialog::~AdjParamDialog()
{
    if (m_printThread && m_printThread->isRunning()) {
        m_printThread->stop();
        m_printThread->wait();
    }
    delete ui;
}

void AdjParamDialog::initDialog()
{
    if (!g_pSysInfo) {
        QMessageBox::critical(this, "Error", "g_pSysInfo is NULL. Cannot initialize dialog.");
        return;
    }
    
    // Copy global calibration params to local
    memcpy(&m_calParam, &g_Calbration, sizeof(RYCalbrationParam));

    populateComboBoxes();
    setupOffsetTrees();
    updateGroupOffsetList();
    updateJetOffsetList();

    // Load calibration print parameters
    ui->comboPrintDir->setCurrentIndex(m_calParam.nPrtDir);
    ui->spinBoxAdjDpi->setValue(m_calParam.fxadjdpi);
    ui->spinBoxRunSpeed->setValue(m_calParam.fXRunSpd);
    ui->spinBoxMaxWidth->setValue(m_calParam.fXMaxPrtWidth);
    ui->spinBoxMaxHeight->setValue(m_calParam.fYMaxPrtHeight);
    ui->spinBoxXPos->setValue(m_calParam.fPrtXPos);
    ui->spinBoxGrayBits->setValue(m_calParam.nGrayBits);
    
    ui->buttonStopPrint->setEnabled(false);
}

void AdjParamDialog::populateComboBoxes()
{
    QStringList colors, groups, jets, dirs;
    for (int i = 0; i < MAX_COLORS; ++i) colors << QString("C-%1").arg(i + 1);
    for (int i = 0; i < MAX_GROUP; ++i) groups << QString("G-%1").arg(i + 1);
    for (int i = 0; i < MAX_JETROW; ++i) jets << QString("Jet-%1").arg(i + 1);
    dirs << "负方向" << "正方向";

    ui->comboGroupColor->addItems(colors);
    ui->comboJetColor->addItems(colors);
    ui->comboGroupGroup->addItems(groups);
    ui->comboJetGroup->addItems(groups);
    ui->comboJetJet->addItems(jets);
    ui->comboGroupDir->addItems(dirs);
    ui->comboJetDir->addItems(dirs);
    ui->comboPrintDir->addItems(dirs);
}

void AdjParamDialog::setupOffsetTrees()
{
    QStringList groupHeaders, jetHeaders;
    groupHeaders << "颜色/组";
    jetHeaders << "颜色/组/喷嘴";
    for (int c = 0; c < MAX_COLORS; c++) {
        groupHeaders << QString("C-%1").arg(c + 1);
        jetHeaders << QString("C-%1").arg(c + 1);
    }
    ui->treeGroupOffset->setHeaderLabels(groupHeaders);
    ui->treeJetOffset->setHeaderLabels(jetHeaders);
}

void AdjParamDialog::updateGroupOffsetList()
{
    ui->treeGroupOffset->clear();
    int dir = ui->comboGroupDir->currentIndex();

    for (int g = 0; g < MAX_GROUP; ++g) {
        QStringList row;
        row << QString("G-%1").arg(g + 1);
        for (int c = 0; c < MAX_COLORS; ++c) {
            row << QString::number(g_sysParam.nPhgXGroupOff[c][g][dir]);
        }
        new QTreeWidgetItem(ui->treeGroupOffset, row);
    }
    for(int i=0; i<ui->treeGroupOffset->columnCount(); ++i) ui->treeGroupOffset->resizeColumnToContents(i);
}

void AdjParamDialog::updateJetOffsetList()
{
    ui->treeJetOffset->clear();
    int dir = ui->comboJetDir->currentIndex();

    for (int g = 0; g < MAX_GROUP; ++g) {
        QTreeWidgetItem* groupItem = new QTreeWidgetItem(ui->treeJetOffset, QStringList(QString("G-%1").arg(g + 1)));
        for (int j = 0; j < 8; ++j) { // As per MFC code
            QStringList jetRow;
            jetRow << QString("  Jet-%1").arg(j + 1);
            for (int c = 0; c < MAX_COLORS; ++c) {
                jetRow << QString::number(g_sysParam.nPhgJetRowOff[c][g][j][dir]);
            }
            new QTreeWidgetItem(groupItem, jetRow);
        }
    }
    ui->treeJetOffset->expandAll();
    for(int i=0; i<ui->treeJetOffset->columnCount(); ++i) ui->treeJetOffset->resizeColumnToContents(i);
}


// --- Slots ---

void AdjParamDialog::on_buttonGroupOffset_clicked()
{
    int color = ui->comboGroupColor->currentIndex();
    int group = ui->comboGroupGroup->currentIndex();
    int dir = ui->comboGroupDir->currentIndex();
    int offset = ui->spinBoxGroupOffset->value();

    if (color < 0 || group < 0 || dir < 0) return;

    g_sysParam.nPhgXGroupOff[color][group][dir] = offset;
    updateGroupOffsetList();
}

void AdjParamDialog::on_buttonJetOffset_clicked()
{
    int color = ui->comboJetColor->currentIndex();
    int group = ui->comboJetGroup->currentIndex();
    int jet = ui->comboJetJet->currentIndex();
    int dir = ui->comboJetDir->currentIndex();
    int offset = ui->spinBoxJetOffset->value();

    if (color < 0 || group < 0 || jet < 0 || dir < 0) return;

    g_sysParam.nPhgJetRowOff[color][group][jet][dir] = offset;
    updateJetOffsetList();
}

void AdjParamDialog::on_comboGroupDir_currentIndexChanged(int index) { updateGroupOffsetList(); }
void AdjParamDialog::on_comboJetDir_currentIndexChanged(int index) { updateJetOffsetList(); }

void AdjParamDialog::on_buttonLackJet_clicked()
{
    LackJetDialog dlg(this);
    dlg.exec();
}

void AdjParamDialog::on_buttonApplyAndClose_clicked()
{
    // Apply final BiDir offset like in MFC
    int biDirOffset = ui->spinBoxAdjDpi->value(); // Assuming this is the control
    if (g_Calbration.fxadjdpi > 0) {
        g_sysParam.nBiDirEncPrtOff = (int)((float)g_pSysInfo->nXSysEncDPI / g_Calbration.fxadjdpi * biDirOffset);
    }
    
    // Save all calibration parameters back to global
    memcpy(&g_Calbration, &m_calParam, sizeof(RYCalbrationParam));
    
    if(!DEV_UpdateParam(&g_sysParam))
        QMessageBox::warning(this, "警告", "更新双向偏移补偿失败。");

    accept();
}

void AdjParamDialog::startCalibrationPrint(int adjType)
{
    if (m_printThread && m_printThread->isRunning()) {
        QMessageBox::warning(this, "警告", "另一个校准打印正在进行中。");
        return;
    }

    // Update local params from UI
    m_calParam.nPrtDir = ui->comboPrintDir->currentIndex();
    m_calParam.fxadjdpi = ui->spinBoxAdjDpi->value();
    m_calParam.fXRunSpd = ui->spinBoxRunSpeed->value();
    m_calParam.fXMaxPrtWidth = ui->spinBoxMaxWidth->value();
    m_calParam.fYMaxPrtHeight = ui->spinBoxMaxHeight->value();
    m_calParam.fPrtXPos = ui->spinBoxXPos->value();
    m_calParam.nGrayBits = ui->spinBoxGrayBits->value();
    m_calParam.nAdjType = adjType;
    
    // Copy to global struct for the library
    memcpy(&g_Calbration, &m_calParam, sizeof(RYCalbrationParam));

    m_printThread = new CalibrationPrintThread(m_calParam, m_hwMutex, this);
    connect(m_printThread, &CalibrationPrintThread::printFinished, this, &AdjParamDialog::onPrintFinished);
    connect(m_printThread, &CalibrationPrintThread::errorOccurred, this, &AdjParamDialog::onPrintError);
    connect(m_printThread, &QThread::finished, m_printThread, &QObject::deleteLater);

    m_printThread->start();

    ui->tabWidget->findChild<QGroupBox*>("groupBox_4")->setEnabled(false);
    ui->buttonStopPrint->setEnabled(true);
}

void AdjParamDialog::onPrintFinished()
{
    QMessageBox::information(this, "完成", "校准打印已完成。");
    ui->tabWidget->findChild<QGroupBox*>("groupBox_4")->setEnabled(true);
    ui->buttonStopPrint->setEnabled(false);
}

void AdjParamDialog::onPrintError(const QString &error)
{
    QMessageBox::critical(this, "打印错误", error);
    ui->tabWidget->findChild<QGroupBox*>("groupBox_4")->setEnabled(true);
    ui->buttonStopPrint->setEnabled(false);
}

void AdjParamDialog::on_buttonStopPrint_clicked()
{
    if (m_printThread && m_printThread->isRunning()) {
        m_printThread->stop();
    }
}

// Slots for starting each calibration type
void AdjParamDialog::on_buttonPrintStatus_clicked() { startCalibrationPrint(0); }
void AdjParamDialog::on_buttonPrintVertical_clicked() { startCalibrationPrint(1); }
void AdjParamDialog::on_buttonPrintStep_clicked() { startCalibrationPrint(2); }
void AdjParamDialog::on_buttonPrintXOffsetGroup_clicked() { startCalibrationPrint(3); }
void AdjParamDialog::on_buttonPrintXOffsetJet_clicked() { startCalibrationPrint(4); }
void AdjParamDialog::on_buttonPrintBidirectional_clicked() { startCalibrationPrint(5); }

