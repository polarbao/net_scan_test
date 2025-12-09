/**
 * @file RYPrtCtler.h
 * @brief 融跃打印控制器SDK主头文件
 * @details 提供设备管理、图像打印、喷头控制等核心API
 * @version 1.0
 * @date 2024
 * @copyright 融跃科技
 */

#ifndef RYPRTCTLER_H
#define RYPRTCTLER_H

// DLL导入/导出宏定义
#ifdef RYPRTCTLER_EXPORTS
#define RYPRTCTLER_API extern "C" __declspec(dllexport)
#else
#define RYPRTCTLER_API extern "C" __declspec(dllimport)
#endif

#include "ryprtapi.h"

// ============================================================================
// 一、设备管理API
// ============================================================================

/**
 * @brief 打开并初始化打印设备
 * 
 * @param notifyhand 通知句柄（可选，用于异步通知回调）
 * @param szDllFilePath DLL和配置文件所在目录路径
 * 
 * @return int 返回值说明：
 *         - 1: 设备打开成功
 *         - -2: pcs配置文件加载失败
 *         - -3: 打开设备卡连接失败
 *         - -4: graycfg灰度配置文件加载失败
 *         - -5: 设备数量超过许可限制
 * 
 * @note 必须在所有其他API调用之前执行
 * @note szDllFilePath指向的目录必须包含pcs文件和graycfg文件
 * @warning 全局只能打开一次设备
 * 
 * @example
 * @code
 * int result = DEV_OpenDevice(NULL, "C:\\Config\\");
 * if (result == 1) {
 *     // 设备打开成功，可以继续初始化
 * } else {
 *     // 根据返回值处理错误
 *     switch(result) {
 *         case -2: printf("配置文件加载失败\n"); break;
 *         case -3: printf("设备连接失败\n"); break;
 *         // ...
 *     }
 * }
 * @endcode
 */
RYPRTCTLER_API int __stdcall DEV_OpenDevice(void* notifyhand, unsigned char* szDllFilePath);

/**
 * @brief 初始化设备，设置编码器初始位置
 * 
 * @param nXEncInitVal X轴编码器初始位置值，通常设为0或特定起始位置
 * 
 * @return int 返回值说明：
 *         - >= 0: 初始化成功
 *         - < 0: 初始化失败，返回错误码
 * 
 * @note 必须在DEV_OpenDevice成功后调用
 * @note 编码器值用于精确的打印位置控制
 * 
 * @example
 * @code
 * // 设置编码器初始位置为0
 * int result = DEV_InitDevice(0);
 * if (result >= 0) {
 *     printf("设备初始化成功\n");
 * }
 * @endcode
 */
RYPRTCTLER_API int __stdcall DEV_InitDevice(unsigned int nXEncInitVal);

/**
 * @brief 检查设备是否连接
 * 
 * @return bool 连接状态：
 *         - true: 设备已连接且正常工作
 *         - false: 设备未连接或连接异常
 * 
 * @note 建议在MonitorThread中每100-200ms检查一次
 * @warning 不要在打印关键路径中频繁调用，可能影响性能
 * 
 * @example
 * @code
 * // 在监控线程中定期检查
 * while (!stopMonitor) {
 *     bool isConnected = DEV_DeviceIsConnected();
 *     if (!isConnected) {
 *         // 处理设备断开
 *         emit deviceDisconnected();
 *     }
 *     Sleep(200);
 * }
 * @endcode
 */
RYPRTCTLER_API bool __stdcall DEV_DeviceIsConnected();

/**
 * @brief 关闭设备连接，释放所有资源
 * 
 * @return bool 操作结果：
 *         - true: 关闭成功
 *         - false: 关闭失败
 * 
 * @note 关闭前应停止所有打印作业和运动控制
 * @note 应用程序退出前必须调用
 * 
 * @example
 * @code
 * // 安全关闭设备
 * IDP_StopPrintJob();           // 1. 停止打印
 * IDP_FreeImageLayer(-1);       // 2. 释放所有图层
 * DEM_StopAll(TRUE);            // 3. 停止所有运动
 * bool result = DEV_CloseDevice(); // 4. 关闭设备
 * @endcode
 */
RYPRTCTLER_API bool __stdcall DEV_CloseDevice();

