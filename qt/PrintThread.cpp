#include "PrintThread.h"
#include "../Inc/RYPrtCtler.h"
#include "../Inc/MoveCtl.h" // For DEM_* functions
#include <QMutexLocker>
#include <QDebug>
#include <QElapsedTimer> // For potential timeouts

#define ZERO_POSITION 0 // Assumed value from MFC project

// 外部变量声明
extern PRTJOB_ITEM g_testJob;
extern PRTIMG_LAYER g_PrtImgLayer;
extern QByteArray g_prtData[MAX_COLORS]; // Global image data buffer
extern int g_prtDataSize; // Global image data size
extern MOV_Config g_movConfig; // Global motion configuration
extern int PrtBuffNum; // External buffer counter (managed by producer/consumer)

#include "../Inc/ryprtapi.h" // For DEV_GetPrinterEncValue and other SDK functions


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
    PrtRunInfo prtInfo;
    int nPassCount;
    LPPassDataItem pPrtPassDes;
    QString szErr;
    
    // 1. Job Initialization: Call IDP_SartPrintJob
    // 注意：与MFC不同，这里不在构造函数中调用，而是在run()中调用
    // MFC在PrintThreadSeparate中调用IDP_SartPrintJob
    int nResult = IDP_SartPrintJob(&g_testJob);
    if (nResult < 0) {
        emit errorOccurred(QString("启动打印作业失败，错误码：%1").arg(nResult));
        return;
    }

    // Initialize PrtBuffNum 
    PrtBuffNum = 0; 
    
    // 2. 图像图层写入和打印执行循环（合并MFC的两个线程逻辑）
    //    按照MFC的方式：边写入边打印，使用缓冲区管理
    for (int nLayerIndex = 0; nLayerIndex < m_jobImgLayerCounts; nLayerIndex++) {
        if (m_stop) {
            IDP_StopPrintJob();
            emit printFinished();
            return;
        }

        // 2.1 写入图像数据（对应MFC PrintThreadSeparate逻辑）
        LPBYTE pBmpFile[MAX_COLORS];
        for (int c = 0; c < g_PrtImgLayer.nColorCnts; c++) {
            // Ensure g_prtData[c] contains valid data
            if (g_prtData[c].isEmpty()) {
                 emit errorOccurred(QString("图像通道 %1 的数据为空！").arg(c + 1));
                 IDP_StopPrintJob();
                 return;
            }
            pBmpFile[c] = (LPBYTE)g_prtData[c].data();
        }
        
        g_PrtImgLayer.nLayerIndex = nLayerIndex; // Set current layer index
        g_PrtImgLayer.nValidClrMask = 0xFFFF;

        // 使用PrtBuffNum作为设备缓冲区信号量
        // MFC限制为5，这里保持一致
        // 重要：如果缓冲区满，等待直到有空间
        bool layerWritten = false;
        while (!layerWritten && !m_stop) {
            if (PrtBuffNum < 5) {
                // 有空间，写入数据
                PrtBuffNum++;
                
                // 获取互斥锁进行写入（对应MFC的WaitForSingleObject）
                m_hwMutex->lock();
                int writeResult = IDP_WriteImgLayerData(&g_PrtImgLayer, pBmpFile, g_prtDataSize);
                m_hwMutex->unlock();
                
                if (writeResult < 0) {
                    PrtBuffNum--; // 写入失败，减少计数
                    emit errorOccurred(QString("写入图像层 %1 数据失败，错误码：%2").arg(nLayerIndex).arg(writeResult));
                    IDP_StopPrintJob();
                    return;
                }
                layerWritten = true;
            } else {
                // 缓冲区满，等待
                msleep(1);
            }
        }
        
        if (m_stop) {
            IDP_StopPrintJob();
            emit printFinished();
            return;
        }
    } // End of image layer writing loop

    // 3. Pass执行循环（对应MFC PrintThread逻辑）
    //    所有图层数据写入完成后，开始执行打印
    for (int nLayerIndex = 0; nLayerIndex < m_jobImgLayerCounts; nLayerIndex++) {
        if (m_stop) break;
        
        // 等待缓冲区就绪（对应MFC: while (PrtBuffNum<=0) Sleep(10);）
        while (PrtBuffNum <= 0 && !m_stop) {
            msleep(10);
        }
        if (m_stop) break;

        // 获取互斥锁启动图层打印（对应MFC的WaitForSingleObject）
        m_hwMutex->lock();
        nPassCount = IDP_StartLayerPrint(nLayerIndex);
        
        // Wait until layer is ready to print
        // 注意：在MFC中，这个循环没有释放互斥锁，这里保持一致
        while (nPassCount == -1 && !m_stop) {
            nPassCount = IDP_StartLayerPrint(nLayerIndex);
        }
        
        if (m_stop) {
            m_hwMutex->unlock();
            break;
        }
        
        if (nPassCount > 0) {
            // PASS循环（对应MFC中的for(int i=0;i<nPassCount;i++)）
            for (int i = 0; i < nPassCount; i++) {
                if (m_stop) {
                    m_hwMutex->unlock();
                    break;
                }
                
                // 获取PASS数据项（对应MFC的IDP_GetPassItem）
                pPrtPassDes = nullptr;
                QElapsedTimer passItemTimer;
                passItemTimer.start();
                const int PASS_ITEM_TIMEOUT_MS = 60 * 1000; // 60秒超时

                // 等待PASS就绪（对应MFC的do-while循环）
                // 注意：MFC在这个循环中没有释放互斥锁
                while(!m_stop) {
                    pPrtPassDes = IDP_GetPassItem(nLayerIndex, i);
                    if (pPrtPassDes) {
                        if ((pPrtPassDes->nProcState > 2) && (pPrtPassDes->nProcState < 6)) {
                            break; // PASS就绪（状态3,4,5都可以执行打印）
                        }
                    }
                    if (passItemTimer.elapsed() >= PASS_ITEM_TIMEOUT_MS) {
                        m_hwMutex->unlock();
                        emit errorOccurred(QString("获取PASS数据项超时，图层：%1, PASS：%2").arg(nLayerIndex).arg(i));
                        IDP_StopPrintJob();
                        return;
                    }
                    // MFC中没有sleep，这里保持兼容性，但不sleep可能占用CPU
                    // msleep(1); // 可选：减少CPU占用
                }
                
                if (m_stop || !pPrtPassDes) {
                    m_hwMutex->unlock();
                    break;
                }
                if (pPrtPassDes->bIsNullPass) continue; // 跳过空PASS

                // 4. Motion Control (DEM_MoveY, DEM_MoveX)
#ifdef RY_MOVE_CTL
                int nSpd = MM_TO_DOT(g_movConfig.fyMovSpd, g_movConfig.fySysdpi); // Y speed
                if (i == 0) { // First PASS, Y-axis positioning
                    // Y axis initial positioning based on fYMinJetImgPos
                    // Example: DEM_MoveY(TRUE, nSpd, MM_TO_DOT(pPrtPassDes->fYMinJetImgPos, g_movConfig.fySysdpi), true, false, g_movConfig.fyMovRate);
                    // MFC had a comment "//һPASS Yλöλ //pPrtPassDes->fYMinJetImgPos ΪYʼӡλYλľ //MOV_Y(pPrtPassDes->fYMinJetImgPos);"
                    // This implies the Y-axis position is set based on fYMinJetImgPos for the first pass.
                    // Assuming DEM_MoveY for positioning based on fYMinJetImgPos
                    DEM_MoveY(TRUE, nSpd, pPrtPassDes->fYMinJetImgPos, true, false, g_movConfig.fyMovRate);
                } else { // Subsequent PASSes, Y-axis step
                    int nYStep = MM_TO_DOT(pPrtPassDes->nStpVector / 1000.0, g_movConfig.fySysdpi) * 48; // MFC factor 48
                    DEM_MoveY(TRUE, nSpd, nYStep, true, false, g_movConfig.fyMovRate);
                }

                // X-axis positioning before print
                int nXMoveSpd = MM_TO_DOT(g_movConfig.fxMovSpd, g_movConfig.fxSysdpi); // X speed
                int nMovBuf = MM_TO_DOT(g_movConfig.fMovBuf, g_movConfig.fxSysdpi);
                int nDestPos;
                int ndistance;

                if (pPrtPassDes->bEncPrtDir) { // Forward print
                    nDestPos = (int)pPrtPassDes->nStartEncPos + ZERO_POSITION - nMovBuf;
                    ndistance = nDestPos - DEV_GetPrinterEncValue();
                } else { // Reverse print
                    nDestPos = (int)pPrtPassDes->nStartEncPos + ZERO_POSITION + nMovBuf;
                    ndistance = nDestPos - DEV_GetPrinterEncValue();
                }
                DEM_MoveX(FALSE, nXMoveSpd, ndistance, TRUE, g_movConfig.fxMovRate);
#endif
                // 执行PASS打印（对应MFC的IDP_DoPassPrint）
                int printPassResult = IDP_DoPassPrint(pPrtPassDes);
                if (printPassResult < 0) {
                    m_hwMutex->unlock();
                    emit errorOccurred(QString("执行打印PASS失败，图层：%1, PASS：%2, 错误码：%3").arg(nLayerIndex).arg(i).arg(printPassResult));
                    IDP_StopPrintJob();
                    return; 
                }
                
                // X轴打印后回退运动（对应MFC逻辑）
#ifdef RY_MOVE_CTL
                int nMoveLen = pPrtPassDes->nXPrtColumns * pPrtPassDes->fPrtPrecession + nMovBuf * 2;
                DEM_MoveX(!pPrtPassDes->bEncPrtDir, nXMoveSpd, nMoveLen, TRUE, g_movConfig.fxMovRate);
#endif

                // 监控打印状态（对应MFC中的do-while循环）
                // MFC在这里检查prtInfo.nPrtState == 1（正在打印）
                do {
                    if (m_stop) {
                        m_hwMutex->unlock();
                        IDP_StopPrintJob();
                        emit printFinished();
                        return;
                    }
                    IDP_GetPrintState(&prtInfo);
                    emit printStateChanged(prtInfo.nPrtState);
                    msleep(10);
                } while (prtInfo.nPrtState == 1 && !m_stop);
            } // End of pass loop
        } else {
             m_hwMutex->unlock();
             emit errorOccurred(QString("启动图层 %1 失败，错误码: %2").arg(nLayerIndex).arg(nPassCount));
             IDP_StopPrintJob();
             return;
        }

        // 释放互斥锁（对应MFC的ReleaseMutex）
        m_hwMutex->unlock();
        
        if(m_stop) break;

        // 释放图层资源
        IDP_FreeImageLayer(nLayerIndex);
        PrtBuffNum--; // 减少缓冲区计数，对应MFC逻辑
    } // End of layer loop
    
    // Cleanup
    IDP_StopPrintJob();
    IDP_FreeImageLayer(-1); // 清除所有剩余图层
    
    emit printFinished();
}

