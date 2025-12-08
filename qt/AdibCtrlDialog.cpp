#include "AdibCtrlDialog.h"
#include "ui_AdibCtrlDialog.h"
#include <QMessageBox>
#include <QCloseEvent>
#include <QDebug>

// Extern global variables from the SDK and other parts of the application
extern LPPRINTER_INFO g_pSysInfo;
extern ADIB_PARAM adibCurState; // Used for polling
extern ADIB_PARAM _adibSettings; // Used for sending commands

// Helper function to initialize ADIB parameters, similar to the MFC version
void InitAdibParam()
{
    memset(&_adibSettings, 0, sizeof(ADIB_PARAM));
    _adibSettings.fAirThreshold = 0.5f;
    for (int i = 0; i < 8; i++)
    {
        _adibSettings.fcurAirPress[i] = 5.2f;
        _adibSettings.fcurvoltage[i] = 24.0f;
    }
}

AdibCtrlDialog::AdibCtrlDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AdibCtrlDialog),
    m_timer(new QTimer(this)),
    m_isCleanMode(false),
    m_isCleanPump1(false),
    m_isCleanPump2(false),
    m_isCleanHead(false),
    m_isConnected(false)
{
    ui->setupUi(this);
    InitAdibParam(); // Initialize the settings structure
    initUI();

    // Connect signals and slots
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &AdibCtrlDialog::reject);
    connect(m_timer, &QTimer::timeout, this, &AdibCtrlDialog::updateState);
    connect(ui->applyButton, &QPushButton::clicked, this, &AdibCtrlDialog::on_applyButton_clicked);
    connect(ui->enableSetCheckBox, &QCheckBox::toggled, this, &AdibCtrlDialog::on_enableSetCheckBox_toggled);
    connect(ui->cleanModeButton, &QPushButton::clicked, this, &AdibCtrlDialog::on_cleanModeButton_clicked);
    connect(ui->cleanPump1Button, &QPushButton::clicked, this, &AdibCtrlDialog::on_cleanPump1Button_clicked);
    connect(ui->cleanPump2Button, &QPushButton::clicked, this, &AdibCtrlDialog::on_cleanPump2Button_clicked);
    connect(ui->cleanHeadButton, &QPushButton::clicked, this, &AdibCtrlDialog::on_cleanHeadButton_clicked);
    connect(ui->inkSupplyButton, &QPushButton::clicked, this, &AdibCtrlDialog::on_inkSupplyButton_clicked);
    connect(ui->resetErrorButton, &QPushButton::clicked, this, &AdibCtrlDialog::on_resetErrorButton_clicked);
    connect(ui->iicReadButton, &QPushButton::clicked, this, &AdibCtrlDialog::on_iicReadButton_clicked);
    connect(ui->iicSaveButton, &QPushButton::clicked, this, &AdibCtrlDialog::on_iicSaveButton_clicked);


    // Initialize device and start polling
    if (g_pSysInfo && (g_pSysInfo->nSysFunOption & 0x4)) { // Check if ADIB is enabled in system params
        // This command seems to be for initialization or reading initial settings
        ::DEV_AdibControl(&_adibSettings, 0x40000, 0, NULL, 0);
        m_timer->start(1000); // Poll every 1 second
    } else {
        QMessageBox::warning(this, "Not Enabled", "ADIB control is not enabled in system parameters.");
        ui->groupBox_params->setEnabled(false);
        ui->groupBox_controls->setEnabled(false);
    }
}

AdibCtrlDialog::~AdibCtrlDialog()
{
    if (m_timer->isActive()) {
        m_timer->stop();
    }
    delete ui;
}

void AdibCtrlDialog::initUI()
{
    // Setup table widget
    QStringList headers = { "Parameter", "1", "2", "3", "4", "5", "6", "7", "8" };
    ui->paramTableWidget->setColumnCount(headers.size());
    ui->paramTableWidget->setHorizontalHeaderLabels(headers);

    QStringList verticalHeaders = { "Voltage (V)", "Temperature (°C)", "Pressure (kpa)" };
    ui->paramTableWidget->setRowCount(verticalHeaders.size());
    ui->paramTableWidget->setVerticalHeaderLabels(verticalHeaders);

    for (int i = 0; i < verticalHeaders.size(); ++i) {
        for (int j = 1; j < headers.size(); ++j) {
            ui->paramTableWidget->setItem(i, j, new QTableWidgetItem());
        }
    }
    ui->paramTableWidget->setEnabled(false);
    ui->applyButton->hide();

    // Set initial values from g_pSysInfo if available
    if (g_pSysInfo) {
        ui->airHoldLineEdit->setText(QString::number(g_pSysInfo->nAdibInfo.fAirThreshold, 'f', 2));
        ui->inkTimeLineEdit->setText(QString::number(g_pSysInfo->nAdibInfo.fPressInkTime, 'f', 2));
        ui->supplyTimeLineEdit->setText(QString::number(g_pSysInfo->nAdibInfo.fInkSupplyTime, 'f', 2));
    }
}