/**
 * @brief 更新设备系统参数
 * 
 * @param pParam 指向RYUSR_PARAM结构的指针，包含所有系统配置参数
 * 
 * @return bool 操作结果：
 *         - true: 更新成功
 *         - false: 更新失败
 * 
 * @note 修改喷头偏移、打印速度、温度电压等参数后需要调用
 * @note 参数会立即生效
 * 
 * @see RYUSR_PARAM 结构定义
 * 
 * @example
 * @code
 * // 修改打印速度后更新参数
 * g_sysParam.fPrtScanSpd = 15.0f;
 * bool result = DEV_UpdateParam(&g_sysParam);
 * @endcode
 */
RYPRTCTLER_API bool __stdcall DEV_UpdateParam(LPRYUSR_PARAM pParam);

/**
 * @brief 获取当前设备参数
 * 
 * @param pParam 输出参数，接收当前系统参数
 * 
 * @return bool 操作结果：
 *         - true: 获取成功
 *         - false: 获取失败
 * 
 * @note 用于读取当前配置以供修改或保存
 * 
 * @example
 * @code
 * RYUSR_PARAM currentParam;
 * if (DEV_GetCurParam(&currentParam)) {
 *     // 使用获取的参数
 *     float speed = currentParam.fPrtScanSpd;
 * }
 * @endcode
 */
RYPRTCTLER_API bool __stdcall DEV_GetCurParam(LPRYUSR_PARAM pParam);

/**
 * @brief 获取设备详细信息（状态、温度、电压等）
 * 
 * @return LPPRINTER_INFO 指向PRINTER_INFO结构的指针，包含设备完整状态
 * 
 * @note 返回的指针指向SDK内部内存，不要手动释放
 * @note 建议在监控线程中定期调用（每200ms）
 * 
 * @see PRINTER_INFO 结构定义
 * 
 * @example
 * @code
 * // 在监控线程中使用
 * LPPRINTER_INFO pInfo = DEV_GetDeviceInfo();
 * if (pInfo) {
 *     // 获取系统状态
 *     unsigned int sysStatus = pInfo->nSysStatus;
 *     // 获取打印状态
 *     unsigned int printStatus = pInfo->nPrintStatus;
 *     // 获取温度信息
 *     float temp = pInfo->sysDrvInfo[0][0].phinfo[0].fCurTemp[0];
 * }
 * @endcode
 */
RYPRTCTLER_API LPPRINTER_INFO __stdcall DEV_GetDeviceInfo();

/**
 * @brief 重新加载喷头波形文件
 * 
 * @param nClrMask 颜色掩码，bit0-bit15对应color0-color15
 *                 例如：0x0001表示color0，0x0003表示color0和color1
 * @param nPHGMask 喷头组掩码，bit0-bit7对应group0-group7
 * @param szWaveFile 波形文件完整路径
 * 
 * @return int 返回值说明：
 *         - >= 0: 重载成功
 *         - < 0: 重载失败，返回错误码
 * 
 * @note 更换喷头或优化打印质量时使用
 * @warning 波形文件必须与喷头类型匹配
 * 
 * @example
 * @code
 * // 重载颜色0的所有喷头组的波形
 * int result = DEV_ReloadWaveForm(0x0001, 0xFF, "C:\\Waves\\color0.wav");
 * if (result >= 0) {
 *     printf("波形重载成功\n");
 * }
 * @endcode
 */
RYPRTCTLER_API int __stdcall DEV_ReloadWaveForm(unsigned nClrMask, unsigned int nPHGMask, char* szWaveFile);

/**
 * @brief 重置打印机编码器位置值
 * 
 * @param nPos 新的编码器位置值，通常设为0
 * 
 * @return bool 操作结果：
 *         - true: 重置成功
 *         - false: 重置失败
 * 
 * @note 用于回零操作后重置编码器
 * @note 开始新的打印作业前建议重置
 * 
 * @example
 * @code
 * // 回到零位后重置编码器
 * if (atHomePosition) {
 *     DEV_ResetPrinterEncValue(0);
 * }
 * @endcode
 */
RYPRTCTLER_API bool __stdcall DEV_ResetPrinterEncValue(unsigned int nPos);

