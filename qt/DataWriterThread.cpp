#include "DataWriterThread.h"
#include "../Inc/RYPrtCtler.h"
#include <QDebug>
#include <windows.h>

DataWriterThread::DataWriterThread(int layerCounts, QObject *parent)
    : QThread(parent)
    , m_layerCounts(layerCounts)
    , m_stop(false)
{
}

DataWriterThread::~DataWriterThread()
{
    stop();
    wait();
}

void DataWriterThread::stop()
{
    QMutexLocker locker(&m_stopMutex);
    m_stop = true;
}

void DataWriterThread::run()
{
    qDebug() << "[DataWriterThread] Thread started";
    
    // ========== 阶段1: 启动打印作业 ==========
    qDebug() << "[DataWriterThread] Starting print job...";
    int nResult = IDP_SartPrintJob(&g_testJob);
    qDebug() << "[DataWriterThread] IDP_SartPrintJob result:" << nResult;
    
    if (nResult < 0) {
        emit errorOccurred(QString("启动打印作业失败，错误码：%1").arg(nResult));
        return;
    }
    
    // ========== 阶段2: 初始化缓冲区计数 ==========
    PrtBuffNum = 0;
    qDebug() << "[DataWriterThread] Initialized PrtBuffNum = 0";
    
    // ========== 阶段3: 图层数据写入循环 ==========
    for (int n = 0; n < m_layerCounts; n++) {
        // 检查停止标志
        {
            QMutexLocker locker(&m_stopMutex);
            if (m_stop) {
                qDebug() << "[DataWriterThread] Stopped by user";
                emit writeFinished();
                return;
            }
        }
        
        // 3.1 准备图像数据指针
        LPBYTE pBmpFile[MAX_COLORS];
        for (int c = 0; c < g_PrtImgLayer.nColorCnts; c++) {
            if (g_prtData[c].isEmpty()) {
                emit errorOccurred(QString("图像通道 %1 的数据为空！").arg(c + 1));
                return;
            }
            pBmpFile[c] = (LPBYTE)g_prtData[c].data();
        }
        
        // 3.2 设置图层信息
        g_PrtImgLayer.nLayerIndex = n;
        g_PrtImgLayer.nValidClrMask = 0xFFFF;
        
        qDebug() << "[DataWriterThread] Preparing to write layer" << n;
        
        // 3.3 缓冲区管理（关键！）
        bool layerWritten = false;
        while (!layerWritten) {
            // 检查停止标志
            {
                QMutexLocker locker(&m_stopMutex);
                if (m_stop) {
                    qDebug() << "[DataWriterThread] Stopped by user";
                    emit writeFinished();
                    return;
                }
            }
            
            if (PrtBuffNum < 5) {
                // 缓冲区未满，可以写入
                PrtBuffNum++;
                qDebug() << "[DataWriterThread] PrtBuffNum++" << PrtBuffNum;
                
                // 3.4 临界区：写入数据
                qDebug() << "[DataWriterThread] Acquiring mutex for layer" << n;
                DWORD waitResult = WaitForSingleObject(g_PrtMutex, INFINITE);
                if (waitResult != WAIT_OBJECT_0) {
                    PrtBuffNum--;
                    emit errorOccurred(QString("获取互斥锁失败，层：%1").arg(n));
                    return;
                }
                
                qDebug() << "[DataWriterThread] Writing layer" << n;
                int writeResult = IDP_WriteImgLayerData(&g_PrtImgLayer, pBmpFile, g_prtDataSize);
                
                ReleaseMutex(g_PrtMutex);
                qDebug() << "[DataWriterThread] Released mutex for layer" << n;
                
                if (writeResult != 1) {
                    PrtBuffNum--;
                    emit errorOccurred(QString("写入图像层 %1 失败，结果：%2").arg(n).arg(writeResult));
                    return;
                }
                
                qDebug() << "[DataWriterThread] Successfully wrote layer" << n;
                layerWritten = true;
                
                // 发送进度信号
                emit writeProgress(n + 1, m_layerCounts);
            }
            else {
                // 缓冲区满，等待
                qDebug() << "[DataWriterThread] Buffer full (PrtBuffNum=5), waiting...";
                QThread::msleep(1);  // 与MFC一致，等待1ms
            }
        }
    }
    
    // ========== 阶段4: 写入完成 ==========
    qDebug() << "[DataWriterThread] All layers written, thread finishing";
    emit writeFinished();
}

