#include "AirInkDialog.h"
#include "ui_AirInkDialog.h"
#include "../Inc/RYPrtCtler.h"
#include <QCloseEvent>
#include <QMessageBox>

// Reference global variables
extern RYUSR_PARAM g_sysParam;
extern LPPRINTER_INFO g_pSysInfo;

AirInkDialog::AirInkDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AirInkDialog),
    m_isPushingInk(false)
{
    ui->setupUi(this);
    
    // Initialize state array
    for(int i=0; i<6; ++i) {
        m_outputStates[i] = false;
    }

    initDialog();

    // Setup timers
    m_statusTimer = new QTimer(this);
    connect(m_statusTimer, &QTimer::timeout, this, &AirInkDialog::onStatusTimerTimeout);

    m_pushInkTimer = new QTimer(this);
    m_pushInkTimer->setSingleShot(true);
    connect(m_pushInkTimer, &QTimer::timeout, this, &AirInkDialog::onPushInkTimerTimeout);

    m_statusTimer->start(200); // Poll status every 200ms
}

AirInkDialog::~AirInkDialog()
{
    delete ui;
}

void AirInkDialog::initDialog()
{
    loadParams();
    updateStatusDisplays();

    // Disable controls based on master switches
    ui->groupBox_3->findChild<QPushButton*>("buttonOut1")->setEnabled(ui->checkEnableSupply->isChecked());
    ui->groupBox_3->findChild<QPushButton*>("buttonOut2")->setEnabled(ui->checkEnableSupply->isChecked());
    
    bool pressEnabled = !ui->checkEnablePress->isChecked();
    ui->groupBox_3->findChild<QPushButton*>("buttonOut3")->setEnabled(pressEnabled);
    ui->groupBox_3->findChild<QPushButton*>("buttonOut4")->setEnabled(pressEnabled);
    ui->groupBox_3->findChild<QPushButton*>("buttonOut5")->setEnabled(pressEnabled);
    ui->groupBox_3->findChild<QPushButton*>("buttonOut6")->setEnabled(pressEnabled);
}

void AirInkDialog::loadParams()
{
    for (int i = 0; i < 2; i++) {
        ui->spinBoxPress1->setValue(g_sysParam.fAirCtlPress[0][i]);
        ui->spinBoxSafePress1->setValue(g_sysParam.fSafeAirPress[0][i]);
        ui->spinBoxInkTemp1->setValue(g_sysParam.fInkTemp[0][i]);
    }

    ui->checkEnableSupply->setChecked(g_sysParam.nIoOption & 0x1);
    ui->checkEnablePress->setChecked(g_sysParam.nIoOption & 0x2);
    ui->checkEnableAutoPress->setChecked(g_sysParam.nIoOption & 0x4);
}

void AirInkDialog::updateStatusDisplays()
{
    if (!g_pSysInfo) return;

    ui->labelCurrentPress1->setText(QString::number(g_pSysInfo->InkCtlInfo[0].fInkAirPressure[0], 'f', 1));
    ui->labelCurrentPress2->setText(QString::number(g_pSysInfo->InkCtlInfo[0].fInkAirPressure[1], 'f', 1));

    // Update ink level indicators
    const QColor errorColor(Qt::red);
    const QColor normalColor(Qt::green);
    
    auto setLabelColor = [](QLabel* label, bool isError) {
        label->setStyleSheet(isError ? "background-color: red;" : "background-color: green;");
    };

    setLabelColor(ui->labelInkLevel1, g_pSysInfo->nIoState[0] & (1 << 16));
    setLabelColor(ui->labelInkLevel2, g_pSysInfo->nIoState[0] & (1 << 17));
    setLabelColor(ui->labelInkLevel3, g_pSysInfo->nIoState[0] & (1 << 18));
    setLabelColor(ui->labelInkLevel4, g_pSysInfo->nIoState[0] & (1 << 19));
    setLabelColor(ui->labelInkLevelProtect, g_pSysInfo->nIoState[0] & (1 << 21));

    // Update button states from hardware
    for(int i=0; i<6; ++i) {
        updateOutputButton(i, g_pSysInfo->nIoState[0] & (1 << i));
    }
}

void AirInkDialog::updateOutputButton(int index, bool active)
{
    QPushButton* buttons[] = {
        ui->buttonOut1, ui->buttonOut2, ui->buttonOut3,
        ui->buttonOut4, ui->buttonOut5, ui->buttonOut6
    };
    const char* names[] = {"J58", "J59", "J61", "J62", "J63", "J64"};
    
    if (index < 0 || index >= 6) return;

    m_outputStates[index] = active;
    buttons[index]->setText(QString("%1 %2").arg(active ? "关闭" : "打开").arg(names[index]));
}