void AdibCtrlDialog::closeEvent(QCloseEvent *event)
{
    // Ensure ink supply is turned off when closing the dialog
    ::DEV_AdibControl(&_adibSettings, 0xA00, 1, NULL, 0);
    QDialog::closeEvent(event);
}

void AdibCtrlDialog::updateState()
{
    if (ui->enableSetCheckBox->isChecked()) {
        return; // Don't poll when in edit mode
    }

    // Poll the device state
    UINT nIoOption = 0x10; // Read command
    m_isConnected = ::DEV_AdibControl(&adibCurState, nIoOption, 0, NULL, 0);

    // Update UI with the new state
    updateUI(m_isConnected);
}

void AdibCtrlDialog::updateUI(bool isConnected)
{
    if (isConnected) {
        ui->statusLabel->setText(QString("Status: Connected [FW Ver: %1]").arg(g_pSysInfo->nAdibInfo.nFMver, 8, 16, QChar('0')));
    } else {
        ui->statusLabel->setText("Status: Disconnected");
    }

    LPADIB_PARAM currentParams = &g_pSysInfo->nAdibInfo;

    // Update table
    for (int i = 0; i < 8; ++i) {
        ui->paramTableWidget->item(0, i + 1)->setText(QString::number(currentParams->fcurvoltage[i], 'f', 2));
        ui->paramTableWidget->item(1, i + 1)->setText(QString::number(currentParams->fcurInkTankTemp[i], 'f', 2));
        ui->paramTablewidget->item(2, i + 1)->setText(QString::number(currentParams->fcurAirPress[i], 'f', 2));
    }

    // Update ink status display (placeholder)
    // The original MFC control is complex. For now, just display the raw status value.
    ui->inkStatusLabel->setText(QString("Logic Input Status: 0x%1").arg(currentParams->nLgStatus, 6, 16, QChar('0')));

    // Replicate the complex message box logic from MFC
    // This part is tricky and may need adjustment based on expected device behavior
    static bool bLink1WinOpen = false;
    if ((currentParams->nStatus & 0x400) && !bLink1WinOpen) {
        bLink1WinOpen = true;
        int ret = QMessageBox::question(this, "Continue Ink Supply?", "Continue ink supply for channel 1?", QMessageBox::Yes | QMessageBox::No);
        if (ret == QMessageBox::Yes) {
            sendCommand(0x100 | 0x400, true);
        }
        bLink1WinOpen = false;
    }

    static bool bLink2WinOpen = false;
    if ((currentParams->nStatus & 0x800) && !bLink2WinOpen) {
        bLink2WinOpen = true;
        int ret = QMessageBox::question(this, "Continue Ink Supply?", "Continue ink supply for channel 2?", QMessageBox::Yes | QMessageBox::No);
        if (ret == QMessageBox::Yes) {
            sendCommand(0x100 | 0x400, true); // Assuming same command for both
        }
        bLink2WinOpen = false;
    }
}


void AdibCtrlDialog::on_applyButton_clicked()
{
    if (!ui->enableSetCheckBox->isChecked()) return;

    UINT nIoOption = 0;
    ADIB_PARAM newSettings = _adibSettings; // Start with current settings

    // Read from table
    for (int i = 0; i < 8; ++i) {
        newSettings.fcurvoltage[i] = ui->paramTableWidget->item(0, i + 1)->text().toFloat();
        newSettings.fcurInkTankTemp[i] = ui->paramTableWidget->item(1, i + 1)->text().toFloat();
        newSettings.fcurAirPress[i] = ui->paramTableWidget->item(2, i + 1)->text().toFloat();
    }
    // Assuming if any value in a row is changed, the whole row is sent
    nIoOption |= 0x2;  // Temp
    nIoOption |= 0x4;  // Air Press
    nIoOption |= 0x10; // Voltage

    // Read from line edits
    newSettings.fAirThreshold = ui->airHoldLineEdit->text().toFloat();
    nIoOption |= 0x20;

    newSettings.fPressInkTime = ui->inkTimeLineEdit->text().toFloat();
    nIoOption |= 0x10000;
    
    newSettings.fInkSupplyTime = ui->supplyTimeLineEdit->text().toFloat();
    nIoOption |= 0x20000;

    // Send the settings to the device
    sendCommand(nIoOption, true, &newSettings);

    // Save to IIC as well
    on_iicSaveButton_clicked();

    // Disable editing
    ui->enableSetCheckBox->setChecked(false);
}

