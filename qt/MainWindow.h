#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QThread>
#include <QMutex>
#include "ui_MainWindow.h"

// 前向声明
class PrintInfoTreeWidget;
class ImagePreviewWidget;
class PrintThread;
class MonitorThread;
class ParameterManager;
class PrintJobDialog;
class SysParamDialog;

// 全局参数结构（与MFC版本保持一致）
extern RYUSR_PARAM g_sysParam;
extern PRTJOB_ITEM g_testJob;
extern PRTIMG_LAYER g_PrtImgLayer;
extern RYCalbrationParam g_Calbration;

/**
 * @brief 主窗口类
 * 
 * 对应MFC版本的CNetScanDemoDlg
 * 负责主界面显示和控制，打印作业管理，图像加载和预览，设备信息显示
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // 按钮点击槽函数
    void onButtonStartJobClicked();
    void onButtonLoadImageClicked();
    void onButtonReloadWaveClicked();
    void onButtonVTSetClicked();
    void onButtonCaliClicked();
    void onButtonSysParamClicked();
    void onButtonFlashClicked();
    void onButtonResetEncoderClicked();
    void onButtonMoveCtlClicked();
    void onButtonAirInkClicked();
    void onButtonAdibCtlClicked();
    
    // 复选框槽函数
    void onCheckInkWatchToggled(bool checked);
    void onCheckCloseAutoCheckToggled(bool checked);
    
    // 编辑框槽函数
    void onFilePathChanged(const QString &text);
    
    // 线程信号槽
    void onPrintStateChanged(uint state);
    void onPrintProgress(int progress);
    void onPrintFinished();
    void onPrintError(const QString &error);
    void onMonitorDataUpdated();

private:
    // 初始化函数
    void initSystem();
    void initDeviceInfo();
    void initSysParam();
    void initMoveSys();
    
    // 更新函数
    void updateButtonState();
    void updatePrtState(uint state);
    void updateVTListInfo();
    
    // 工具函数
    int getSrcData(const QString &filePath);
    QString getApplicationPath() const;
    
    // UI控件
    Ui::MainWindow *ui;
    PrintInfoTreeWidget *m_printInfoTree;
    PrintInfoTreeWidget *m_drvInfoTree;
    ImagePreviewWidget *m_imagePreview;
    
    // 线程
    PrintThread *m_printThread;
    MonitorThread *m_monitorThread;
    QMutex m_printMutex;
    
    // 参数管理
    ParameterManager *m_paramManager;
    
    // 状态变量
    bool m_jobStarted;
    bool m_printing;
    bool m_imageLoaded;
    bool m_stopMonitor;
    QString m_prtFilePath;
    bool m_enableInkWatch;
    bool m_closeAutoCheck;
    
    // 打印数据缓冲区
    QByteArray m_prtData[MAX_COLORS];
    int m_prtDataSize;
    int m_jobImgLayerCounts;
    
    // 设备信息
    int m_prtRevColumn;
    int m_prtEncoderVal;
};

#endif // MAINWINDOW_H

