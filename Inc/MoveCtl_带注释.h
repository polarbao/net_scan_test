/**
 * @file MoveCtl.h
 * @brief 运动控制API头文件
 * @details 提供X/Y轴运动控制、限位检测、位置查询等功能
 * @version 1.0
 * @date 2024
 * @copyright 融跃科技
 */

#ifndef MOVECTL_H
#define MOVECTL_H

#ifdef RYPRTCTLER_EXPORTS
#define RYPRTCTLER_API extern "C" __declspec(dllexport)
#else
#define RYPRTCTLER_API extern "C" __declspec(dllimport)
#endif

// ============================================================================
// 一、系统初始化API
// ============================================================================

/**
 * @brief 初始化运动控制系统
 * 
 * @param szDllFile 运动控制DLL文件路径
 * 
 * @return int 返回值说明：
 *         - >= 0: 初始化成功
 *         - < 0: 初始化失败
 * 
 * @note 必须在DEV_OpenDevice之后调用
 * @note 必须在开始任何运动前调用
 * 
 * @example
 * @code
 * int result = DEM_Initialize("C:\\Libs\\MoveCtl.dll");
 * if (result >= 0) {
 *     printf("运动控制初始化成功\n");
 * }
 * @endcode
 */
RYPRTCTLER_API int _stdcall DEM_Initialize(char* szDllFile);

/**
 * @brief 更新X轴运动配置
 * 
 * @param nXMovAcc X轴加速时间（单位取决于具体硬件）
 * @param nIoOption IO控制选项
 * 
 * @note 修改X轴运动参数（速度、加速度等）后需要调用
 * @note 参数会立即生效
 * 
 * @example
 * @code
 * // 更新X轴配置
 * DEM_UpdateXMovCfg(g_movConfig.fxMovAcc, g_movConfig.fxIoOption);
 * @endcode
 */
RYPRTCTLER_API void _stdcall DEM_UpdateXMovCfg(int nXMovAcc, int nIoOption);

/**
 * @brief 更新Y轴运动配置
 * 
 * @param nYMovAcc Y轴加速时间
 * @param nIoOption IO控制选项
 * 
 * @note 修改Y轴运动参数后需要调用
 * 
 * @example
 * @code
 * // 更新Y轴配置
 * DEM_UpdateYMovCfg(g_movConfig.fyMovAcc, g_movConfig.fyIoOption);
 * @endcode
 */
RYPRTCTLER_API void _stdcall DEM_UpdateYMovCfg(int nYMovAcc, int nIoOption);

/**
 * @brief 启用运动控制
 * 
 * @note 必须在开始运动前调用
 * @note 通常在系统初始化时调用一次
 * 
 * @example
 * @code
 * DEM_Initialize(dllPath);
 * DEM_EnableRun();  // 启用运动
 * @endcode
 */
RYPRTCTLER_API void _stdcall DEM_EnableRun();

// ============================================================================
// 二、位置查询API
// ============================================================================

/**
 * @brief 获取当前Y轴位置
 * 
 * @param nAxis1Pos 输出Y轴1的位置（脉冲值）
 * @param nAxis2Pos 输出Y轴2的位置（脉冲值，如果是双Y轴系统）
 * 
 * @note 对于单Y轴系统，nAxis2Pos可能无效
 * @note 位置单位：脉冲数
 * 
 * @example
 * @code
 * int yPos1, yPos2;
 * DEM_GetYPosition(yPos1, yPos2);
 * printf("Y轴位置: %d 脉冲\n", yPos1);
 * 
 * // 转换为毫米
 * float yPosMM = DOT_TO_MM3(yPos1, g_movConfig.fySysdpi);
 * printf("Y轴位置: %.2f mm\n", yPosMM);
 * @endcode
 */
RYPRTCTLER_API void _stdcall DEM_GetYPosition(int& nAxis1Pos, int& nAxis2Pos);

/**
 * @brief 获取当前X轴位置
 * 
 * @return unsigned int 当前X轴位置（脉冲值）
 * 
 * @note 位置单位：脉冲数
 * @note 与编码器值相关
 * 
 * @example
 * @code
 * unsigned int xPos = DEM_GetXPosition();
 * printf("X轴位置: %u 脉冲\n", xPos);
 * 
 * // 转换为毫米
 * float xPosMM = DOT_TO_MM3(xPos, g_movConfig.fxSysdpi);
 * printf("X轴位置: %.2f mm\n", xPosMM);
 * @endcode
 */
