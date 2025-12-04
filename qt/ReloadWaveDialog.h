#ifndef RELOADWAVEDIALOG_H
#define RELOADWAVEDIALOG_H

#include <QDialog>
#include <QString>

namespace Ui {
class ReloadWaveDialog;
}

/**
 * @brief 波形加载对话框
 * 
 * 对应MFC版本的CReloadWaveDlg。
 * 提供UI让用户选择颜色、组别和波形文件路径。
 */
class ReloadWaveDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ReloadWaveDialog(QWidget *parent = nullptr);
    ~ReloadWaveDialog();

    // 公共接口，用于获取用户选择的值
    int selectedColorIndex() const;
    int selectedGroupIndex() const;
    QString selectedWavePath() const;

private slots:
    void on_buttonBrowse_clicked();
    void on_buttonBox_accepted();

private:
    void initDialog();
    
    Ui::ReloadWaveDialog *ui;
};

#endif // RELOADWAVEDIALOG_H