/**
 * @brief 设置虚拟打印编码器（测试用）
 * 
 * @param bEnable 是否启用虚拟编码器（true:虚拟 false:实际）
 * @param bEncDir 编码器方向（true:正向 false:负向）
 * @param nClkFreq 虚拟编码器时钟频率（Hz）
 * 
 * @return bool 操作结果：
 *         - true: 设置成功
 *         - false: 设置失败
 * 
 * @note 用于无实际编码器时的测试和调试
 * @warning 生产环境应使用实际编码器
 * 
 * @example
 * @code
 * // 启用虚拟编码器用于测试
 * DEV_SetVirtualPrtEncoder(true, true, 100000);
 * @endcode
 */
RYPRTCTLER_API bool __stdcall DEV_SetVirtualPrtEncoder(bool bEnable, bool bEncDir, unsigned int nClkFreq);

/**
 * @brief 查询当前打印机编码器位置值
 * 
 * @return unsigned int 当前编码器值（32位无符号整数）
 * 
 * @note 用于运动控制中计算目标位置和距离
 * @note 编码器值会随着打印头移动实时更新
 * 
 * @example
 * @code
 * // 计算到目标位置的距离
 * unsigned int currentPos = DEV_GetPrinterEncValue();
 * unsigned int targetPos = 10000;
 * int distance = targetPos - currentPos;
 * 
 * // 执行X轴运动
 * DEM_MoveX(FALSE, speed, distance, TRUE, 1.0f);
 * @endcode
 */
RYPRTCTLER_API unsigned int __stdcall DEV_GetPrinterEncValue();

// ============================================================================
// 二、图像打印API（核心功能）
// ============================================================================

/**
 * @brief 启动打印作业（必须首先调用）
 * 
 * @param pJobItem 指向PRTJOB_ITEM结构的指针，包含作业配置
 * 
 * @return int 返回值说明：
 *         - >= 0: 启动成功
 *         - < 0: 启动失败，返回错误码
 * 
 * @note 【关键API】在双线程架构中由PrintThreadSeparate/DataWriterThread调用
 * @note 必须在写入任何图像数据之前调用
 * @note 全局只能有一个活动作业
 * @note 作业参数在整个打印过程中保持有效
 * 
 * @see PRTJOB_ITEM 作业参数结构
 * 
 * @example
 * @code
 * // 配置打印作业
 * g_testJob.nJobID = 0;
 * strcpy(g_testJob.szJobName, "Test Job");
 * g_testJob.fPrtXPos = 100.0f;  // X轴起始位置(mm)
 * g_testJob.fOutXdpi = 600.0f;
 * g_testJob.fOutYdpi = 600.0f;
 * g_testJob.nPrtCtl = 0x01;     // bit0:白色跳过
 * 
 * // 启动作业
 * int result = IDP_SartPrintJob(&g_testJob);
 * if (result >= 0) {
 *     printf("打印作业启动成功\n");
 *     // 可以开始写入图层数据
 * }
 * @endcode
 */
RYPRTCTLER_API int __stdcall IDP_SartPrintJob(LPPRTJOB_ITEM pJobItem);

/**
 * @brief 停止当前打印作业
 * 
 * @return bool 操作结果：
 *         - true: 停止成功
 *         - false: 停止失败
 * 
 * @note 用户中断或打印完成时调用
 * @note 应该在释放图层资源之前调用
 * 
 * @example
 * @code
 * // 正常完成流程
 * IDP_StopPrintJob();           // 1. 停止作业
 * IDP_FreeImageLayer(-1);       // 2. 释放所有图层
 * CloseHandle(g_PrtMutex);      // 3. 关闭互斥锁
 * @endcode
 */
RYPRTCTLER_API bool __stdcall IDP_StopPrintJob();

