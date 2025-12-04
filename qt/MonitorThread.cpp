#include "MonitorThread.h"
#include "../Inc/RYPrtCtler.h"
#include <QMutexLocker>
#include <QDebug>

// 外部变量声明
extern LPPRINTER_INFO g_pSysInfo;
extern bool g_IsRoladWave;

MonitorThread::MonitorThread(QMutex* hwMutex, QObject *parent)
    : QThread(parent)
    , m_hwMutex(hwMutex)
    , m_stop(false)
    , m_closeAutoCheck(false)
{
}

MonitorThread::~MonitorThread()
{
    // The stop() and wait() logic will be handled by the main window destructor
    // to ensure proper shutdown order.
}

void MonitorThread::stop()
{
    QMutexLocker locker(&m_internalMutex);
    m_stop = true;
    m_condition.wakeAll();
}

void MonitorThread::setCloseAutoCheck(bool close)
{
    QMutexLocker locker(&m_internalMutex);
    m_closeAutoCheck = close;
}

bool MonitorThread::getCloseAutoCheck() const
{
    // This is called from within the run() loop, which is on this thread,
    // so no lock is needed here.
    return m_closeAutoCheck;
}

void MonitorThread::run()
{
    PrtRunInfo prtInfo;
    UINT nLastPrtState = 0xFF;
    DWORD dwChkTick = GetTickCount();
    bool lastConnectedState = false;
    
    while (true) {
        {
            QMutexLocker locker(&m_internalMutex);
            if (m_stop) break;
        }

        bool isConnected;
        bool shouldUpdateInfo = false;
        
        { // Hardware Access Lock
            QMutexLocker hwLocker(m_hwMutex);

            // 检查设备连接状态
            isConnected = DEV_DeviceIsConnected();
            
            // 获取打印状态
            if (!IDP_GetPrintState(&prtInfo)) {
                // Don't emit error continuously
                msleep(100);
                continue;
            }

            // 在空闲或完成状态下更新设备信息
            if (prtInfo.nPrtState == 0 || prtInfo.nPrtState == 3) {
                DWORD currentTick = GetTickCount();
                if ((currentTick - dwChkTick) > 200) {
                    if (!getCloseAutoCheck()) {
                        g_pSysInfo = DEV_GetDeviceInfo();
                        shouldUpdateInfo = true;
                    }
                    dwChkTick = currentTick;
                }
            }
        } // Hardware Access Lock END

        if (isConnected != lastConnectedState) {
            emit deviceConnectedChanged(isConnected);
            lastConnectedState = isConnected;
        }
        
        if (shouldUpdateInfo) {
             // 检查波形重载结果
            if (g_pSysInfo && g_pSysInfo->nHWInitResult == 17 && g_IsRoladWave) {
                emit errorOccurred("波形文件失败");
                g_IsRoladWave = false; // Reset flag
            }
            emit dataUpdated();
        }
        
        // 休眠100ms
        msleep(100);
    }
}

