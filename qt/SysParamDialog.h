#ifndef SYSPARAMDIALOG_H
#define SYSPARAMDIALOG_H

#include <QDialog>
#include <QString>

namespace Ui {
class SysParamDialog;
}

/**
 * @brief 系统参数对话框
 * 
 * 对应MFC版本的CSysParamDlg
 * 负责系统参数配置
 */
class SysParamDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SysParamDialog(QWidget *parent = nullptr);
    ~SysParamDialog();
    
    // Getter方法
    float cbDistance() const { return m_cbDistance; }
    float cbWidth() const { return m_cbWidth; }
    float cbInterval() const { return m_cbInterval; }
    float cbInkDensity() const { return m_cbInkDensity; }
    int prtPeriod() const { return m_prtPeriod; }
    int cbValidMask() const { return m_cbValidMask; }
    int cbPosMode() const { return m_cbPosMode; }
    bool equiDis() const { return m_equiDis; }
    int microJetUnit() const { return m_microJetUnit; }
    int microJetCount() const { return m_microJetCount; }
    int microStpJet() const { return m_microStpJet; }
    bool encReverse() const { return m_encReverse; }
    bool idleFlash() const { return m_idleFlash; }
    bool bigInkDrops() const { return m_bigInkDrops; }
    QString logPath() const { return m_logPath; }
    int colorSel() const { return m_colorSel; }
    int overlapProc() const { return m_overlapProc; }
    float flashPrtFrequency() const { return m_flashPrtFrequency; }
    float brustValidSec() const { return m_brustValidSec; }
    float brustCycleSec() const { return m_brustCycleSec; }
    bool useAdib() const { return m_useAdib; }

private slots:
    void onAccepted();
    void onButtonSetClicked();

private:
    void initUI();
    void loadFromGlobal();
    void saveToGlobal();
    void initFlashList();
    void reDrawFlashList();
    
    Ui::SysParamDialog *ui;
    
    // 参数变量
    float m_cbDistance;
    float m_cbWidth;
    float m_cbInterval;
    float m_cbInkDensity;
    int m_prtPeriod;
    int m_cbValidMask;
    int m_cbPosMode;
    bool m_equiDis;
    int m_microJetUnit;
    int m_microJetCount;
    int m_microStpJet;
    bool m_encReverse;
    bool m_idleFlash;
    bool m_bigInkDrops;
    QString m_logPath;
    int m_colorSel;
    int m_overlapProc;
    float m_flashPrtFrequency;
    float m_brustValidSec;
    float m_brustCycleSec;
    bool m_useAdib;
};

#endif // SYSPARAMDIALOG_H

