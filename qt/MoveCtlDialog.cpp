#include "MoveCtlDialog.h"
#include "ui_MoveCtlDialog.h"
#include "../Inc/RYPrtCtler.h"

// 外部全局变量声明
extern MOV_Config g_movConfig;

MoveCtlDialog::MoveCtlDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MoveCtlDialog)
{
    ui->setupUi(this);
    initDialog();

    // Connect signals to slots
    connect(ui->buttonUp, &QPushButton::clicked, this, &MoveCtlDialog::on_buttonUp_clicked);
    connect(ui->buttonDown, &QPushButton::clicked, this, &MoveCtlDialog::on_buttonDown_clicked);
    connect(ui->buttonLeft, &QPushButton::clicked, this, &MoveCtlDialog::on_buttonLeft_clicked);
    connect(ui->buttonRight, &QPushButton::clicked, this, &MoveCtlDialog::on_buttonRight_clicked);
    connect(ui->buttonStop, &QPushButton::clicked, this, &MoveCtlDialog::on_buttonStop_clicked);
    connect(ui->buttonReset, &QPushButton::clicked, this, &MoveCtlDialog::on_buttonReset_clicked);
    connect(ui->buttonApply, &QPushButton::clicked, this, &MoveCtlDialog::on_buttonApply_clicked);
    connect(ui->buttonClose, &QPushButton::clicked, this, &MoveCtlDialog::on_buttonClose_clicked);
}

MoveCtlDialog::~MoveCtlDialog()
{
    delete ui;
}

void MoveCtlDialog::initDialog()
{
    // Enable the motion system before doing anything
#ifdef RY_MOVE_CTL
    DEM_EnableRun();
#endif
    loadParamsFromConfig();
}

void MoveCtlDialog::loadParamsFromConfig()
{
    // Load X-axis params
    ui->spinBoxXSpd->setValue(g_movConfig.fxMovSpd);
    ui->spinBoxXAcc->setValue(g_movConfig.fxMovAcc);
    ui->spinBoxXUnit->setValue(g_movConfig.fxMovUnit);
    ui->spinBoxXDpi->setValue(g_movConfig.fxSysdpi);
    ui->spinBoxXRate->setValue(g_movConfig.fxMovRate);
    ui->checkXReverse->setChecked(g_movConfig.fxIoOption & 0x01); // Assuming bit 0 is reverse flag

    // Load Y-axis params
    ui->spinBoxYSpd->setValue(g_movConfig.fyMovSpd);
    ui->spinBoxYAcc->setValue(g_movConfig.fyMovAcc);
    ui->spinBoxYUnit->setValue(g_movConfig.fyMovUnit);
    ui->spinBoxYDpi->setValue(g_movConfig.fySysdpi);
    ui->spinBoxYRate->setValue(g_movConfig.fyMovRate);
    ui->checkYReverse->setChecked(g_movConfig.fyIoOption & 0x01); // Assuming bit 0 is reverse flag
}

void MoveCtlDialog::saveParamsToConfig()
{
    // Save X-axis params
    g_movConfig.fxMovSpd = ui->spinBoxXSpd->value();
    g_movConfig.fxMovAcc = ui->spinBoxXAcc->value();
    g_movConfig.fxMovUnit = ui->spinBoxXUnit->value();
    g_movConfig.fxSysdpi = ui->spinBoxXDpi->value();
    g_movConfig.fxMovRate = ui->spinBoxXRate->value();
    if (ui->checkXReverse->isChecked()) {
        g_movConfig.fxIoOption |= 0x01;
    } else {
        g_movConfig.fxIoOption &= ~0x01;
    }

    // Save Y-axis params
    g_movConfig.fyMovSpd = ui->spinBoxYSpd->value();
    g_movConfig.fyMovAcc = ui->spinBoxYAcc->value();
    g_movConfig.fyMovUnit = ui->spinBoxYUnit->value();
    g_movConfig.fySysdpi = ui->spinBoxYDpi->value();
    g_movConfig.fyMovRate = ui->spinBoxYRate->value();
    if (ui->checkYReverse->isChecked()) {
        g_movConfig.fyIoOption |= 0x01;
    } else {
        g_movConfig.fyIoOption &= ~0x01;
    }
}

// Slots implementation

void MoveCtlDialog::on_buttonApply_clicked()
{
    saveParamsToConfig();
    // Maybe update the hardware config immediately
#ifdef RY_MOVE_CTL
    DEM_UpdateXMovCfg(g_movConfig.fxMovAcc, g_movConfig.fxIoOption);
    DEM_UpdateYMovCfg(g_movConfig.fyMovAcc, g_movConfig.fyIoOption);
#endif
}

void MoveCtlDialog::on_buttonUp_clicked() // Y- (Front)
{
#ifdef RY_MOVE_CTL
    bool isReversed = ui->checkYReverse->isChecked();
    int speed = mmToDots(g_movConfig.fyMovSpd, g_movConfig.fySysdpi) * 48;
    int steps = mmToDots(g_movConfig.fyMovUnit, g_movConfig.fySysdpi) * 48;
    DEM_MoveY(isReversed, speed, steps, false, false, g_movConfig.fyMovRate);
#endif
}

void MoveCtlDialog::on_buttonDown_clicked() // Y+ (Back)
{
#ifdef RY_MOVE_CTL
    bool isReversed = !ui->checkYReverse->isChecked(); // Opposite direction
    int speed = mmToDots(g_movConfig.fyMovSpd, g_movConfig.fySysdpi) * 48;
    int steps = mmToDots(g_movConfig.fyMovUnit, g_movConfig.fySysdpi) * 48;
    DEM_MoveY(isReversed, speed, steps, false, false, g_movConfig.fyMovRate);
#endif
}

void MoveCtlDialog::on_buttonLeft_clicked() // X-
{
#ifdef RY_MOVE_CTL
    bool isReversed = ui->checkXReverse->isChecked();
    int speed = mmToDots(g_movConfig.fxMovSpd, g_movConfig.fxSysdpi);
    int steps = mmToDots(g_movConfig.fxMovUnit, g_movConfig.fxSysdpi);
    DEM_MoveX(isReversed, speed, steps, false, g_movConfig.fxMovRate);
#endif
}

void MoveCtlDialog::on_buttonRight_clicked() // X+
{
#ifdef RY_MOVE_CTL
    bool isReversed = !ui->checkXReverse->isChecked(); // Opposite direction
    int speed = mmToDots(g_movConfig.fxMovSpd, g_movConfig.fxSysdpi);
    int steps = mmToDots(g_movConfig.fxMovUnit, g_movConfig.fxSysdpi);
    DEM_MoveX(isReversed, speed, steps, false, g_movConfig.fxMovRate);
#endif
}

void MoveCtlDialog::on_buttonStop_clicked()
{
#ifdef RY_MOVE_CTL
    DEM_StopAll(TRUE);
#endif
}

void MoveCtlDialog::on_buttonReset_clicked()
{
#ifdef RY_MOVE_CTL
    DEM_StopAll(TRUE);
    DEM_UpdateXMovCfg(g_movConfig.fxMovAcc, g_movConfig.fxIoOption);
    DEM_UpdateYMovCfg(g_movConfig.fyMovAcc, g_movConfig.fyIoOption);
    DEM_EnableRun();
#endif
}

void MoveCtlDialog::on_buttonClose_clicked()
{
    // Disable motion system on close to be safe
#ifdef RY_MOVE_CTL
    DEM_StopAll(TRUE);
    DEM_EnableRun(FALSE);
#endif
    this->accept();
}
