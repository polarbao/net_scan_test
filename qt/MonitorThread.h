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
    explicit MonitorThread(QObject *parent = nullptr);
    ~MonitorThread();
    
    void stop();
    bool isStopped() const;
    void setCloseAutoCheck(bool close);  // 设置是否关闭自动检查

signals:
    void dataUpdated();
    void deviceConnectedChanged(bool connected);
    void errorOccurred(const QString &error);

protected:
    void run() override;

private:
    bool m_stop;
    mutable QMutex m_mutex;
    QWaitCondition m_condition;
    bool m_closeAutoCheck;  // 是否关闭自动检查
    
    bool shouldStop() const;
    bool getCloseAutoCheck() const;
    void checkDeviceStatus();
    void updateDeviceInfo();
};

#endif // MONITORTHREAD_H

