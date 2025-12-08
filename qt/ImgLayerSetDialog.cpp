#include "ImgLayerSetDialog.h"
#include "ui_ImgLayerSetDialog.h"
#include <QDebug>

extern PRTIMG_LAYER g_PrtImgLayer;

ImgLayerSetDialog::ImgLayerSetDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ImgLayerSetDialog)
{
    ui->setupUi(this);
    loadParams();
}

ImgLayerSetDialog::~ImgLayerSetDialog()
{
    delete ui;
}

void ImgLayerSetDialog::loadParams()
{
    // Populate combo boxes
    ui->prtDirComboBox->addItems({"Forward", "Reverse"});
    ui->featherModeComboBox->addItems({"Mode 0", "Mode 1", "Mode 2", "Mode 3", "Mode 4", "Mode 5", "Mode 6", "Custom"});
    ui->imgTypeComboBox->addItems({"BMP", "PRT", "CLI"});
    ui->scanCtlValueComboBox->addItems({"Mode 0", "Mode 1", "Mode 2"});
    for (int i = 1; i <= 8; ++i) {
        ui->mulityInkComboBox->addItem(QString("%1x").arg(i));
        ui->passVolComboBox->addItem(QString("1/%1").arg(i));
    }


    // Set values from global g_PrtImgLayer
    ui->startJetOffLineEdit->setText(QString::number(g_PrtImgLayer.nStartJetOffset));
    ui->prtDirComboBox->setCurrentIndex(g_PrtImgLayer.nPrtDir);
    ui->xPosOffLineEdit->setText(QString::number(g_PrtImgLayer.fPrtXOffet));
    ui->prtYOffetLineEdit->setText(QString::number(g_PrtImgLayer.fPrtYOffet));
    ui->colorCntsLineEdit->setText(QString::number(g_PrtImgLayer.nColorCnts));
    ui->xDPILineEdit->setText(QString::number(g_PrtImgLayer.nXDPI));
    ui->yDPILineEdit->setText(QString::number(g_PrtImgLayer.nYDPI));
    ui->startPassIndexLineEdit->setText(QString::number(g_PrtImgLayer.nStartPassIndex));
    ui->featherModeComboBox->setCurrentIndex(g_PrtImgLayer.nFeatherMode);
    ui->customFeatherJetsLineEdit->setText(QString::number(g_PrtImgLayer.nCustomFeatherJets));
    ui->edgeScalePixelLineEdit->setText(QString::number(g_PrtImgLayer.nEdgeScalePixel));
    ui->rotateAngleLineEdit->setText(QString::number(g_PrtImgLayer.fRotateAngle));
    ui->dstXScaleLineEdit->setText(QString::number(g_PrtImgLayer.fDstXScale));
    ui->dstYScaleLineEdit->setText(QString::number(g_PrtImgLayer.fDstYScale));
    ui->layerDensityLineEdit->setText(QString::number(g_PrtImgLayer.fLayerDensity));
    ui->edgeDensityLineEdit->setText(QString::number(g_PrtImgLayer.fEdgeDensity));
    ui->xScanSpdLineEdit->setText(QString::number(g_PrtImgLayer.fXScanSpd));
    ui->scanCtlValueComboBox->setCurrentIndex(g_PrtImgLayer.nScanCtlValue);
    ui->imgTypeComboBox->setCurrentIndex(g_PrtImgLayer.nImgType);

    ui->mulityInkComboBox->setCurrentIndex(g_PrtImgLayer.nInkMultication > 0 ? g_PrtImgLayer.nInkMultication - 1 : 0);
    ui->passVolComboBox->setCurrentIndex(g_PrtImgLayer.nPassVolRate > 0 ? g_PrtImgLayer.nPassVolRate - 1 : 0);

    // Flags
    ui->doubleDirCheckBox->setChecked(g_PrtImgLayer.nPrtFlag & 0x1);
    ui->yReverseCheckBox->setChecked(g_PrtImgLayer.nPrtFlag & 0x2);
    ui->xInsertCheckBox->setChecked(g_PrtImgLayer.nPrtFlag & 0x4);
    ui->switchByWhiteCheckBox->setChecked(g_PrtImgLayer.nPrtFlag & 0x8);
    ui->edgeNoReduceCheckBox->setChecked(g_PrtImgLayer.nPrtFlag & 0x10);
    ui->prtAreaCheckBox->setChecked(g_PrtImgLayer.nPrtFlag & 0x20);
    ui->grayBitsCheckBox->setChecked(g_PrtImgLayer.nGrayBits == 2);
}