/**
 * @brief 将图像图层数据写入设备缓冲区（生产者操作）
 * 
 * @param lpLayerInfo 图层配置信息结构指针
 * @param pSrcBuf 图像数据指针数组，每个元素对应一个颜色通道
 * @param nBytes 每个颜色通道的数据字节数
 * 
 * @return int 返回值说明：
 *         - 1: 数据写入成功
 *         - 0: 该图层已存在
 *         - -110001: 图层信息为空
 *         - -110002: 图层编号不在有效范围内
 *         - -110004: PC内存不足
 *         - -110005: 图像预处理错误
 * 
 * @note 【关键API】在双线程架构中由PrintThreadSeparate/DataWriterThread调用
 * @note **必须在互斥锁保护下调用**
 * @note 写入成功后应将PrtBuffNum递增
 * @note 最多可缓存5个图层（PrtBuffNum <= 5）
 * @note 图像数据必须是单色位图格式（1bit）
 * 
 * @see PRTIMG_LAYER 图层信息结构
 * 
 * @example
 * @code
 * // 准备图层信息
 * g_PrtImgLayer.nLayerIndex = 0;
 * g_PrtImgLayer.nColorCnts = 1;
 * g_PrtImgLayer.nXDPI = 720;
 * g_PrtImgLayer.nYDPI = 600;
 * g_PrtImgLayer.nWidth = 1000;
 * g_PrtImgLayer.nHeight = 1000;
 * g_PrtImgLayer.nBytesPerLine = 128;
 * g_PrtImgLayer.nGrayBits = 1;
 * g_PrtImgLayer.nValidClrMask = 0xFFFF;
 * g_PrtImgLayer.nPrtDir = 1;
 * 
 * // 准备图像数据
 * LPBYTE pBmpFile[MAX_COLORS];
 * pBmpFile[0] = imageData;
 * 
 * // 等待缓冲区有空间
 * while (PrtBuffNum >= 5) {
 *     Sleep(1);
 * }
 * 
 * // 写入数据（需要互斥锁保护）
 * PrtBuffNum++;
 * WaitForSingleObject(g_PrtMutex, INFINITE);
 * int result = IDP_WriteImgLayerData(&g_PrtImgLayer, pBmpFile, dataSize);
 * ReleaseMutex(g_PrtMutex);
 * 
 * if (result != 1) {
 *     PrtBuffNum--;  // 写入失败，恢复计数
 *     // 错误处理
 * }
 * @endcode
 */
RYPRTCTLER_API int __stdcall IDP_WriteImgLayerData(LPPRTIMG_LAYER lpLayerInfo, unsigned char* pSrcBuf[], int nBytes);

/**
 * @brief 从文件写入图层数据
 * 
 * @param lpLayerInfo 图层配置信息
 * @param szFile 图像文件路径（支持BMP等格式）
 * 
 * @return int 返回值同IDP_WriteImgLayerData
 * 
 * @note 直接从文件加载，无需预加载到内存
 * @note 文件必须是单色位图格式
 * 
 * @example
 * @code
 * g_PrtImgLayer.nLayerIndex = 0;
 * // ... 设置其他参数 ...
 * 
 * int result = IDP_WriteImgLayerFile(&g_PrtImgLayer, "C:\\Images\\layer0.bmp");
 * @endcode
 */
RYPRTCTLER_API int __stdcall IDP_WriteImgLayerFile(LPPRTIMG_LAYER lpLayerInfo, char* szFile);

/**
 * @brief 启动指定图层的打印处理（消费者操作）
 * 
 * @param nLayerIndex 要打印的图层索引（从0开始）
 * 
 * @return int 返回值说明：
 *         - > 0: 成功，返回该图层的PASS数量
 *         - -1: 图像数据未准备好（需要重试）
 *         - -120000 ~ -120016: 启动失败（不同错误类型）
 *         - -121000: 板卡内存分配失败
 *         - -122000: 创建数据发送线程失败
 * 
 * @note 【关键API】在双线程架构中由PrintThread/PrintExecutorThread调用
 * @note **必须在IDP_WriteImgLayerData之后调用**
 * @note **必须在互斥锁保护下调用**
 * @note 返回-1时应该重试（数据未就绪）
 * @note 返回的PASS数量用于后续的PASS循环
 * 
 * @example
 * @code
 * // 等待数据就绪
 * while (PrtBuffNum <= 0) {
 *     Sleep(10);
 * }
 * 
 * // 获取互斥锁
 * WaitForSingleObject(g_PrtMutex, INFINITE);
 * 
 * // 启动图层打印
 * int nPassCount = IDP_StartLayerPrint(layerIndex);
 * 
 * // 重试机制（如果数据未就绪）
 * while (nPassCount == -1) {
 *     nPassCount = IDP_StartLayerPrint(layerIndex);
 * }
 * 
 * if (nPassCount > 0) {
 *     printf("图层%d有%d个PASS需要打印\n", layerIndex, nPassCount);
 *     // 进入PASS循环
 * }
 * 
 * // 使用完毕后释放互斥锁
 * ReleaseMutex(g_PrtMutex);
 * @endcode
 */
