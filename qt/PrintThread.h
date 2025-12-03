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
    explicit PrintThread(int jobImgLayerCounts = 1, QObject *parent = nullptr);
    ~PrintThread();
    
    void stop();
    bool isStopped() const;

signals:
    void printStateChanged(uint state);
    void printProgress(int progress);
    void printFinished();
    void errorOccurred(const QString &error);

protected:
    void run() override;

private:
    bool m_stop;
    mutable QMutex m_mutex;
    QWaitCondition m_condition;
    int m_jobImgLayerCounts;  // 作业图像图层数量
    bool m_mutexAcquired;     // 标记Windows互斥锁是否已获取
    
    bool shouldStop() const;
    void ensureMutexReleased();  // 确保互斥锁释放
};

#endif // PRINTTHREAD_H

