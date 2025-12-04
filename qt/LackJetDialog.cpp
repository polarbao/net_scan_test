#include "LackJetDialog.h"
#include "ui_LackJetDialog.h"
#include "../Inc/RYPrtCtler.h" // For MAX_COLORS, MAX_GROUP
#include <QMessageBox>

// Reference global variables
extern RYUSR_PARAM g_sysParam;

LackJetDialog::LackJetDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LackJetDialog)
{
    ui->setupUi(this);
    initDialog();

    // Connect signals and slots
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &LackJetDialog::reject);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &LackJetDialog::accept);

    connect(ui->comboColor, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &LackJetDialog::on_comboColor_currentIndexChanged);
    connect(ui->comboGroup, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &LackJetDialog::on_comboGroup_currentIndexChanged);

    connect(ui->buttonAdd, &QPushButton::clicked, this, &LackJetDialog::on_buttonAdd_clicked);
    connect(ui->buttonRemove, &QPushButton::clicked, this, &LackJetDialog::on_buttonRemove_clicked);
    connect(ui->buttonClearAll, &QPushButton::clicked, this, &LackJetDialog::on_buttonClearAll_clicked);
}

LackJetDialog::~LackJetDialog()
{
    delete ui;
}

void LackJetDialog::initDialog()
{
    populateComboBoxes();
    updateJetList();
}

void LackJetDialog::populateComboBoxes()
{
    ui->comboColor->clear();
    for (int i = 0; i < MAX_COLORS; ++i) {
        ui->comboColor->addItem(QString("颜色 %1").arg(i), i);
    }

    ui->comboGroup->clear();
    for (int i = 0; i < MAX_GROUP; ++i) {
        ui->comboGroup->addItem(QString("组 %1").arg(i), i);
    }
}

void LackJetDialog::updateJetList()
{
    ui->listWidgetJets->clear();
    int color = ui->comboColor->currentIndex();
    int group = ui->comboGroup->currentIndex();

    if (color < 0 || group < 0) return;

    int count = g_sysParam.nLackJetCount[color][group];
    for (int i = 0; i < count; ++i) {
        int jet = g_sysParam.nLackJetTbl[color][group][i];
        ui->listWidgetJets->addItem(QString::number(jet));
    }
}

void LackJetDialog::on_buttonAdd_clicked()
{
    int color = ui->comboColor->currentIndex();
    int group = ui->comboGroup->currentIndex();
    if (color < 0 || group < 0) return;

    int& count = g_sysParam.nLackJetCount[color][group];
    if (count >= 32) {
        QMessageBox::warning(this, "警告", "缺嘴补偿数量已达上限 (32)。");
        return;
    }

    int newJet = ui->spinBoxLackJet->value();

    // Check for duplicates
    for(int i=0; i < count; ++i) {
        if(g_sysParam.nLackJetTbl[color][group][i] == newJet) {
            return; // Already exists
        }
    }

    g_sysParam.nLackJetTbl[color][group][count] = newJet;
    count++;

    updateJetList();
}

void LackJetDialog::on_buttonRemove_clicked()
{
    int color = ui->comboColor->currentIndex();
    int group = ui->comboGroup->currentIndex();
    if (color < 0 || group < 0) return;

    QListWidgetItem *selectedItem = ui->listWidgetJets->currentItem();
    if (!selectedItem) return;

    int jetToRemove = selectedItem->text().toInt();
    int& count = g_sysParam.nLackJetCount[color][group];
    int foundIndex = -1;

    // Find the jet in the array
    for (int i = 0; i < count; ++i) {
        if (g_sysParam.nLackJetTbl[color][group][i] == jetToRemove) {
            foundIndex = i;
            break;
        }
    }

    // Remove it by shifting elements
    if (foundIndex != -1) {
        for (int i = foundIndex; i < count - 1; ++i) {
            g_sysParam.nLackJetTbl[color][group][i] = g_sysParam.nLackJetTbl[color][group][i + 1];
        }
        count--;
        g_sysParam.nLackJetTbl[color][group][count] = 0; // Clear the last element
    }

    updateJetList();
}

void LackJetDialog::on_buttonClearAll_clicked()
{
    if (QMessageBox::question(this, "确认", "确定要清除所有缺嘴补偿数据吗？", QMessageBox::Yes | QMessageBox::No) == QMessageBox::No) {
        return;
    }
    
    for (int c = 0; c < MAX_COLORS; ++c) {
        for (int g = 0; g < MAX_GROUP; ++g) {
            g_sysParam.nLackJetCount[c][g] = 0;
            memset(g_sysParam.nLackJetTbl[c][g], 0, sizeof(g_sysParam.nLackJetTbl[c][g]));
        }
    }

    updateJetList();
}

void LackJetDialog::on_comboColor_currentIndexChanged(int index)
{
    updateJetList();
}

void LackJetDialog::on_comboGroup_currentIndexChanged(int index)
{
    updateJetList();
}