void AirInkDialog::onStatusTimerTimeout()
{
    updateStatusDisplays();
}

void AirInkDialog::onPushInkTimerTimeout()
{
    MVT_SetOutPut(0, 0x8, FALSE); // Turn off push ink valve
    m_isPushingInk = false;
    ui->buttonPushInk->setText("开始压墨");
}

void AirInkDialog::on_buttonSetParams_clicked()
{
    if (!ui->checkEnablePress->isChecked()) {
        QMessageBox::warning(this, "警告", "请先打开负压总开关。");
        return;
    }

    g_sysParam.fAirCtlPress[0][0] = ui->spinBoxPress1->value();
    g_sysParam.fSafeAirPress[0][0] = ui->spinBoxSafePress1->value();
    g_sysParam.fInkTemp[0][0] = ui->spinBoxInkTemp1->value();
    g_sysParam.fAirCtlPress[0][1] = ui->spinBoxPress2->value();
    g_sysParam.fSafeAirPress[0][1] = ui->spinBoxSafePress2->value();
    g_sysParam.fInkTemp[0][1] = ui->spinBoxInkTemp2->value();

    if (DEV_UpdateParam(&g_sysParam)) {
        if (!MVT_SetAirPressCtlVal(0))
            QMessageBox::critical(this, "错误", "设置温度气压失败。");
        else
            QMessageBox::information(this, "成功", "参数设置成功。");
    } else {
        QMessageBox::critical(this, "错误", "更新系统参数失败。");
    }
}

void AirInkDialog::on_buttonPushInk_clicked()
{
    if (m_isPushingInk) {
        m_pushInkTimer->stop();
        onPushInkTimerTimeout(); // Manually call to stop and reset
    } else {
        MVT_SetOutPut(0, 0x8, TRUE); // Turn on push ink valve
        int duration = ui->spinBoxPushTime->value() * 1000;
        m_pushInkTimer->start(duration);
        m_isPushingInk = true;
        ui->buttonPushInk->setText("停止压墨");
    }
}

void AirInkDialog::on_checkEnableSupply_toggled(bool checked)
{
    if (checked) g_sysParam.nIoOption |= 0x1;
    else g_sysParam.nIoOption &= ~0x1;
    
    ui->buttonOut1->setEnabled(checked);
    ui->buttonOut2->setEnabled(checked);

    DEV_UpdateParam(&g_sysParam);
    MVT_SetOutPutConfig(0);
}

void AirInkDialog::on_checkEnablePress_toggled(bool checked)
{
    if (checked) g_sysParam.nIoOption |= 0x2;
    else g_sysParam.nIoOption &= ~0x2;

    DEV_UpdateParam(&g_sysParam);
    if (!MVT_SetOutPutConfig(0)) {
        QMessageBox::critical(this, "错误", "MVT_SetOutPutConfig 调用失败。");
    }
}

void AirInkDialog::on_checkEnableAutoPress_toggled(bool checked)
{
    if (checked) g_sysParam.nIoOption |= 0x4;
    else g_sysParam.nIoOption &= ~0x4;

    bool manualEnabled = !checked;
    ui->buttonOut3->setEnabled(manualEnabled);
    ui->buttonOut4->setEnabled(manualEnabled);
    ui->buttonOut5->setEnabled(manualEnabled);
    ui->buttonOut6->setEnabled(manualEnabled);
    
    DEV_UpdateParam(&g_sysParam);
    MVT_SetOutPutConfig(0);
}

void AirInkDialog::on_buttonOut1_clicked() { MVT_SetOutPut(0, 0x1, !m_outputStates[0]); }
void AirInkDialog::on_buttonOut2_clicked() { MVT_SetOutPut(0, 0x2, !m_outputStates[1]); }
void AirInkDialog::on_buttonOut3_clicked() { MVT_SetOutPut(0, 0x4, !m_outputStates[2]); }
void AirInkDialog::on_buttonOut4_clicked() { MVT_SetOutPut(0, 0x8, !m_outputStates[3]); }
void AirInkDialog::on_buttonOut5_clicked() { MVT_SetOutPut(0, 0x10, !m_outputStates[4]); }
void AirInkDialog::on_buttonOut6_clicked() { MVT_SetOutPut(0, 0x20, !m_outputStates[5]); }


void AirInkDialog::on_buttonClose_clicked()
{
    this->accept();
}

void AirInkDialog::closeEvent(QCloseEvent *event)
{
    m_statusTimer->stop();
    event->accept();
}
