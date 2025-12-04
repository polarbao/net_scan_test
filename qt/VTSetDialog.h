#ifndef VTSETDIALOG_H
#define VTSETDIALOG_H

#include <QDialog>
#include "../Inc/RYPrtCtler.h"

namespace Ui {
class VTSetDialog;
}

/**
 * @brief 温度电压设置对话框
 * 
 * 对应MFC版本的CVTSetDlg，用于设置打印头的温度和电压。
 * 所有更改都将写入全局的 g_sysParam 结构体中。
 */
class VTSetDialog : public QDialog
{
    Q_OBJECT

public:
    explicit VTSetDialog(QWidget *parent = nullptr);
    ~VTSetDialog();

private slots:
    void on_buttonTempSet_clicked();
    void on_buttonVolSet_clicked();
    void on_buttonClose_clicked();
    void on_comboPHID_currentIndexChanged(int index);

private:
    void initDialog();
    void populateComboBoxes();
    void setupVTListTree();
    void redrawVTList();

    Ui::VTSetDialog *ui;

    // 从MFC版本中借鉴的成员变量
    int m_nTempCount;
    int m_nVolCount;
    int m_nSplit;
    int m_nActivePHCount;
    int m_nPhTBL[MAX_PH_CNT]; // 存储有效的喷头ID映射
};

#endif // VTSETDIALOG_H
