#include "ReloadWaveDialog.h"
#include "ui_ReloadWaveDialog.h"
#include "../Inc/RYPrtCtler.h" // For MAX_COLORS and MAX_GROUP
#include <QFileDialog>

ReloadWaveDialog::ReloadWaveDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ReloadWaveDialog)
{
    ui->setupUi(this);
    initDialog();

    connect(ui->buttonBrowse, &QPushButton::clicked, this, &ReloadWaveDialog::on_buttonBrowse_clicked);
}

ReloadWaveDialog::~ReloadWaveDialog()
{
    delete ui;
}

void ReloadWaveDialog::initDialog()
{
    // Populate color combo box
    ui->comboColor->clear();
    for (int i = 0; i < MAX_COLORS; ++i) {
        ui->comboColor->addItem(QString("颜色 %1").arg(i), i);
    }

    // Populate group combo box
    ui->comboGroup->clear();
    for (int i = 0; i < MAX_GROUP; ++i) {
        ui->comboGroup->addItem(QString("组 %1").arg(i), i);
    }
}

int ReloadWaveDialog::selectedColorIndex() const
{
    return ui->comboColor->currentData().toInt();
}

int ReloadWaveDialog::selectedGroupIndex() const
{
    return ui->comboGroup->currentData().toInt();
}

QString ReloadWaveDialog::selectedWavePath() const
{
    return ui->lineEditPath->text();
}

void ReloadWaveDialog::on_buttonBrowse_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(
        this,
        "选择波形文件",
        "",
        "波形文件 (*.wfm *.bin);;所有文件 (*.*)"
    );

    if (!filePath.isEmpty()) {
        ui->lineEditPath->setText(filePath);
    }
}

void ReloadWaveDialog::on_buttonBox_accepted()
{
    if (selectedWavePath().isEmpty()) {
        // Optionally show a message to the user
        // QMessageBox::warning(this, "输入错误", "请选择一个波形文件。");
        // Don't close the dialog
        return;
    }
    accept();
}
