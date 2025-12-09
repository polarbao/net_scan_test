#ifndef DATAWRITERTHREAD_H
#define DATAWRITERTHREAD_H

#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QByteArray>

// 定义MAX_COLORS
#ifndef MAX_COLORS
#define MAX_COLORS 16
#endif

// 外部全局变量声明
extern PRTJOB_ITEM g_testJob;
extern PRTIMG_LAYER g_PrtImgLayer;
extern QByteArray g_prtData[MAX_COLORS];
extern int g_prtDataSize;
extern int PrtBuffNum;
extern HANDLE g_PrtMutex;

/**
 * @brief 数据写入线程类
 * 
 * 对应MFC版本的PrintThreadSeparate静态函数
 * 负责将图像数据写入设备缓冲区（生产者角色）
 * 
 * 功能：
 * 1. 启动打印作业 (IDP_SartPrintJob)
 * 2. 循环写入图像图层数据到设备缓冲区
 * 3. 使用PrtBuffNum管理缓冲区（最多5个图层）
 * 4. 使用g_PrtMutex保护临界区
 */
class DataWriterThread : public QThread
{
    Q_OBJECT

public:
    explicit DataWriterThread(int layerCounts, QObject *parent = nullptr);
    ~DataWriterThread();
    
    /**
     * @brief 停止线程
     */
    void stop();
    
    /**
     * @brief 检查线程是否已停止
     */
    bool isStopped() const { return m_stop; }

signals:
    /**
     * @brief 数据写入完成信号
     */
    void writeFinished();
    
    /**
     * @brief 发生错误信号
     * @param error 错误描述
     */
    void errorOccurred(const QString &error);
    
    /**
     * @brief 写入进度信号
     * @param current 当前写入的图层索引
     * @param total 总图层数
     */
    void writeProgress(int current, int total);

protected:
    /**
     * @brief 线程执行函数
     * 
     * 实现MFC PrintThreadSeparate的逻辑：
     * 1. 调用IDP_SartPrintJob启动打印作业
     * 2. 初始化PrtBuffNum = 0
     * 3. 循环写入每个图层：
     *    - 如果PrtBuffNum < 5: 写入数据，PrtBuffNum++
     *    - 否则：等待直到PrtBuffNum < 5
     * 4. 线程结束
     */
    void run() override;

private:
    int m_layerCounts;          ///< 图层总数
    volatile bool m_stop;        ///< 停止标志
    QMutex m_stopMutex;         ///< 保护m_stop的互斥锁
};

#endif // DATAWRITERTHREAD_H