RYPRTCTLER_API unsigned int _stdcall DEM_GetXPosition();

// ============================================================================
// 三、运动控制API（核心功能）
// ============================================================================

/**
 * @brief 控制X轴运动
 * 
 * @param bDir 运动方向（true:正向 false:负向）
 * @param nSpeed 运动速度（脉冲/秒）
 * @param nStep 运动距离（脉冲数）
 * @param bWait 是否等待运动完成（true:阻塞等待 false:立即返回）
 * @param fXMoveRate X轴速率调整系数（0.1-2.0，通常为1.0）
 * 
 * @return bool 操作结果：
 *         - true: 运动启动成功
 *         - false: 运动启动失败
 * 
 * @note 【打印中使用】PASS打印前的X轴定位和打印后的回退
 * @note 速度和距离使用MM_TO_DOT宏从毫米转换为脉冲
 * @note bWait=true时函数会阻塞到运动完成
 * 
 * @example
 * @code
 * // PASS打印前的X轴定位
 * int nMovBuf = MM_TO_DOT(g_movConfig.fMovBuf, g_movConfig.fxSysdpi);
 * int nDestPos;
 * 
 * if (pPassItem->bEncPrtDir) {
 *     // 正向打印
 *     nDestPos = pPassItem->nStartEncPos + ZERO_POSITION - nMovBuf;
 * } else {
 *     // 反向打印
 *     nDestPos = pPassItem->nStartEncPos + ZERO_POSITION + nMovBuf;
 * }
 * 
 * int currentPos = DEV_GetPrinterEncValue();
 * int distance = nDestPos - currentPos;
 * int speed = MM_TO_DOT(g_movConfig.fxMovSpd, g_movConfig.fxSysdpi);
 * 
 * // 执行X轴运动，等待完成
 * DEM_MoveX(FALSE, speed, distance, TRUE, g_movConfig.fxMovRate);
 * 
 * // PASS打印后的X轴回退
 * int nMoveLen = pPassItem->nXPrtColumns * 
 *                pPassItem->fPrtPrecession + nMovBuf * 2;
 * DEM_MoveX(!pPassItem->bEncPrtDir, speed, nMoveLen, 
 *           TRUE, g_movConfig.fxMovRate);
 * @endcode
 */
RYPRTCTLER_API bool _stdcall DEM_MoveX(
    bool bDir, 
    unsigned int nSpeed, 
    float nStep, 
    bool bWait, 
    float fXMoveRate
);

/**
 * @brief 控制Y轴运动
 * 
 * @param bDir 运动方向（true:正向 false:负向）
 * @param nSpeed 运动速度（脉冲/秒）
 * @param nStep 运动距离（脉冲数）
 * @param bWait 是否等待运动完成
 * @param bSynCtl 双Y轴同步控制（true:同步 false:独立）
 * @param fYMoveRate Y轴速率调整系数（0.1-2.0，通常为1.0）
 * 
 * @return bool 操作结果：
 *         - true: 运动启动成功
 *         - false: 运动启动失败
 * 
 * @note 【打印中使用】PASS之间的Y轴步进和第一个PASS前的定位
 * @note bSynCtl仅在双Y轴系统中有效
 * 
 * @example
 * @code
 * int nSpeed = MM_TO_DOT(g_movConfig.fyMovSpd, g_movConfig.fySysdpi) * 48;
 * 
 * if (passIndex == 0) {
 *     // 第一个PASS: Y轴定位到起始位置
 *     float yPos = pPassItem->fYMinJetImgPos;
 *     DEM_MoveY(TRUE, nSpeed, yPos, true, false, g_movConfig.fyMovRate);
 * } else {
 *     // 后续PASS: Y轴步进
 *     // nStpVector单位是um（微米），需要转换
 *     int nYStep = MM_TO_DOT(pPassItem->nStpVector / 1000.0, 
 *                            g_movConfig.fySysdpi) * 48;
 *     DEM_MoveY(TRUE, nSpeed, nYStep, true, false, g_movConfig.fyMovRate);
 * }
 * @endcode
 */
RYPRTCTLER_API bool _stdcall DEM_MoveY(
    bool bDir, 
    unsigned int nSpeed, 
    float nStep, 
    bool bWait, 
    bool bSynCtl, 
    float fYMoveRate
);

