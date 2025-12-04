#include "VTSetDialog.h"
#include "ui_VTSetDialog.h"
#include <QDebug>
#include <QStringList>

// 引用在MainWindow.cpp中定义的全局变量
extern RYUSR_PARAM g_sysParam;
extern LPPRINTER_INFO g_pSysInfo;
extern BOOL g_bPHValid[MAX_PH_CNT];

VTSetDialog::VTSetDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::VTSetDialog),
    m_nTempCount(0),
    m_nVolCount(0),
    m_nSplit(1),
    m_nActivePHCount(0)
{
    ui->setupUi(this);
    initDialog();

    // 连接信号和槽
    connect(ui->buttonClose, &QPushButton::clicked, this, &VTSetDialog::on_buttonClose_clicked);
    connect(ui->buttonTempSet, &QPushButton::clicked, this, &VTSetDialog::on_buttonTempSet_clicked);
    connect(ui->buttonVolSet, &QPushButton::clicked, this, &VTSetDialog::on_buttonVolSet_clicked);
    connect(ui->comboPHID, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &VTSetDialog::on_comboPHID_currentIndexChanged);
}

VTSetDialog::~VTSetDialog()
{
    delete ui;
}

void VTSetDialog::initDialog()
{
    if (!g_pSysInfo) {
        qWarning("VTSetDialog: g_pSysInfo is null, cannot initialize.");
        return;
    }

    // 从硬件信息中获取温度和电压控制点的数量
    m_nTempCount = g_pSysInfo->clrph_info[0].nTmpCtlCnt;
    m_nVolCount = g_pSysInfo->clrph_info[0].nVolCnt;
    
    // 特定喷头类型的特殊处理 (Xaar)
    m_nSplit = (g_pSysInfo->clrph_info[0].nPhType == 19) ? 4 : 1;
    m_nVolCount /= m_nSplit;

    populateComboBoxes();
    setupVTListTree();
    redrawVTList();
    
    // 设置SpinBox的范围
    ui->spinBoxTempVal->setRange(0, 100);
    ui->spinBoxVolVal->setRange(0, 100);

    // 根据第一个喷头的值初始化UI
    if (m_nActivePHCount > 0) {
        int firstPH = m_nPhTBL[0];
        if (m_nTempCount > 0) {
            ui->spinBoxTempVal->setValue(g_sysParam.phctl_param[firstPH].fDestTemp[0]);
        }
        if (m_nVolCount > 0) {
            ui->spinBoxVolVal->setValue(g_sysParam.phctl_param[firstPH].fStdVoltage);
        }
    }
}

void VTSetDialog::populateComboBoxes()
{
    // 填充喷头ID下拉框
    ui->comboPHID->clear();
    m_nActivePHCount = 0;
    for (int i = 0; i < MAX_PH_CNT; ++i) {
        if (g_bPHValid[i]) {
            ui->comboPHID->addItem(QString("PH-%1").arg(i), i);
            m_nPhTBL[m_nActivePHCount++] = i;
        }
    }

    // 填充温度选择下拉框
    ui->comboTempSel->clear();
    if (m_nTempCount > 0) {
        for (int i = 0; i < m_nTempCount; ++i) {
            ui->comboTempSel->addItem(QString("Temp-%1").arg(i));
        }
        ui->comboTempSel->setEnabled(true);
    } else {
        ui->comboTempSel->setEnabled(false);
    }

    // 填充电压选择下拉框
    ui->comboVolSel->clear();
    ui->comboVolSel->addItem("标准电压");
    for (int i = 0; i < m_nVolCount; ++i) {
        ui->comboVolSel->addItem(QString("Vol-%1").arg(i));
    }
}

void VTSetDialog::setupVTListTree()
{
    ui->treeWidgetVT->clear();
    QStringList headers;
    headers << "喷头ID";

    for (int i = 0; i < m_nTempCount; ++i) {
        headers << QString("温度T%1").arg(i);
    }
    headers << "基准电压";
    for (int i = 0; i < m_nVolCount; ++i) {
        headers << QString("校准电压V%1").arg(i);
    }
    
    ui->treeWidgetVT->setColumnCount(headers.size());
    ui->treeWidgetVT->setHeaderLabels(headers);
}

