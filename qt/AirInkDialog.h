#ifndef AIRINKDIALOG_H
#define AIRINKDIALOG_H

#include <QDialog>
#include <QTimer>

namespace Ui {
class AirInkDialog;
}

/**
 * @brief 气墨系统控制对话框
 * 
 * 对应MFC版本的CAirInkDialog。
 * 负责设置和监控墨路系统的气压、温度，并提供手动控制功能。
 */
class AirInkDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AirInkDialog(QWidget *parent = nullptr);
    ~AirInkDialog();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    // UI 动作
    void on_buttonSetParams_clicked();
    void on_buttonPushInk_clicked();
    
    // 主使能开关
    void on_checkEnableSupply_toggled(bool checked);
    void on_checkEnablePress_toggled(bool checked);
    void on_checkEnableAutoPress_toggled(bool checked);

    // 6个独立的输出控制按钮
    void on_buttonOut1_clicked();
    void on_buttonOut2_clicked();
    void on_buttonOut3_clicked();
    void on_buttonOut4_clicked();
    void on_buttonOut5_clicked();
    void on_buttonOut6_clicked();
    
    // 定时器槽函数
    void onStatusTimerTimeout();
    void onPushInkTimerTimeout();
    
    void on_buttonClose_clicked();

private:
    void initDialog();
    void loadParams();
    void updateStatusDisplays();
    void updateOutputButton(int index, bool active);

    Ui::AirInkDialog *ui;

    QTimer* m_statusTimer;
    QTimer* m_pushInkTimer;
    
    // 状态变量
    bool m_isPushingInk;
    bool m_outputStates[6];
};

#endif // AIRINKDIALOG_H
