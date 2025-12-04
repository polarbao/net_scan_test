#ifndef ADJPARAMDIALOG_H
#define ADJPARAMDIALOG_H

#include <QDialog>
#include "../Inc/ryprtapi.h" // For RYCalbrationParam

// Forward declarations
namespace Ui {
class AdjParamDialog;
}
class LackJetDialog;
class CalibrationPrintThread;

/**
 * @brief 校准与补偿对话框
 * 
 * 对应MFC版本的CAdjParamDlg。
 * 这是一个功能复杂的对话框，用于：
 * 1. 设置颜色/组/喷嘴的X轴偏移。
 * 2. 启动多种类型的校准打印（状态、垂直、步进等）。
 * 3. 启动缺嘴补偿对话框。
 */
class AdjParamDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AdjParamDialog(QMutex* hwMutex, QWidget *parent = nullptr);
    ~AdjParamDialog();

private slots:
    // ... (rest of the slots are the same)
    void on_buttonGroupOffset_clicked();
    void on_buttonJetOffset_clicked();
    void on_comboGroupDir_currentIndexChanged(int index);
    void on_comboJetDir_currentIndexChanged(int index);
    void on_buttonPrintStatus_clicked();
    void on_buttonPrintVertical_clicked();
    void on_buttonPrintStep_clicked();
    void on_buttonPrintXOffsetGroup_clicked();
    void on_buttonPrintXOffsetJet_clicked();
    void on_buttonPrintBidirectional_clicked();
    void on_buttonStopPrint_clicked();
    void on_buttonLackJet_clicked();
    void on_buttonApplyAndClose_clicked();
    void onPrintFinished();
    void onPrintError(const QString &error);

private:
    void initDialog();
    void populateComboBoxes();
    void setupOffsetTrees();
    void updateGroupOffsetList();
    void updateJetOffsetList();
    void startCalibrationPrint(int adjType);

    Ui::AdjParamDialog *ui;
    QMutex* m_hwMutex; // Shared hardware mutex
    CalibrationPrintThread *m_printThread;

    RYCalbrationParam m_calParam; // 本地校准参数副本
};

#endif // ADJPARAMDIALOG_H
