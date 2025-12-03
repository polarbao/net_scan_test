#include "MonitorThread.h"
#include "../Inc/RYPrtCtler.h"
#include <QMutexLocker>
#include <QDebug>
#include <QDateTime>
#include <Windows.h>

// 外部变量声明
extern LPPRINTER_INFO g_pSysInfo;
extern bool g_IsRoladWave;
extern int g_nPHType;

MonitorThread::MonitorThread(QObject *parent)
    : QThread(parent)
    , m_stop(false)
    , m_closeAutoCheck(false)
{
}

MonitorThread::~MonitorThread()
{
    stop();
    
    // 等待线程结束，设置超时避免无限等待
    if (!wait(5000)) {
        qWarning() << "MonitorThread did not finish in time, terminating...";
        terminate();
        wait(1000);
    }
}

void MonitorThread::stop()
{
    QMutexLocker locker(&m_mutex);
    m_stop = true;
    m_condition.wakeAll();
}

bool MonitorThread::isStopped() const
{
    QMutexLocker locker(&m_mutex);
    return m_stop;
}

bool MonitorThread::shouldStop() const
{
    QMutexLocker locker(&m_mutex);
    return m_stop;
}

void MonitorThread::setCloseAutoCheck(bool close)
{
    QMutexLocker locker(&m_mutex);
    m_closeAutoCheck = close;
}

bool MonitorThread::getCloseAutoCheck() const
{
    QMutexLocker locker(&m_mutex);
    return m_closeAutoCheck;
}

void MonitorThread::run()
{
    PrtRunInfo prtInfo;
    UINT nLastPrtState = 0xFF;
    DWORD dwChkTick = GetTickCount();
    bool lastConnectedState = false;
    
    while (!shouldStop()) {
        // 检查设备连接状态
        bool isConnected = DEV_DeviceIsConnected();
        if (isConnected != lastConnectedState) {
            emit deviceConnectedChanged(isConnected);
            lastConnectedState = isConnected;
        }
        
        // 获取打印状态
        if (!IDP_GetPrintState(&prtInfo)) {
            emit errorOccurred("IDP_GetPrintState error");
            msleep(100);
            continue;
        }
        
        // 在空闲或完成状态下更新设备信息
        if (prtInfo.nPrtState == 0 || prtInfo.nPrtState == 3) {
            DWORD currentTick = GetTickCount();
            if ((currentTick - dwChkTick) > 200) {
                // 检查是否需要关闭自动检查
                if (!getCloseAutoCheck()) {
                    updateDeviceInfo();
                }
                dwChkTick = currentTick;
            }
        }
        
        // 检查波形重载结果
        if (g_pSysInfo && g_pSysInfo->nHWInitResult == 17 && g_IsRoladWave) {
            emit errorOccurred("波形文件失败");
            g_IsRoladWave = false;
        }
        
        // 更新打印状态（如果需要）
        if (prtInfo.nPrtState != nLastPrtState) {
            nLastPrtState = prtInfo.nPrtState;
            // 可以通过信号发送状态变化
        }
        
        // 发送数据更新信号
        emit dataUpdated();
        
        // 休眠100ms
        msleep(100);
    }
}

void MonitorThread::checkDeviceStatus()
{
    // 检查设备连接状态
    bool isConnected = DEV_DeviceIsConnected();
    emit deviceConnectedChanged(isConnected);
}

void MonitorThread::updateDeviceInfo()
{
    // 获取设备信息（注意：g_pSysInfo是全局变量，多线程访问需要保护）
    // 这里假设DEV_GetDeviceInfo()是线程安全的，或者返回的是只读数据
    LPPRINTER_INFO sysInfo = DEV_GetDeviceInfo();
    
    if (sysInfo) {
        // 更新全局变量（需要保护，但为了兼容MFC版本，暂时直接赋值）
        // 理想情况下应该使用互斥锁保护
        g_pSysInfo = sysInfo;
        
        // 获取编码器值
        int encoderValue = DEV_GetPrinterEncValue();
        
        // 发送数据更新信号，让MainWindow更新界面
        emit dataUpdated();
    }
}