RYPRTCTLER_API int __stdcall IDP_StartLayerPrint(int nLayerIndex);

/**
 * @brief 获取指定PASS的详细数据项
 * 
 * @param nLayerIndex 图层索引
 * @param nPassID PASS索引（从0到nPassCount-1）
 * 
 * @return LPPassDataItem 指向PassDataItem结构的指针
 * 
 * @note 【关键API】在PASS循环中调用
 * @note 返回的指针指向SDK内部内存，不要手动释放
 * @note **必须循环等待nProcState进入就绪状态（3-5）**
 * @note 如果bIsNullPass为true，应跳过该PASS
 * 
 * @see PassDataItem PASS数据结构
 * 
 * @example
 * @code
 * LPPassDataItem pPassItem = nullptr;
 * 
 * // 等待PASS就绪（状态3,4,5可以打印）
 * do {
 *     pPassItem = IDP_GetPassItem(layerIndex, passIndex);
 *     if (pPassItem) {
 *         if ((pPassItem->nProcState > 2) && (pPassItem->nProcState < 6)) {
 *             break;  // PASS就绪
 *         }
 *     }
 *     if (!jobStarted) {
 *         return;  // 作业被停止
 *     }
 *     // 注意：MFC中这里没有Sleep，紧密循环保证快速响应
 * } while (true);
 * 
 * // 检查是否为空PASS
 * if (pPassItem->bIsNullPass) {
 *     continue;  // 跳过空PASS
 * }
 * 
 * // 使用PASS数据进行打印
 * printf("PASS%d 打印方向: %s\n", passIndex, 
 *        pPassItem->bEncPrtDir ? "正向" : "负向");
 * printf("Y轴步进: %d um\n", pPassItem->nStpVector);
 * printf("起始编码器位置: %u\n", pPassItem->nStartEncPos);
 * @endcode
 */
RYPRTCTLER_API LPPassDataItem __stdcall IDP_GetPassItem(unsigned int nLayerIndex, int nPassID);

/**
 * @brief 执行指定PASS的打印
 * 
 * @param pPassItem 从IDP_GetPassItem获取的PASS数据项
 * 
 * @return int 返回值说明：
 *         - > 0: 打印成功
 *         - <= 0: 打印失败
 * 
 * @note 【关键API】在PASS就绪后调用
 * @note **必须在运动控制完成后调用**
 * @note 打印期间硬件会自动控制喷头喷射
 * 
 * @example
 * @code
 * // 完整的PASS打印流程
 * LPPassDataItem pPassItem = IDP_GetPassItem(layerIndex, passIndex);
 * 
 * // 1. Y轴运动控制（如果不是第一个PASS）
 * #ifdef RY_MOVE_CTL
 * if (passIndex > 0) {
 *     int nYStep = MM_TO_DOT(pPassItem->nStpVector / 1000.0, 
 *                            g_movConfig.fySysdpi) * 48;
 *     int nSpeed = MM_TO_DOT(g_movConfig.fyMovSpd, g_movConfig.fySysdpi) * 48;
 *     DEM_MoveY(TRUE, nSpeed, nYStep, true, false, g_movConfig.fyMovRate);
 * }
 * 
 * // 2. X轴定位
 * int nMovBuf = MM_TO_DOT(g_movConfig.fMovBuf, g_movConfig.fxSysdpi);
 * int nDestPos;
 * if (pPassItem->bEncPrtDir) {
 *     nDestPos = pPassItem->nStartEncPos + ZERO_POSITION - nMovBuf;
 * } else {
 *     nDestPos = pPassItem->nStartEncPos + ZERO_POSITION + nMovBuf;
 * }
 * int ndistance = nDestPos - DEV_GetPrinterEncValue();
 * int nXSpeed = MM_TO_DOT(g_movConfig.fxMovSpd, g_movConfig.fxSysdpi);
 * DEM_MoveX(FALSE, nXSpeed, ndistance, TRUE, g_movConfig.fxMovRate);
 * #endif
 * 
 * // 3. 执行打印
 * int result = IDP_DoPassPrint(pPassItem);
 * if (result > 0) {
 *     printf("PASS%d打印成功\n", passIndex);
 *     
 *     // 4. X轴回退（可选）
 *     #ifdef RY_MOVE_CTL
 *     int nMoveLen = pPassItem->nXPrtColumns * 
 *                    pPassItem->fPrtPrecession + nMovBuf * 2;
 *     DEM_MoveX(!pPassItem->bEncPrtDir, nXSpeed, nMoveLen, 
 *               TRUE, g_movConfig.fxMovRate);
 *     #endif
 * }
 * @endcode
 */
