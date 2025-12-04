#include "PrintThread.h"
#include "../Inc/RYPrtCtler.h"
#include <QMutexLocker>
#include <QDebug>

// 外部变量声明
extern PRTJOB_ITEM g_testJob;
extern PRTIMG_LAYER g_PrtImgLayer;
extern int PrtBuffNum; // This suggests another thread is preparing data.

PrintThread::PrintThread(int jobImgLayerCounts, QMutex* hwMutex, QObject *parent)
    : QThread(parent)
    , m_hwMutex(hwMutex)
    , m_stop(false)
    , m_jobImgLayerCounts(jobImgLayerCounts)
{
}

PrintThread::~PrintThread()
{
    // Stop and wait is handled by MainWindow
}

void PrintThread::stop()
{
    QMutexLocker locker(&m_internalMutex);
    m_stop = true;
    m_condition.wakeAll();
}

void PrintThread::run()
{
    QMutexLocker hwLocker(m_hwMutex); // Lock for the entire duration of the print job

    PrtRunInfo prtInfo;
    int nPassCount;
    LPPassDataItem pPrtPassDes;

    for (int n = 0; n < m_jobImgLayerCounts; n++) {
        if (m_stop) break;
        
        // This loop suggests a producer-consumer pattern that isn't fully implemented in this thread.
        // Assuming another thread calls IDP_WriteImgLayerData.
        while (PrtBuffNum <= 0 && !m_stop) {
            msleep(10);
        }
        
        if (m_stop) break;
        
        // 启动图层打印
        nPassCount = IDP_StartLayerPrint(n);
        
        while (nPassCount == -1 && !m_stop) {
            msleep(10); // Wait for layer to be ready
            nPassCount = IDP_StartLayerPrint(n);
        }
        
        if (m_stop) break;
        
        if (nPassCount > 0) {
            for (int i = 0; i < nPassCount; i++) {
                if (m_stop) break;
                
                // 获取PASS数据项
                pPrtPassDes = nullptr;
                while(!m_stop) {
                    pPrtPassDes = IDP_GetPassItem(n, i);
                    if (pPrtPassDes && (pPrtPassDes->nProcState > 2) && (pPrtPassDes->nProcState < 6)) {
                        break; // Pass ready
                    }
                    msleep(10);
                }
                
                if (m_stop || !pPrtPassDes) break;

                // 执行PASS打印
                if (IDP_DoPassPrint(pPrtPassDes) < 0) {
                    emit errorOccurred("打印失败 (IDP_DoPassPrint failed)");
                    // The lock will be released when the function returns
                    return; 
                }
                
                // 监控打印状态
                do {
                    if (m_stop) break;
                    IDP_GetPrintState(&prtInfo);
                    emit printStateChanged(prtInfo.nPrtState);
                    msleep(10);
                } while (prtInfo.nPrtState == 1 && !m_stop);
            }
        } else {
             emit errorOccurred(QString("启动图层 %1 失败，错误码: %2").arg(n).arg(nPassCount));
        }

        if(m_stop) break;

        IDP_FreeImageLayer(n);
        PrtBuffNum--; // Decrement buffer count, assuming producer/consumer model
    }
    
    // Cleanup will happen regardless of how the loop exits
    if (!m_stop) {
        IDP_StopPrintJob();
    }
    
    emit printFinished();
}

