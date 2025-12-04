#ifndef PRINTTHREAD_H
#define PRINTTHREAD_H

#include <QThread>
#include <QMutex>
#include <QWaitCondition>

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