RYPRTCTLER_API int __stdcall IDP_DoPassPrint(LPPassDataItem pPassItem);

/**
 * @brief 获取打印运行状态
 * 
 * @param pRTinfo 输出参数，填充打印运行信息
 * 
 * @return bool 操作结果：
 *         - true: 获取成功
 *         - false: 获取失败
 * 
 * @note 用于监控打印进度和状态
 * @note nPrtState: 0=空闲 1=打印中 2=暂停 3=冲洗
 * 
 * @see PrtRunInfo 打印运行信息结构
 * 
 * @example
 * @code
 * PrtRunInfo prtInfo;
 * 
 * // 监控打印状态，等待PASS打印完成
 * do {
 *     IDP_GetPrintState(&prtInfo);
 *     if (prtInfo.nPrtState == 1) {
 *         // 正在打印，继续等待
 *         printf("打印进度: 图层%d PASS%d\n", 
 *                prtInfo.nPrintLayerIndex, prtInfo.nPrintPassIndex);
 *         Sleep(10);
 *     } else {
 *         // 打印完成或其他状态
 *         break;
 *     }
 * } while (true);
 * @endcode
 */
RYPRTCTLER_API bool __stdcall IDP_GetPrintState(LPPrtRunInfo pRTinfo);

/**
 * @brief 释放图像图层资源
 * 
 * @param nLayerIndex 图层索引
 *                    - >= 0: 释放指定图层
 *                    - -1: 释放所有图层
 * 
 * @return int 返回值说明：
 *         - >= 0: 释放成功
 *         - < 0: 释放失败
 * 
 * @note 每个图层打印完成后必须释放
 * @note 释放后PrtBuffNum应该递减
 * @note 打印作业结束时使用-1释放所有图层
 * 
 * @example
 * @code
 * // 打印完一个图层后释放
 * IDP_FreeImageLayer(layerIndex);
 * PrtBuffNum--;  // 缓冲区计数减少
 * 
 * // 打印作业结束时释放所有
 * IDP_StopPrintJob();
 * IDP_FreeImageLayer(-1);  // 释放所有图层
 * @endcode
 */
RYPRTCTLER_API int __stdcall IDP_FreeImageLayer(int nLayerIndex);

/**
 * @brief 启动校准打印
 * 
 * @param pParam 校准参数结构指针
 * 
 * @return int 返回值说明：
 *         - >= 0: 启动成功
 *         - < 0: 启动失败
 * 
 * @note 用于设备校准和测试
 * @note 校准类型：0=状态图 1=垂直校准 2=角度 3=X双向颜色 4=X距离 5=Y距离
 * 
 * @see RYCalbrationParam 校准参数结构
 * 
 * @example
 * @code
 * RYCalbrationParam calibParam;
 * calibParam.nAdjType = 1;      // 垂直校准
 * calibParam.fPrtXPos = 10.0f;
 * calibParam.fPrtYPos = 0.0f;
 * calibParam.fxadjdpi = 600.0f;
 * calibParam.fStpSize = 0.1f;   // 步进0.1mm
 * 
 * int result = IDP_StartCalibration(&calibParam);
 * @endcode
 */
RYPRTCTLER_API int __stdcall IDP_StartCalibration(LPRYCalbrationParam pParam);

/**
 * @brief 控制喷头冲洗功能
 * 
 * @param bOpen 冲洗开关（true:开启 false:关闭）
 * 
 * @return bool 操作结果：
 *         - true: 操作成功
 *         - false: 操作失败
 * 
 * @note 用于保持喷嘴湿润，防止堵塞
 * @note 长时间不打印前应开启冲洗
 * 
 * @example
 * @code
 * // 开启冲洗
 * IDP_FlashPrtCtl(true);
 * Sleep(5000);  // 冲洗5秒
 * IDP_FlashPrtCtl(false);
 * @endcode
 */