void AdibCtrlDialog::on_enableSetCheckBox_toggled(bool checked)
{
    ui->paramTableWidget->setEnabled(checked);
    ui->airHoldLineEdit->setEnabled(checked);
    ui->inkTimeLineEdit->setEnabled(checked);
    ui->supplyTimeLineEdit->setEnabled(checked);
    ui->applyButton->setVisible(checked);
}

// --- Manual Control Buttons ---

void AdibCtrlDialog::on_cleanModeButton_clicked()
{
    m_isCleanMode = !m_isCleanMode;
    sendCommand(m_isCleanMode ? 0x80 : 0x40, true);
    ui->cleanModeButton->setText(m_isCleanMode ? "Clean Mode Off" : "Clean Mode On");
}

void AdibCtrlDialog::on_cleanPump1Button_clicked()
{
    m_isCleanPump1 = !m_isCleanPump1;
    sendCommand(m_isCleanPump1 ? 0x4000 : 0x8000, true);
    ui->cleanPump1Button->setText(m_isCleanPump1 ? "Clean Pump 1 Off" : "Clean Pump 1 On");
}

void AdibCtrlDialog::on_cleanPump2Button_clicked()
{
    m_isCleanPump2 = !m_isCleanPump2;
    sendCommand(m_isCleanPump2 ? 0x1000 : 0x2000, true);
    ui->cleanPump2Button->setText(m_isCleanPump2 ? "Clean Pump 2 Off" : "Clean Pump 2 On");
}

void AdibCtrlDialog::on_cleanHeadButton_clicked()
{
    m_isCleanHead = !m_isCleanHead;
    // This function in MFC also called MVT_SetOutPut. This dependency should be handled carefully.
    // For now, just sending the ADIB command.
    // MVT_SetOutPut(0, 0x1, m_isCleanHead);
    // MVT_SetOutPut(0, 0x2, m_isCleanHead);
    qDebug() << "MVT_SetOutPut would be called here. This function must be available globally.";
    sendCommand(m_isCleanHead ? 0x4000 : 0x8000, true); // Re-using pump1 commands as per MFC code
    ui->cleanHeadButton->setText(m_isCleanHead ? "Clean Head Off" : "Clean Head On");
}

void AdibCtrlDialog::on_inkSupplyButton_clicked()
{
    // This button resets some error flags and sends an ink supply command
    sendCommand(0x100 | 0x400, true);
}

void AdibCtrlDialog::on_resetErrorButton_clicked()
{
    // There is no direct "Reset Error" command in the MFC code.
    // This might be a placeholder or intended to clear software flags.
    QMessageBox::information(this, "Info", "Error reset functionality not defined in original code.");
}

void AdibCtrlDialog::on_iicReadButton_clicked()
{
    // The MFC code combines this with apply. Here it's separate.
    // 0x40000000 = Read IIC
    sendCommand(0x40000000, true);
    QMessageBox::information(this, "IIC Read", "Sent IIC read command. Values should update shortly.");
}

void AdibCtrlDialog::on_iicSaveButton_clicked()
{
    // 0x80000000 = Save IIC
    sendCommand(0x80000000, true);
    QMessageBox::information(this, "IIC Save", "Sent IIC save command.");
}


void AdibCtrlDialog::sendCommand(UINT command, bool write, const ADIB_PARAM* params)
{
    // A wrapper for the main SDK call
    int writeFlag = write ? 1 : 0;
    ADIB_PARAM* settings = params ? const_cast<ADIB_PARAM*>(params) : &_adibSettings;

    if (!::DEV_AdibControl(settings, command, writeFlag, NULL, 0)) {
       // QMessageBox::warning(this, "Command Failed", QString("ADIB command 0x%1 failed to execute.").arg(command, 8, 16, QChar('0')));
    }
}
