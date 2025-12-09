#ifndef PRINTEXECUTORTHREAD_H
#define PRINTEXECUTORTHREAD_H

#include <QThread>
#include <QMutex>
#include <QWaitCondition>

// 外部全局变量声明
extern PRTJOB_ITEM g_testJob;
extern PRTIMG_LAYER g_PrtImgLayer;
extern MOV_Config g_movConfig;
extern int PrtBuffNum;
extern HANDLE g_PrtMutex;

/**
 * @brief 打印执行线程类
 * 
 * 对应MFC版本的PrintThread静态函数
 * 负责从缓冲区读取并执行打印（消费者角色）
 * 
 * 功能：
 * 1. 等待缓冲区有数据（PrtBuffNum > 0）
 * 2. 启动图层打印 (IDP_StartLayerPrint)
 * 3. 循环处理每个PASS：
 *    - 等待PASS就绪
 *    - 运动控制
 *    - 执行打印 (IDP_DoPassPrint)
 * 4. 释放图层资源，PrtBuffNum--
 */
class PrintExecutorThread : public QThread
{
    Q_OBJECT

public:
    explicit PrintExecutorThread(int layerCounts, QObject *parent = nullptr);
    ~PrintExecutorThread();
    
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
     * @brief 打印完成信号
     */
    void printFinished();
    
    /**
     * @brief 打印进度信号
     * @param current 当前打印的图层索引
     * @param total 总图层数
     */
    void printProgress(int current, int total);
    
    /**
     * @brief 打印状态改变信号
     * @param state 打印状态
     */
    void printStateChanged(uint state);
    
    /**
     * @brief 发生错误信号
     * @param error 错误描述
     */
    void errorOccurred(const QString &error);

protected:
    /**
     * @brief 线程执行函数
     * 
     * 实现MFC PrintThread的逻辑：
     * 1. 循环处理每个图层：
     *    - 等待PrtBuffNum > 0（有数据）
     *    - 获取互斥锁
     *    - 调用IDP_StartLayerPrint启动图层打印
     *    - 循环处理每个PASS：
     *      * 等待PASS就绪
     *      * 运动控制（可选）
     *      * IDP_DoPassPrint执行打印
     *    - 释放互斥锁
     *    - IDP_FreeImageLayer释放图层
     *    - PrtBuffNum--
     * 2. 清理工作
     */
    void run() override;

private:
    int m_layerCounts;          ///< 图层总数
    volatile bool m_stop;        ///< 停止标志
    volatile bool m_jobStarted;  ///< 作业是否已启动（用于PASS等待检查）
    QMutex m_stopMutex;         ///< 保护m_stop的互斥锁
};

#endif // PRINTEXECUTORTHREAD_H