RYPRTCTLER_API bool __stdcall IDP_FlashPrtCtl(bool bOpen);

/**
 * @brief 设置校准数据回调函数
 * 
 * @param pFun 回调函数指针
 * 
 * @note 回调函数接收校准打印生成的图像数据
 * @note 用于自动分析校准结果
 * 
 * @see LPRY_CALI_PHGDCall 回调函数类型定义
 */
RYPRTCTLER_API void __stdcall IDP_SetCaliDataCallBack(LPRY_CALI_PHGDCall pFun);

// ============================================================================
// 三、温度电压控制API
// ============================================================================

/**
 * @brief 更新喷头电压
 * 
 * @param fstdVcom 标准电压数组（最多32个值）
 * @param nPhID 喷头ID
 * 
 * @return bool 操作结果：
 *         - true: 更新成功
 *         - false: 更新失败
 * 
 * @note 用于调整喷头电压以优化打印质量
 * @note 电压范围取决于具体喷头型号
 * 
 * @example
 * @code
 * float voltages[32];
 * voltages[0] = 12.5f;  // A通道电压
 * voltages[1] = 12.3f;  // B通道电压
 * // ... 设置其他通道 ...
 * 
 * MVT_UpdatePhVoltage(voltages, phID);
 * @endcode
 */
RYPRTCTLER_API bool __stdcall MVT_UpdatePhVoltage(float* fstdVcom, unsigned int nPhID);

/**
 * @brief 设置喷头标准温度
 * 
 * @param fstdTmp 标准温度数组（最多4个值）
 * @param nPhID 喷头ID
 * 
 * @return bool 操作结果：
 *         - true: 设置成功
 *         - false: 设置失败
 * 
 * @note 温度控制保证打印质量稳定性
 * @note 温度单位：摄氏度
 * 
 * @example
 * @code
 * float temps[4];
 * temps[0] = 35.0f;  // 喷头目标温度35°C
 * 
 * MVT_SetPhStdTemp(temps, phID);
 * @endcode
 */
RYPRTCTLER_API bool __stdcall MVT_SetPhStdTemp(float* fstdTmp, unsigned int nPhID);

/**
 * @brief 获取喷头当前电压
 * 
 * @param fCurVcom 输出参数，接收当前电压值（最多32个）
 * @param nPhID 喷头ID
 * 
 * @return bool 操作结果：
 *         - true: 获取成功
 *         - false: 获取失败
 * 
 * @note 用于监控喷头电压状态
 * 
 * @example
 * @code
 * float currentVoltages[32];
 * if (MVT_CurPhVoltage(currentVoltages, phID)) {
 *     printf("A通道电压: %.2fV\n", currentVoltages[0]);
 * }
 * @endcode
 */
RYPRTCTLER_API bool __stdcall MVT_CurPhVoltage(float* fCurVcom, unsigned int nPhID);

/**
 * @brief 获取喷头当前温度
 * 
 * @param fCurTemp 输出参数，接收当前温度值（最多4个）
 * @param nPhID 喷头ID
 * 
 * @return bool 操作结果：
 *         - true: 获取成功
 *         - false: 获取失败
 * 
 * @note 用于温度监控和过热保护
 * 
 * @example
 * @code
 * float currentTemps[4];
 * if (MVT_GetCurPhTemp(currentTemps, phID)) {
 *     printf("喷头温度: %.1f°C\n", currentTemps[0]);
 *     if (currentTemps[0] > 45.0f) {
 *         // 过热警告
 *     }
 * }
 * @endcode
 */
RYPRTCTLER_API bool __stdcall MVT_GetCurPhTemp(float* fCurTemp, unsigned int nPhID);

// ============================================================================
// 四、气压和IO控制API
// ============================================================================

/**
 * @brief 设置气压控制值
 * 
 * @param nPrtIndex 打印控制器索引
 * 
 * @return bool 操作结果：
 *         - true: 设置成功
 *         - false: 设置失败
 * 
 * @note 用于气压辅助喷射系统
 */