// ============================================================================
// 四、运动停止API
// ============================================================================

/**
 * @brief 停止X轴运动
 * 
 * @param bImmedStop 停止方式
 *                   - true: 立即停止（急停）
 *                   - false: 减速停止（平滑）
 * 
 * @note 急停可能对机械系统有冲击
 * @note 正常情况下建议使用减速停止
 */
RYPRTCTLER_API void _stdcall DEM_StopXMove(bool bImmedStop);

/**
 * @brief 停止Y轴运动
 * 
 * @param bImmedStop 停止方式
 *                   - true: 立即停止（急停）
 *                   - false: 减速停止（平滑）
 * 
 * @note 急停可能对机械系统有冲击
 */
RYPRTCTLER_API void _stdcall DEM_StopYMove(bool bImmedStop);

/**
 * @brief 停止所有轴运动（紧急停止）
 * 
 * @param bImmedStop 停止方式
 *                   - true: 立即停止所有轴
 *                   - false: 减速停止所有轴
 * 
 * @note 【紧急情况使用】用户按下急停按钮或发生错误时调用
 * 
 * @example
 * @code
 * // 紧急停止按钮处理
 * void OnEmergencyStop() {
 *     DEM_StopAll(TRUE);        // 立即停止所有运动
 *     IDP_StopPrintJob();       // 停止打印
 * }
 * @endcode
 */
RYPRTCTLER_API void _stdcall DEM_StopAll(bool bImmedStop);

// ============================================================================
// 五、运动状态查询API
// ============================================================================

/**
 * @brief 等待X轴停止运动
 * 
 * @return bool 等待结果：
 *         - true: X轴已停止
 *         - false: 超时或错误
 * 
 * @note 用于确保X轴完全停止后再执行其他操作
 * @note 函数会阻塞直到X轴停止或超时
 * 
 * @example
 * @code
 * DEM_MoveX(FALSE, speed, distance, FALSE, 1.0f);  // 异步启动
 * // ... 执行其他操作 ...
 * DEM_WaitUntilXStop();  // 等待X轴完全停止
 * @endcode
 */
RYPRTCTLER_API bool _stdcall DEM_WaitUntilXStop();

/**
 * @brief 等待Y轴停止运动
 * 
 * @return bool 等待结果：
 *         - true: Y轴已停止
 *         - false: 超时或错误
 * 
 * @note 函数会阻塞直到Y轴停止或超时
 */
RYPRTCTLER_API bool _stdcall DEM_WaitUntilYStop();

/**
 * @brief 判断X轴是否正在运动
 * 
 * @param pDir 输出运动方向（true:正向 false:负向）
 * @param pRPos 输出剩余距离（脉冲数）
 * 
 * @return bool X轴运动状态：
 *         - true: X轴正在运动
 *         - false: X轴已停止
 * 
 * @note 用于非阻塞方式检查运动状态
 * 
 * @example
 * @code
 * bool dir;
 * unsigned int remainDist;
 * 
 * if (DEM_XIsMoving(&dir, &remainDist)) {
 *     printf("X轴正在运动，剩余: %u 脉冲\n", remainDist);
 * } else {
 *     printf("X轴已停止\n");
 * }
 * @endcode
 */
RYPRTCTLER_API bool _stdcall DEM_XIsMoving(bool* pDir, unsigned int* pRPos);

/**
 * @brief 判断Y轴是否正在运动
 * 
 * @param pDir 输出运动方向
 * @param pRPos 输出剩余距离（脉冲数）
 * 
 * @return bool Y轴运动状态：
 *         - true: Y轴正在运动
 *         - false: Y轴已停止
 * 
 * @note 用于非阻塞方式检查运动状态
 */
RYPRTCTLER_API bool _stdcall DEM_YIsMoving(bool* pDir, unsigned int* pRPos);

// ============================================================================
// 六、限位和零位检测API
// ============================================================================

/**
 * @brief 获取X轴零位信号状态
 * 
 * @return bool 零位状态：
 *         - true: 在零位（原点）
 *         - false: 不在零位
 * 
 * @note 用于回零操作
 * @note 零位通常由光电开关或霍尔传感器检测
 * 
 * @example
 * @code
 * // 回零操作
 * DEM_MoveX(FALSE, speed, longDistance, FALSE, 1.0f);
 * while (!DEM_GetXZeroSig()) {
 *     Sleep(10);  // 等待到达零位
 * }
 * DEM_StopXMove(FALSE);
 * DEV_ResetPrinterEncValue(0);  // 重置编码器
 * @endcode
 */
