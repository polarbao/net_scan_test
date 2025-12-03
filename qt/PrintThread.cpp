#include "PrintThread.h"
#include "../Inc/RYPrtCtler.h"
#include <QMutexLocker>
#include <QDebug>
#include <Windows.h>

// 外部变量声明
extern HANDLE g_PrtMutex;
extern PRTJOB_ITEM g_testJob;
extern PRTIMG_LAYER g_PrtImgLayer;
extern int PrtBuffNum;

PrintThread::PrintThread(int jobImgLayerCounts, QObject *parent)
    : QThread(parent)
    , m_stop(false)
    , m_jobImgLayerCounts(jobImgLayerCounts)
    , m_mutexAcquired(false)
{
}

PrintThread::~PrintThread()
{
    // 确保互斥锁已释放
    ensureMutexReleased();
    
    // 停止线程
    stop();
    
    // 等待线程结束，设置超时避免无限等待
    if (!wait(5000)) {
        // 如果5秒内线程未结束，强制终止（不推荐，但作为最后手段）
        qWarning() << "PrintThread did not finish in time, terminating...";
        terminate();
        wait(1000);
    }
}

void PrintThread::stop()
{
    QMutexLocker locker(&m_mutex);
    m_stop = true;
    m_condition.wakeAll();
}

bool PrintThread::isStopped() const
{
    QMutexLocker locker(&m_mutex);
    return m_stop;
}

bool PrintThread::shouldStop() const
{
    QMutexLocker locker(&m_mutex);
    return m_stop;
}

void PrintThread::run()
{
    PrtRunInfo prtInfo;
    int nPassCount;
    LPPassDataItem pPrtPassDes;
    m_mutexAcquired = false;  // 重置互斥锁状态
    
    for (int n = 0; n < m_jobImgLayerCounts; n++) {
        if (shouldStop()) {
            break;
        }
        
        // 等待缓冲区就绪
        while (PrtBuffNum <= 0 && !shouldStop()) {
            msleep(10);
        }
        
        if (shouldStop()) {
            break;
        }
        
        // 获取互斥锁
        if (g_PrtMutex != INVALID_HANDLE_VALUE) {
            DWORD waitResult = WaitForSingleObject(g_PrtMutex, INFINITE);
            if (waitResult == WAIT_OBJECT_0) {
                m_mutexAcquired = true;
            } else {
                emit errorOccurred("获取打印互斥锁失败");
                return;
            }
        }
        
        // 启动图层打印
        nPassCount = IDP_StartLayerPrint(n);
        
        // 如果未就绪，重试
        while (nPassCount == -1 && !shouldStop()) {
            ensureMutexReleased();
            msleep(10);
            if (shouldStop()) {
                break;
            }
            if (g_PrtMutex != INVALID_HANDLE_VALUE) {
                DWORD waitResult = WaitForSingleObject(g_PrtMutex, INFINITE);
                if (waitResult == WAIT_OBJECT_0) {
                    m_mutexAcquired = true;
                } else {
                    emit errorOccurred("获取打印互斥锁失败");
                    return;
                }
            }
            nPassCount = IDP_StartLayerPrint(n);
        }
        
        if (shouldStop()) {
            ensureMutexReleased();
            break;
        }
        
        if (nPassCount > 0) {
            // 处理PASS循环
            for (int i = 0; i < nPassCount; i++) {
                if (shouldStop()) {
                    break;
                }
                
                // 获取PASS数据项
                do {
                    pPrtPassDes = IDP_GetPassItem(n, i);
                    if (shouldStop()) {
                        break;
                    }
                    if (pPrtPassDes) {
                        if ((pPrtPassDes->nProcState > 2) && 
                            (pPrtPassDes->nProcState < 6)) {
                            // 状态3,4,5都可以执行打印
                            break;
                        }
                    }
                    msleep(10);
                } while (1);
                
                if (shouldStop()) {
                    break;
                }
                
                if (pPrtPassDes) {
                    // 执行PASS打印
                    if (IDP_DoPassPrint(pPrtPassDes) < 0) {
                        emit errorOccurred("打印失败");
                        ensureMutexReleased();
                        return;
                    }
                    
                    // 监控打印状态
                    do {
                        IDP_GetPrintState(&prtInfo);
                        emit printStateChanged(prtInfo.nPrtState);
                        
                        if (shouldStop()) {
                            break;
                        }
                        msleep(1);
                    } while (prtInfo.nPrtState == 1 && !shouldStop());
                } else {
                    break;
                }
            }
        }
        
        // 释放互斥锁
        ensureMutexReleased();
    }
    
    // 确保互斥锁已释放
    ensureMutexReleased();
    
    // 清理资源
    IDP_FreeImageLayer(-1);
    IDP_StopPrintJob();
    
    emit printFinished();
}

void PrintThread::ensureMutexReleased()
{
    if (m_mutexAcquired && g_PrtMutex != INVALID_HANDLE_VALUE) {
        ReleaseMutex(g_PrtMutex);
        m_mutexAcquired = false;
    }
}

