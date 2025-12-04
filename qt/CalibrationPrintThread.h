#ifndef CALIBRATIONPRINTTHREAD_H
#define CALIBRATIONPRINTTHREAD_H

#include <QThread>
#include <QString>
#include "../Inc/ryprtapi.h"

/**
 * @brief 校准打印线程
 * 
 * 负责在后台执行校准打印的整个流程，避免UI阻塞。
 * 流程：启动校准 -> 循环获取Pass -> 执行Pass -> 等待Pass完成 -> 结束
 */
class CalibrationPrintThread : public QThread
{
    Q_OBJECT

public:
    explicit CalibrationPrintThread(RYCalbrationParam params, QMutex* hwMutex, QObject *parent = nullptr);
    void stop();

signals:
    void printFinished();
    void errorOccurred(const QString &error);

protected:
    void run() override;

private:
    RYCalbrationParam m_params;
    QMutex* m_hwMutex;
    volatile bool m_stopped;
};

#endif // CALIBRATIONPRINTTHREAD_H
