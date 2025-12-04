#ifndef LACKJETDIALOG_H
#define LACKJETDIALOG_H

#include <QDialog>

namespace Ui {
class LackJetDialog;
}

/**
 * @brief 缺嘴补偿对话框
 * 
 * 对应MFC版本的CLackJetDlg。
 * 允许用户为特定的颜色/组添加或删除“缺嘴”的喷嘴号。
 */
class LackJetDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LackJetDialog(QWidget *parent = nullptr);
    ~LackJetDialog();

private slots:
    void on_buttonAdd_clicked();
    void on_buttonRemove_clicked();
    void on_buttonClearAll_clicked();

    // 当用户改变颜色或组选择时，更新列表
    void on_comboColor_currentIndexChanged(int index);
    void on_comboGroup_currentIndexChanged(int index);

private:
    void initDialog();
    void populateComboBoxes();
    void updateJetList();

    Ui::LackJetDialog *ui;
};

#endif // LACKJETDIALOG_H
