#ifndef MONITORTHREAD_H
#define MONITORTHREAD_H

#include <QThread>
#include <QMutex>
#include <QWaitCondition>

/**
 * @brief 监控线程类
 * 
 * 对应MFC版本的MonitorThread静态函数
 * 负责持续监控设备状态（温度、电压、连接状态等）
 */
class MonitorThread : public QThread
{
    Q_OBJECT

public:
    explicit MonitorThread(QMutex* hwMutex, QObject *parent = nullptr);
    ~MonitorThread();
    
    void stop();
    void setCloseAutoCheck(bool close);  // 设置是否关闭自动检查

signals:
    void dataUpdated();
    void deviceConnectedChanged(bool connected);
    void errorOccurred(const QString &error);

protected:
    void run() override;

private:
    QMutex* m_hwMutex;      // Mutex for hardware access, shared with other threads
    QMutex m_internalMutex; // Mutex for internal state of this class
    QWaitCondition m_condition;
    
    volatile bool m_stop;
    bool m_closeAutoCheck;  // 是否关闭自动检查
};

#endif // MONITORTHREAD_H