RYPRTCTLER_API bool __stdcall MVT_SetAirPressCtlVal(unsigned int nPrtIndex);

/**
 * @brief 手动控制输出（UV灯、风扇等外设）
 * 
 * @param nPrtIndex 打印控制器索引
 * @param nOutMask 输出通道掩码
 * @param bEnable 启用/禁用
 * 
 * @return bool 操作结果：
 *         - true: 控制成功
 *         - false: 控制失败
 * 
 * @note 用于手动控制外部设备
 * 
 * @example
 * @code
 * // 打开UV灯（假设在bit0）
 * MVT_SetOutPut(0, 0x01, true);
 * @endcode
 */
RYPRTCTLER_API bool __stdcall MVT_SetOutPut(int nPrtIndex, int nOutMask, bool bEnable);

/**
 * @brief 设置输出配置
 * 
 * @param nPrtIndex 打印控制器索引
 * 
 * @return bool 操作结果：
 *         - true: 设置成功
 *         - false: 设置失败
 */
RYPRTCTLER_API bool __stdcall MVT_SetOutPutConfig(int nPrtIndex);

// ============================================================================
// 五、ADIB墨水管理API
// ============================================================================

/**
 * @brief ADIB板卡控制（墨水供应管理）
 * 
 * @param lParam ADIB参数结构指针
 * @param nOption 操作选项
 * @param bSetParam true:设置参数 false:获取参数
 * @param bAbort 中止标志指针
 * @param nPrtIndex 打印控制器索引
 * 
 * @return bool 操作结果：
 *         - true: 操作成功
 *         - false: 操作失败
 * 
 * @note 用于墨水供应系统管理
 * @note ADIB板卡负责墨水供应、液位监测、气压控制等
 * 
 * @see ADIB_PARAM ADIB参数结构
 */
RYPRTCTLER_API bool __stdcall DEV_AdibControl(
    LPADIB_PARAM lParam, 
    unsigned int nOption, 
    bool bSetParam, 
    bool* bAbort, 
    unsigned int nPrtIndex
);

/**
 * @brief 启用墨水观察（喷射状态检测）
 * 
 * @param bOpen 开启/关闭墨水观察
 * @param nPrtIndex 打印控制器索引
 * @param nDrvIndex 驱动器索引
 * 
 * @return bool 操作结果：
 *         - true: 操作成功
 *         - false: 操作失败
 * 
 * @note 用于检测喷嘴喷射状态和缺喷检测
 */
RYPRTCTLER_API bool __stdcall DEV_EnableInkWatch(
    bool bOpen, 
    unsigned int nPrtIndex, 
    unsigned int nDrvIndex
);

// ============================================================================
// 六、系统诊断和固件升级API
// ============================================================================

/**
 * @brief 生成所有寄存器报告
 * 
 * @param szFilePath 报告保存路径
 * 
 * @note 用于系统诊断和故障排查
 * @note 报告包含所有设备寄存器的当前值
 * 
 * @example
 * @code
 * DEV_GenAllRegReport("C:\\Logs\\reg_report.txt");
 * @endcode
 */
RYPRTCTLER_API void __stdcall DEV_GenAllRegReport(const char* szFilePath);

/**
 * @brief 更新设备应用程序固件
 * 
 * @param szFilePath APP固件文件路径
 * @param nPrtIndex 打印控制器索引
 * 
 * @return int 返回值说明：
 *         - >= 0: 更新成功
 *         - < 0: 更新失败
 * 
 * @warning 固件升级有风险，确保文件正确且设备稳定
 * @note 升级过程中不要断电或中断
 */
RYPRTCTLER_API int __stdcall DEV_UpdateAppFile(const char* szFilePath, unsigned int nPrtIndex);

/**
 * @brief 更新FPGA固件
 * 
 * @param szFilePath FPGA固件文件路径
 * @param nPrtIndex 打印控制器索引
 * 
 * @return int 返回值说明：
 *         - >= 0: 更新成功
 *         - < 0: 更新失败
 * 
 * @warning 固件升级有风险，确保文件正确且设备稳定
 * @note 升级过程中不要断电或中断
 */
RYPRTCTLER_API int __stdcall DEV_UpdateFpgaFile(const char* szFilePath, unsigned int nPrtIndex);

#endif // RYPRTCTLER_H

