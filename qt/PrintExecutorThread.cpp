#include "PrintExecutorThread.h"
#include "../Inc/RYPrtCtler.h"
#include "../Inc/MoveCtl.h"
#include "../Inc/ryprtapi.h"
#include <QDebug>
#include <QElapsedTimer>
#include <windows.h>

#define ZERO_POSITION 0
#define MM_TO_DOT(X,DPI) (int)(((float)(X*DPI))/25.4+0.45f)

PrintExecutorThread::PrintExecutorThread(int layerCounts, QObject *parent)
    : QThread(parent)
    , m_layerCounts(layerCounts)
    , m_stop(false)
    , m_jobStarted(true)
{
}

PrintExecutorThread::~PrintExecutorThread()
{
    stop();
    wait();
}

void PrintExecutorThread::stop()
{
    QMutexLocker locker(&m_stopMutex);
    m_stop = true;
    m_jobStarted = false;
}

void PrintExecutorThread::run()
{
    qDebug() << "[PrintExecutorThread] Thread started";
    
    PrtRunInfo prtInfo;
    int nPassCount;
    LPPassDataItem pPrtPassDes;
    
    // 计算运动控制参数（与MFC一致）
    int nMovBuf = MM_TO_DOT(g_movConfig.fMovBuf, g_movConfig.fxSysdpi);
    int nDestPos = 0;
    int ndistance = 0;
    int nXMoveSpd = MM_TO_DOT(g_movConfig.fxMovSpd, g_movConfig.fxSysdpi);
    
    // ========== 图层循环 ==========
    for (int n = 0; n < m_layerCounts; n++) {
        // 检查停止标志
        {
            QMutexLocker locker(&m_stopMutex);
            if (m_stop) {
                qDebug() << "[PrintExecutorThread] Stopped by user";
                emit printFinished();
                return;
            }
        }
        
        qDebug() << "[PrintExecutorThread] Processing layer" << n;
        
        // 3.1 等待缓冲区就绪（对应MFC: while (PrtBuffNum<=0) Sleep(10);）
        qDebug() << "[PrintExecutorThread] Waiting for buffer ready...";
        while (PrtBuffNum <= 0) {
            // 检查停止标志
            {
                QMutexLocker locker(&m_stopMutex);
                if (m_stop) {
                    qDebug() << "[PrintExecutorThread] Stopped while waiting";
                    emit printFinished();
                    return;
                }
            }
            QThread::msleep(10);  // 与MFC一致
        }
        
        qDebug() << "[PrintExecutorThread] Buffer ready, PrtBuffNum=" << PrtBuffNum;
        
        // 3.2 获取互斥锁
        qDebug() << "[PrintExecutorThread] Acquiring mutex for layer" << n;
        DWORD waitResult = WaitForSingleObject(g_PrtMutex, INFINITE);
        if (waitResult != WAIT_OBJECT_0) {
            emit errorOccurred(QString("获取互斥锁失败，层：%1").arg(n));
            return;
        }
        
        // 3.3 启动图层打印
        qDebug() << "[PrintExecutorThread] Starting layer print" << n;
        nPassCount = IDP_StartLayerPrint(n);
        
        // 3.4 等待图层就绪（重试机制，对应MFC逻辑）
        while (nPassCount == -1) {
            // 检查停止标志
            {
                QMutexLocker locker(&m_stopMutex);
                if (m_stop) {
                    ReleaseMutex(g_PrtMutex);
                    qDebug() << "[PrintExecutorThread] Stopped while waiting for layer";
                    emit printFinished();
                    return;
                }
            }
            nPassCount = IDP_StartLayerPrint(n);
        }
        
        qDebug() << "[PrintExecutorThread] Layer" << n << "PASS count:" << nPassCount;
        
        // 3.5 PASS循环
        if (nPassCount > 0) {
            for (int i = 0; i < nPassCount; i++) {
                // 检查停止标志
                {
                    QMutexLocker locker(&m_stopMutex);
                    if (m_stop) {
                        ReleaseMutex(g_PrtMutex);
                        qDebug() << "[PrintExecutorThread] Stopped in PASS loop";
                        emit printFinished();
                        return;
                    }
                }
                
                qDebug() << "[PrintExecutorThread] Processing PASS" << i << "of layer" << n;
                
                // 3.5.1 等待PASS就绪（对应MFC的do-while循环）
                QElapsedTimer passTimer;
                passTimer.start();
                const int PASS_TIMEOUT_MS = 60 * 1000;  // 60秒超时
                
                pPrtPassDes = nullptr;
                while (true) {
                    // 检查停止标志
                    {
                        QMutexLocker locker(&m_stopMutex);
                        if (m_stop || !m_jobStarted) {
                            ReleaseMutex(g_PrtMutex);
                            qDebug() << "[PrintExecutorThread] Job stopped";
                            emit printFinished();
                            return;
                        }
                    }
                    
                    pPrtPassDes = IDP_GetPassItem(n, i);
                    if (pPrtPassDes) {
                        if ((pPrtPassDes->nProcState > 2) && (pPrtPassDes->nProcState < 6)) {
                            // 状态3,4,5都可以执行打印
                            qDebug() << "[PrintExecutorThread] PASS" << i << "ready, state:" << pPrtPassDes->nProcState;
                            break;
                        }
                    }
                    
                    // 超时检查
                    if (passTimer.elapsed() >= PASS_TIMEOUT_MS) {
                        ReleaseMutex(g_PrtMutex);
                        emit errorOccurred(QString("PASS等待超时，层：%1, PASS：%2").arg(n).arg(i));
                        return;
                    }
                    
                    // 注意：MFC中这里没有sleep，紧密循环
                    // 为了降低CPU占用，这里可以选择性地添加极短的sleep
                    // QThread::msleep(1);
                }
                
                // 跳过空PASS
                if (pPrtPassDes->bIsNullPass) {
                    qDebug() << "[PrintExecutorThread] Skipping null PASS" << i;
                    continue;
                }
                
                // 3.5.2 运动控制（对应MFC逻辑）
#ifdef RY_MOVE_CTL
                int nYMoveSpd = MM_TO_DOT(g_movConfig.fyMovSpd, g_movConfig.fySysdpi);
                
                if (i == 0) {
                    // 第一个PASS: Y轴定位
                    // MFC注释：pPrtPassDes->fYMinJetImgPos 为Y开始打印位置到Y定位的距离
                    qDebug() << "[PrintExecutorThread] First PASS, Y positioning";
                    // DEM_MoveY(TRUE, nYMoveSpd, pPrtPassDes->fYMinJetImgPos, true, false, g_movConfig.fyMovRate);
                } else {
                    // 后续PASS: Y轴步进
                    int nYStep = MM_TO_DOT(pPrtPassDes->nStpVector / 1000.0, g_movConfig.fySysdpi) * 48;
                    qDebug() << "[PrintExecutorThread] Y step:" << nYStep;
                    DEM_MoveY(TRUE, nYMoveSpd, nYStep, true, false, g_movConfig.fyMovRate);
                }
                
                // X轴定位
                if (pPrtPassDes->bEncPrtDir) {
                    // 正向打印
                    nDestPos = (int)pPrtPassDes->nStartEncPos + ZERO_POSITION - nMovBuf;
                    ndistance = nDestPos - DEV_GetPrinterEncValue();
                } else {
                    // 反向打印
                    nDestPos = (int)pPrtPassDes->nStartEncPos + ZERO_POSITION + nMovBuf;
                    ndistance = nDestPos - DEV_GetPrinterEncValue();
                }
                qDebug() << "[PrintExecutorThread] X positioning, distance:" << ndistance;
                DEM_MoveX(FALSE, nXMoveSpd, ndistance, TRUE, g_movConfig.fxMovRate);
#endif
                
                // 3.5.3 执行PASS打印
                qDebug() << "[PrintExecutorThread] Executing PASS print" << i;
                int printResult = IDP_DoPassPrint(pPrtPassDes);
                if (printResult > 0) {
                    // 打印成功
                    
                    // X轴回退运动
#ifdef RY_MOVE_CTL
                    int nMoveLen = pPrtPassDes->nXPrtColumns * pPrtPassDes->fPrtPrecession + nMovBuf * 2;
                    qDebug() << "[PrintExecutorThread] X axis return, length:" << nMoveLen;
                    DEM_MoveX(!pPrtPassDes->bEncPrtDir, nXMoveSpd, nMoveLen, TRUE, g_movConfig.fxMovRate);
#endif
                    
                    qDebug() << "[PrintExecutorThread] PASS" << i << "print completed";
                } else {
                    // 打印失败（MFC中只是注释掉的错误处理）
                    qDebug() << "[PrintExecutorThread] PASS print failed, result:" << printResult;
                }
                
                // 注意：MFC中这里没有打印状态监控循环
                // 如果需要，可以添加：
                // do {
                //     IDP_GetPrintState(&prtInfo);
                //     emit printStateChanged(prtInfo.nPrtState);
                //     QThread::msleep(10);
                // } while (prtInfo.nPrtState == 1 && !m_stop);
            }
            // End of PASS loop
        } else {
            // nPassCount <= 0，图层处理错误
            ReleaseMutex(g_PrtMutex);
            emit errorOccurred(QString("图层 %1 处理错误，错误码：%2").arg(n).arg(nPassCount));
            break;
        }
        
        // 3.6 释放互斥锁（对应MFC的ReleaseMutex）
        ReleaseMutex(g_PrtMutex);
        qDebug() << "[PrintExecutorThread] Released mutex for layer" << n;
        
        // 3.7 释放图层资源
        qDebug() << "[PrintExecutorThread] Freeing layer" << n;
        IDP_FreeImageLayer(n);
        
        // 3.8 减少缓冲区计数（消费完成）
        PrtBuffNum--;
        qDebug() << "[PrintExecutorThread] PrtBuffNum--" << PrtBuffNum;
        
        // 发送进度信号
        emit printProgress(n + 1, m_layerCounts);
    }
    // End of layer loop
    
    // ========== 清理工作 ==========
    qDebug() << "[PrintExecutorThread] Print completed, cleaning up";
    IDP_StopPrintJob();
    IDP_FreeImageLayer(-1);
    
    // 注意：g_PrtMutex的关闭应该由主线程处理
    // CloseHandle(g_PrtMutex);
    
    qDebug() << "[PrintExecutorThread] Thread finished";
    emit printFinished();
}