void VTSetDialog::redrawVTList()
{
    ui->treeWidgetVT->clear();
    
    for (int i = 0; i < m_nActivePHCount; ++i) {
        int ph_idx = m_nPhTBL[i];
        
        for (int s = 0; s < m_nSplit; ++s) {
            QStringList rowData;
            if (s == 0) {
                rowData << QString("喷头ID-%1").arg(ph_idx + 1);
            } else {
                rowData << ""; // 在Xaar模式下，子行不显示ID
            }

            if (s == 0) {
                for (int t = 0; t < m_nTempCount; ++t) {
                    rowData << QString::number(g_sysParam.phctl_param[ph_idx].fDestTemp[t], 'f', 1);
                }
            } else {
                for (int t = 0; t < m_nTempCount; ++t) {
                    rowData << "";
                }
            }

            rowData << QString::number(g_sysParam.phctl_param[ph_idx].fStdVoltage, 'f', 1);
            
            for (int v = 0; v < m_nVolCount; ++v) {
                rowData << QString::number(g_sysParam.phctl_param[ph_idx].fVoltage[v + s * m_nVolCount], 'f', 1);
            }

            QTreeWidgetItem *item = new QTreeWidgetItem(rowData);
            ui->treeWidgetVT->addTopLevelItem(item);
        }
    }

    // 调整列宽
    for(int i = 0; i < ui->treeWidgetVT->columnCount(); ++i) {
        ui->treeWidgetVT->resizeColumnToContents(i);
    }
}


void VTSetDialog::on_buttonTempSet_clicked()
{
    double value = ui->spinBoxTempVal->value();
    int tempIndex = ui->comboTempSel->currentIndex();

    if (ui->checkTempAll->isChecked()) {
        // 应用到所有喷头
        for (int i = 0; i < m_nActivePHCount; ++i) {
            int ph_idx = m_nPhTBL[i];
            for (int t = 0; t < m_nTempCount; ++t) {
                 g_sysParam.phctl_param[ph_idx].fDestTemp[t] = value;
            }
        }
    } else {
        // 只应用到选中的喷头
        int ph_id = ui->comboPHID->currentData().toInt();
        if (tempIndex >= 0 && tempIndex < m_nTempCount) {
             g_sysParam.phctl_param[ph_id].fDestTemp[tempIndex] = value;
        }
    }

    redrawVTList();
}

void VTSetDialog::on_buttonVolSet_clicked()
{
    double value = ui->spinBoxVolVal->value();
    int volIndex = ui->comboVolSel->currentIndex();

    if (ui->checkVolAll->isChecked()) {
        // 应用到所有喷头和所有电压点
        for (int i = 0; i < m_nActivePHCount; ++i) {
            int ph_idx = m_nPhTBL[i];
            // 设置基准电压
            g_sysParam.phctl_param[ph_idx].fStdVoltage = value;
            // 设置所有其他电压点
            for (int v = 0; v < m_nVolCount * m_nSplit; ++v) {
                g_sysParam.phctl_param[ph_idx].fVoltage[v] = value;
            }
        }
    } else {
        // 只应用到选中的喷头
        int ph_id = ui->comboPHID->currentData().toInt();
        if (volIndex == 0) { // 基准电压
            g_sysParam.phctl_param[ph_id].fStdVoltage = value;
        } else { // 其他电压点
            int actualVolIndex = volIndex - 1;
            if (actualVolIndex >= 0 && actualVolIndex < m_nVolCount * m_nSplit) {
                 g_sysParam.phctl_param[ph_id].fVoltage[actualVolIndex] = value;
            }
        }
    }
    
    redrawVTList();
}

void VTSetDialog::on_buttonClose_clicked()
{
    this->accept();
}

void VTSetDialog::on_comboPHID_currentIndexChanged(int index)
{
    if (index < 0 || index >= m_nActivePHCount) return;

    int ph_id = ui->comboPHID->currentData().toInt();

    // 更新编辑框中的默认值以反映当前选定的喷头
    if (m_nTempCount > 0) {
        int tempIndex = ui->comboTempSel->currentIndex();
        if (tempIndex >=0)
            ui->spinBoxTempVal->setValue(g_sysParam.phctl_param[ph_id].fDestTemp[tempIndex]);
    }
    if (m_nVolCount > 0) {
        int volIndex = ui->comboVolSel->currentIndex();
        if (volIndex == 0) {
            ui->spinBoxVolVal->setValue(g_sysParam.phctl_param[ph_id].fStdVoltage);
        } else {
            ui->spinBoxVolVal->setValue(g_sysParam.phctl_param[ph_id].fVoltage[volIndex - 1]);
        }
    }
}
