#ifndef MOVECTLDIALOG_H
#define MOVECTLDIALOG_H

#include <QDialog>
#include "../Inc/MoveCtl.h" // Motion control API

// Forward-declare the global config struct
struct MOV_Config;
extern MOV_Config g_movConfig;


namespace Ui {
class MoveCtlDialog;
}

/**
 * @brief 运动控制对话框
 * 
 * 对应MFC版本的CMoveCtlDlg。
 * 提供一个面板用于手动控制打印平台的X, Y轴运动，
 * 并调整相关的运动参数。
 */
class MoveCtlDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MoveCtlDialog(QWidget *parent = nullptr);
    ~MoveCtlDialog();

private slots:
    // 运动按钮
    void on_buttonUp_clicked();    // Y-
    void on_buttonDown_clicked();  // Y+
    void on_buttonLeft_clicked();  // X-
    void on_buttonRight_clicked(); // X+
    
    // 控制按钮
    void on_buttonStop_clicked();
    void on_buttonReset_clicked();
    
    // 参数控制
    void on_buttonApply_clicked();
    void on_buttonClose_clicked();

private:
    void initDialog();
    void loadParamsFromConfig();
    void saveParamsToConfig();

    // Helper to convert mm to dots, replicating MM_TO_DOT
    inline int mmToDots(float mm, float dpi) {
        return static_cast<int>((mm / 25.4f) * dpi);
    }

    Ui::MoveCtlDialog *ui;
};

#endif // MOVECTLDIALOG_H
