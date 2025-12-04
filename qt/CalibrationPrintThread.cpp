#include "CalibrationPrintThread.h"
#include "../Inc/RYPrtCtler.h"
#include <QDebug>
#include <QMutexLocker>

CalibrationPrintThread::CalibrationPrintThread(RYCalbrationParam params, QMutex* hwMutex, QObject *parent)
    : QThread(parent), m_params(params), m_hwMutex(hwMutex), m_stopped(false)
{
}

void CalibrationPrintThread::stop()
{
    m_stopped = true;
    // The hardware call is already locked in the run() method, but this provides a faster stop.
    IDP_StopPrintJob(); 
}

void CalibrationPrintThread::run()
{
    QMutexLocker hwLocker(m_hwMutex);

    int passCount = IDP_StartCalibration(&m_params);
    if (passCount < 0) {
        emit errorOccurred(QString("开始校准失败，错误码: %1").arg(passCount));
        return;
    }

    PrtRunInfo runInfo;
    for (int i = 0; i < passCount; ++i) {
        if (m_stopped) {
            break;
        }

        // Wait for the pass data to be ready
        LPPassDataItem passItem = nullptr;
        while (!m_stopped) {
            passItem = IDP_GetPassItem(0, i);
            if (passItem && (passItem->nProcState > 2 && passItem->nProcState < 6)) {
                break; // Pass is ready to be printed
            }
            msleep(10); // Don't burn CPU
        }

        if (m_stopped || !passItem) {
            break;
        }
        
        // Execute the pass print
        if (IDP_DoPassPrint(passItem) < 0) {
            emit errorOccurred("执行Pass打印失败 (IDP_DoPassPrint failed)");
            break;
        }

        // Wait for the pass to complete
        while (!m_stopped) {
            IDP_GetPrintState(&runInfo);
            if (runInfo.nPrtState != 1) { // 1 means printing
                break;
            }
            msleep(100);
        }
    }

    // Cleanup
    IDP_FreeImageLayer(-1);
    if (!m_stopped) {
         IDP_StopPrintJob();
    }
    
    emit printFinished();
}
