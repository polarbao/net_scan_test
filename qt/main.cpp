#include <QApplication>
#include <QStyleFactory>
#include <QDir>
#include "MainWindow.h"
#include "../Inc/RYPrtCtler.h"

// 全局变量定义（与MFC版本保持一致）
RYUSR_PARAM g_sysParam;
PRTJOB_ITEM g_testJob;
PRTIMG_LAYER g_PrtImgLayer;
RYCalbrationParam g_Calbration;
LPPRINTER_INFO g_pSysInfo = nullptr;
bool g_bPHValid[MAX_PH_CNT] = { false };
HANDLE g_PrtMutex = INVALID_HANDLE_VALUE;
UINT g_nPHType = 0;
bool g_IsRoladWave = false;

/**
 * @brief 初始化图像图层信息
 */
void InitImgLayerInfo()
{
    memset(&g_PrtImgLayer, 0, sizeof(PRTIMG_LAYER));
    g_PrtImgLayer.nLayerIndex = -1;
    g_PrtImgLayer.nXDPI = 720;
    g_PrtImgLayer.nYDPI = 600;
    g_PrtImgLayer.nColorCnts = 1;
    g_PrtImgLayer.nGrayBits = 1;
    g_PrtImgLayer.nPrtDir = 1;
    g_PrtImgLayer.nValidClrMask = 0xFFFF;
    g_PrtImgLayer.fLayerDensity = 1.0f;
    g_PrtImgLayer.nPrtFlag = 1;
}

/**
 * @brief 初始化作业信息
 */
void InitJobInfo()
{
    memset(&g_testJob, 0, sizeof(PRTJOB_ITEM));
    g_testJob.nJobID = 0;
    strncpy_s(g_testJob.szJobName, "Scan Demo", sizeof(g_testJob.szJobName));
    g_testJob.fPrtXPos = 100.0f;
    g_testJob.fClipHeight = 0.0f;
    g_testJob.fClipWidth = 0.0f;
    g_testJob.fOutXdpi = 600.0f;
    g_testJob.fOutYdpi = 600.0f;
    g_testJob.nFileType = 0;
    g_testJob.nOutPixelBits = 1;
}

/**
 * @brief 初始化校准信息
 */
void InitCalibrationInfo()
{
    memset(&g_Calbration, 0, sizeof(RYCalbrationParam));
    g_Calbration.fPrtXPos = 10.0f;
    g_Calbration.fPrtYPos = 0.0f;
    g_Calbration.fStpSize = 0.0f;
    g_Calbration.fxadjdpi = 600.0f;
    g_Calbration.fXMaxPrtWidth = 600.0f;
    g_Calbration.fXRunSpd = 10.0f;
    g_Calbration.fYMaxPrtHeight = 600.0f;
    g_Calbration.nAdjType = 0;
    g_Calbration.nCtrlValue = 0;
    g_Calbration.nGrayBits = 1;
    g_Calbration.nPrtDir = 1;
}

int main(int argc, char *argv[])
{
    // 创建应用程序
    QApplication app(argc, argv);
    
    // 设置应用程序信息
    app.setApplicationName("NetScanDemo");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("RongYue");
    app.setOrganizationDomain("rongyue.com");
    
    // 设置样式（可选）
    // app.setStyle(QStyleFactory::create("Fusion"));
    
    // 初始化全局参数
    InitImgLayerInfo();
    InitJobInfo();
    InitCalibrationInfo();
    
    // 创建主窗口
    MainWindow window;
    window.show();
    
    // 进入事件循环
    return app.exec();
}

