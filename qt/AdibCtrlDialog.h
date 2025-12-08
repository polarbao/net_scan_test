#ifndef ADIBCTRLDIALOG_H
#define ADIBCTRLDIALOG_H

#include <QDialog>
#include <QTimer>
#include "../Inc/ryprtapi.h"

namespace Ui {
class AdibCtrlDialog;
}

class AdibCtrlDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AdibCtrlDialog(QWidget *parent = nullptr);
    ~AdibCtrlDialog();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void on_applyButton_clicked();
    void on_enableSetCheckBox_toggled(bool checked);
    void on_cleanModeButton_clicked();
    void on_cleanPump1Button_clicked();
    void on_cleanPump2Button_clicked();
    void on_cleanHeadButton_clicked();
    void on_inkSupplyButton_clicked();
    void on_resetErrorButton_clicked();
    void on_iicReadButton_clicked();
    void on_iicSaveButton_clicked();
    void updateState();

private:
    void initUI();
    void updateUI(bool isConnected);
    void sendCommand(UINT command, bool write = true, const ADIB_PARAM* params = nullptr);

    Ui::AdibCtrlDialog *ui;
    QTimer *m_timer;

    ADIB_PARAM m_adibSettings;
    bool m_isCleanMode;
    bool m_isCleanPump1;
    bool m_isCleanPump2;
    bool m_isCleanHead;
    bool m_isConnected;
};

#endif // ADIBCTRLDIALOG_H
