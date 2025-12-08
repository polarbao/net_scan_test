#ifndef PRINTTHREAD_H
#define PRINTTHREAD_H

#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QByteArray> // For QByteArray

// Define MAX_COLORS here for now, assuming it's a project-wide constant.
// If it's defined in a shared SDK header, that header should be included instead.
#define MAX_COLORS 16

// Extern declarations for global variables used by PrintThread
extern PRTJOB_ITEM g_testJob;
extern PRTIMG_LAYER g_PrtImgLayer;
extern QByteArray g_prtData[MAX_COLORS]; // Global image data buffer
extern int g_prtDataSize; // Global image data size
extern int PrtBuffNum; // External buffer counter

// MOV_Config struct definition and MM_TO_DOT macro,
// copied from NetScanDemo.h as it's not a shared header in Qt project.
#define MM_TO_DOT(X,DPI) (int)(((float)(X*DPI))/25.4+0.45f)

typedef struct movconfig {
    float fxSysdpi;
    float fySysdpi;
    float fxMovSpd;
    float fyMovSpd;
    float fxMovAcc;
    float fyMovAcc;
    float fxMovUnit;
    float fyMovUnit;
    float fxMovRate;
    float fyMovRate;
    float fxIoOption;
    float fyIoOption;
    float fMovBuf;
}MOV_Config;

extern MOV_Config g_movConfig; // Global motion configuration

/**
 * @brief 打印线程类
 * 
 * 对应MFC版本的PrintThread静态函数
 * 负责打印数据发送和设备控制
 */
class PrintThread : public QThread
{
    Q_OBJECT

public:
    explicit PrintThread(int jobImgLayerCounts, QMutex* hwMutex, QObject *parent = nullptr);
    ~PrintThread();
    
    void stop();

signals:
    void printStateChanged(uint state);
    void printProgress(int progress);
    void printFinished();
    void errorOccurred(const QString &error);

protected:
    void run() override;

private:
    QMutex* m_hwMutex;      // Mutex for hardware access, shared with other threads
    QMutex m_internalMutex; // Mutex for internal state of this class
    QWaitCondition m_condition;
    
    volatile bool m_stop;
    int m_jobImgLayerCounts;  // 作业图像图层数量
};

#endif // PRINTTHREAD_H