void ImgLayerSetDialog::saveParams()
{
    g_PrtImgLayer.nStartJetOffset = ui->startJetOffLineEdit->text().toInt();
    g_PrtImgLayer.nPrtDir = ui->prtDirComboBox->currentIndex();
    g_PrtImgLayer.fPrtXOffet = ui->xPosOffLineEdit->text().toFloat();
    g_PrtImgLayer.fPrtYOffet = ui->prtYOffetLineEdit->text().toFloat();
    g_PrtImgLayer.nColorCnts = ui->colorCntsLineEdit->text().toInt();
    if (g_PrtImgLayer.nColorCnts < 1) g_PrtImgLayer.nColorCnts = 1;
    if (g_PrtImgLayer.nColorCnts > 16) g_PrtImgLayer.nColorCnts = 16;
    g_PrtImgLayer.nXDPI = ui->xDPILineEdit->text().toFloat();
    g_PrtImgLayer.nYDPI = ui->yDPILineEdit->text().toLong();
    g_PrtImgLayer.nStartPassIndex = ui->startPassIndexLineEdit->text().toInt();
    g_PrtImgLayer.nFeatherMode = ui->featherModeComboBox->currentIndex();
    g_PrtImgLayer.nCustomFeatherJets = ui->customFeatherJetsLineEdit->text().toInt();
    g_PrtImgLayer.nEdgeScalePixel = ui->edgeScalePixelLineEdit->text().toInt();
    g_PrtImgLayer.fRotateAngle = ui->rotateAngleLineEdit->text().toFloat();
    g_PrtImgLayer.fDstXScale = ui->dstXScaleLineEdit->text().toFloat();
    g_PrtImgLayer.fDstYScale = ui->dstYScaleLineEdit->text().toFloat();
    g_PrtImgLayer.fLayerDensity = ui->layerDensityLineEdit->text().toFloat();
    g_PrtImgLayer.fEdgeDensity = ui->edgeDensityLineEdit->text().toFloat();
    g_PrtImgLayer.fXScanSpd = ui->xScanSpdLineEdit->text().toFloat();
    g_PrtImgLayer.nScanCtlValue = ui->scanCtlValueComboBox->currentIndex();
    g_PrtImgLayer.nImgType = ui->imgTypeComboBox->currentIndex();

    g_PrtImgLayer.nInkMultication = ui->mulityInkComboBox->currentIndex() + 1;
    g_PrtImgLayer.nPassVolRate = ui->passVolComboBox->currentIndex() + 1;

    // Flags
    g_PrtImgLayer.nPrtFlag &= ~0xff; // Clear the lower 8 bits before setting them
    if (ui->doubleDirCheckBox->isChecked()) g_PrtImgLayer.nPrtFlag |= 0x1;
    if (ui->yReverseCheckBox->isChecked()) g_PrtImgLayer.nPrtFlag |= 0x2;
    if (ui->xInsertCheckBox->isChecked()) g_PrtImgLayer.nPrtFlag |= 0x4;
    if (ui->switchByWhiteCheckBox->isChecked()) g_PrtImgLayer.nPrtFlag |= 0x8;
    if (ui->edgeNoReduceCheckBox->isChecked()) g_PrtImgLayer.nPrtFlag |= 0x10;
    if (ui->prtAreaCheckBox->isChecked()) g_PrtImgLayer.nPrtFlag |= 0x20;

    g_PrtImgLayer.nGrayBits = ui->grayBitsCheckBox->isChecked() ? 2 : 1;
}

void ImgLayerSetDialog::accept()
{
    saveParams();
    QDialog::accept();
}