RYPRTCTLER_API bool _stdcall DEM_GetXZeroSig();

/**
 * @brief 获取X轴正向限位状态
 * 
 * @param bXReverseLmt 是否为反向限位开关
 * 
 * @return bool 限位状态：
 *         - true: 触发限位（到达边界）
 *         - false: 未触发限位
 * 
 * @note 用于安全保护，防止超出运动范围
 * @warning 触发限位时应立即停止运动
 * 
 * @example
 * @code
 * // 在运动循环中检查限位
 * while (moving) {
 *     if (DEM_GetXPosLimt(FALSE)) {
 *         DEM_StopXMove(TRUE);  // 触发限位，立即停止
 *         printf("X轴触发正向限位！\n");
 *         break;
 *     }
 *     Sleep(10);
 * }
 * @endcode
 */
RYPRTCTLER_API bool _stdcall DEM_GetXPosLimt(BOOL bXReverseLmt);

/**
 * @brief 获取X轴负向限位状态
 * 
 * @param bXReverseLmt 是否为反向限位开关
 * 
 * @return bool 限位状态：
 *         - true: 触发限位
 *         - false: 未触发限位
 * 
 * @note 用于安全保护
 */
RYPRTCTLER_API bool _stdcall DEM_GetXNegLimt(BOOL bXReverseLmt);

/**
 * @brief 获取Y轴零位信号状态
 * 
 * @return bool 零位状态：
 *         - true: 在零位
 *         - false: 不在零位
 * 
 * @note 用于Y轴回零操作
 */
RYPRTCTLER_API bool _stdcall DEM_GetYZeroSig();

/**
 * @brief 获取Y轴正向限位状态
 * 
 * @param bYReverseLmt 是否为反向限位开关
 * 
 * @return bool 限位状态：
 *         - true: 触发限位
 *         - false: 未触发限位
 * 
 * @note 用于安全保护
 */
RYPRTCTLER_API bool _stdcall DEM_GetYPosLimt(BOOL bYReverseLmt);

/**
 * @brief 获取Y轴负向限位状态
 * 
 * @param bYReverseLmt 是否为反向限位开关
 * 
 * @return bool 限位状态：
 *         - true: 触发限位
 *         - false: 未触发限位
 * 
 * @note 用于安全保护
 */
RYPRTCTLER_API bool _stdcall DEM_GetYNegLimt(BOOL bYReverseLmt);

// ============================================================================
// 使用说明和注意事项
// ============================================================================

/**
 * @section usage_guide 使用指南
 * 
 * @subsection init_sequence 初始化顺序
 * 1. DEM_Initialize()     - 初始化运动控制系统
 * 2. DEM_UpdateXMovCfg()  - 配置X轴参数
 * 3. DEM_UpdateYMovCfg()  - 配置Y轴参数
 * 4. DEM_EnableRun()      - 启用运动控制
 * 
 * @subsection move_sequence 运动控制顺序
 * 1. 计算目标位置和距离（使用MM_TO_DOT宏）
 * 2. 检查限位状态
 * 3. 调用DEM_MoveX或DEM_MoveY
 * 4. 等待运动完成或监控运动状态
 * 
 * @subsection safety_notes 安全注意事项
 * - 运动前检查限位状态
 * - 运动中监控限位信号
 * - 触发限位时立即停止
 * - 设置合理的速度和加速度
 * - 建议先在低速下测试
 * 
 * @subsection coordinate_system 坐标系统
 * - 位置单位：脉冲（pulse）
 * - 速度单位：脉冲/秒
 * - 使用MM_TO_DOT宏将毫米转换为脉冲
 * - 使用DOT_TO_MM3函数将脉冲转换为毫米
 * - DPI (每英寸点数) 用于单位转换
 * 
 * @subsection typical_values 典型参数值
 * - X轴DPI: 600
 * - Y轴DPI: 600
 * - X轴速度: 10 mm/s
 * - Y轴速度: 10 mm/s
 * - 加速时间: 0.5 s
 * - 移动缓冲: 50 mm
 */

#endif // MOVECTL_H

