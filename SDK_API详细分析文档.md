# SDK API 详细分析文档

## 文档概述

本文档详细分析 Inc 文件夹中的所有SDK头文件，提供每个函数的功能说明、参数解释和使用场景。

**SDK版本**: 单卡网络版扫描系统  
**文档日期**: 2024年12月8日  
**API类型**: Windows DLL (stdcall calling convention)

---

## 目录

1. [设备管理API (RYPrtCtler.h)](#设备管理api)
2. [图像打印API (RYPrtCtler.h)](#图像打印api)
3. [喷头控制API (RYPrtCtler.h)](#喷头控制api)
4. [运动控制API (MoveCtl.h)](#运动控制api)
5. [数据结构定义 (ryprtapi.h)](#数据结构定义)
6. [设备类型定义 (dev_def.h)](#设备类型定义)

---

## 一、设备管理API

### 1.1 DEV_OpenDevice

```cpp
RYPRTCTLER_API int __stdcall DEV_OpenDevice(
    void* notifyhand,           // 通知句柄（可选，用于异步通知）
    unsigned char* szDllFilePath // DLL文件路径
);
```

**功能**: 打开并初始化打印设备

**返回值**:
- `1`: 设备打开成功
- `-2`: pcs文件加载失败
- `-3`: 打开卡连接失败
- `-4`: graycfg文件加载失败
- `-5`: 设备数量超过许可

**使用场景**: 应用程序启动时首次初始化设备

**注意事项**:
- 必须在所有其他API调用之前执行
- szDllFilePath指向配置文件所在目录
- pcs文件包含设备配置信息
- graycfg文件包含灰度配置

**示例**:
```cpp
int result = DEV_OpenDevice(NULL, "C:\\Config\\");
if (result == 1) {
    // 设备打开成功
} else {
    // 处理错误
}
```

---

### 1.2 DEV_InitDevice

```cpp
RYPRTCTLER_API int __stdcall DEV_InitDevice(
    unsigned int nXEncInitVal  // X轴编码器初始值
);
```

**功能**: 初始化设备，设置编码器初始位置

**参数**:
- `nXEncInitVal`: X轴编码器的初始位置值，通常设为0或特定起始位置

**返回值**:
- `>= 0`: 初始化成功
- `< 0`: 初始化失败，返回错误码

**使用场景**: 设备打开后，开始打印前的初始化

**注意事项**:
- 必须在DEV_OpenDevice成功后调用
- 编码器值用于精确的打印位置控制

---

### 1.3 DEV_DeviceIsConnected

```cpp
RYPRTCTLER_API bool __stdcall DEV_DeviceIsConnected();
```

**功能**: 检查设备是否连接

**返回值**:
- `true`: 设备已连接且正常
- `false`: 设备未连接或连接异常

**使用场景**: 
- 定期检查设备状态（MonitorThread）
- 打印前验证设备可用性
- 错误恢复时检测设备连接

**建议**: 
- 在MonitorThread中每100-200ms检查一次
- 不要在打印关键路径中频繁调用

---

### 1.4 DEV_CloseDevice

```cpp
RYPRTCTLER_API bool __stdcall DEV_CloseDevice();
```

**功能**: 关闭设备连接，释放资源

**返回值**:
- `true`: 关闭成功
- `false`: 关闭失败

**使用场景**: 应用程序退出时

**注意事项**:
- 关闭前应停止所有打印作业
- 释放所有图层资源
- 关闭所有运动控制

**示例**:
```cpp
// 停止打印
IDP_StopPrintJob();
// 释放资源
IDP_FreeImageLayer(-1);
// 关闭设备
DEV_CloseDevice();
```

---

### 1.5 DEV_UpdateParam

```cpp
RYPRTCTLER_API bool __stdcall DEV_UpdateParam(
    LPRYUSR_PARAM pParam  // 系统参数结构指针
);
```

**功能**: 更新设备系统参数

**参数**:
- `pParam`: 指向RYUSR_PARAM结构的指针，包含所有系统配置参数

**返回值**:
- `true`: 更新成功
- `false`: 更新失败

**使用场景**:
- 修改喷头偏移参数后
- 更改打印速度设置后
- 调整温度/电压参数后

**RYUSR_PARAM主要字段**:
```cpp
typedef struct tag_RYUSR_SYSPARAM {
    long nParamVer;                     // 参数版本
    unsigned int nPhgValidCtl[MAX_COLORS][MAX_GROUP];  // 喷头打印控制
    int nPhgYJetOffset[MAX_COLORS][MAX_GROUP];         // Y轴偏移
    int nPhgXGroupOff[MAX_COLORS][MAX_GROUP][2];       // X轴组偏移
    float fPrtScanSpd;                  // 打印扫描速度 mm/s
    PHCTL_PARAM phctl_param[MAX_PH_CNT]; // 喷头控制参数
    // ... 更多字段
} RYUSR_PARAM;
```

---

### 1.6 DEV_GetCurParam

```cpp
RYPRTCTLER_API bool __stdcall DEV_GetCurParam(
    LPRYUSR_PARAM pParam  // 接收参数的结构指针
);
```

**功能**: 获取当前设备参数

**参数**:
- `pParam`: 输出参数，填充当前系统参数

**返回值**:
- `true`: 获取成功
- `false`: 获取失败

**使用场景**: 读取当前配置以供修改或保存

---

### 1.7 DEV_GetDeviceInfo

```cpp
RYPRTCTLER_API LPPRINTER_INFO __stdcall DEV_GetDeviceInfo();
```

**功能**: 获取设备详细信息

**返回值**: 指向PRINTER_INFO结构的指针，包含设备完整状态信息

**PRINTER_INFO关键字段**:
```cpp
typedef struct tag_Printer_Info {
    unsigned int nVersion;              // 系统版本号
    unsigned int nCustomerID;           // 客户ID
    unsigned int nXSysEncDPI;           // X轴编码器DPI
    unsigned int nMcpCount;             // 主控制卡数量
    unsigned int nSysStatus;            // 系统状态
    unsigned int nPrintStatus;          // 打印状态 0:空闲 1:打印 2:暂停
    unsigned int nSysColors;            // 系统颜色数
    RYPHI_INFO clrph_info[MAX_COLORS];  // 每个颜色的喷头信息
    DRVINFO sysDrvInfo[MAX_MCP_CNT][MCP_DRV_CNT]; // 驱动卡状态
    PrtRunInfo prt_rtinfo;              // 打印运行信息
} PRINTER_INFO;
```

**使用场景**:
- 监控线程中定期获取设备状态
- 显示设备信息到UI
- 诊断设备问题

**注意**: 返回的指针指向SDK内部内存，不要释放

---

### 1.8 DEV_ReloadWaveForm

```cpp
RYPRTCTLER_API int __stdcall DEV_ReloadWaveForm(
    unsigned int nClrMask,      // 颜色掩码，指定要重载的颜色
    unsigned int nPHGMask,      // 喷头组掩码
    char* szWaveFile           // 波形文件路径
);
```

**功能**: 重新加载喷头波形文件

**参数**:
- `nClrMask`: 位掩码，bit0-bit15对应color0-color15
- `nPHGMask`: 喷头组掩码，指定哪些组需要重载
- `szWaveFile`: 波形文件完整路径

**返回值**:
- `>= 0`: 重载成功
- `< 0`: 重载失败，返回错误码

**使用场景**:
- 更换喷头后
- 优化打印质量
- 调整喷射波形

**示例**:
```cpp
// 重载颜色0的所有喷头组的波形
int result = DEV_ReloadWaveForm(0x1, 0xFF, "C:\\WaveForms\\color0.wav");
```

---

### 1.9 DEV_ResetPrinterEncValue

```cpp
RYPRTCTLER_API bool __stdcall DEV_ResetPrinterEncValue(
    unsigned int nPos  // 新的编码器位置值
);
```

**功能**: 重置打印机编码器位置值

**参数**:
- `nPos`: 设置的新位置值，通常为0

**返回值**:
- `true`: 重置成功
- `false`: 重置失败

**使用场景**:
- 回零操作后重置编码器
- 位置校准
- 开始新的打印作业前

---

### 1.10 DEV_SetVirtualPrtEncoder

```cpp
RYPRTCTLER_API bool __stdcall DEV_SetVirtualPrtEncoder(
    bool bEnable,              // 是否启用虚拟编码器
    bool bEncDir,              // 编码器方向
    unsigned int nClkFreq      // 时钟频率
);
```

**功能**: 设置虚拟打印编码器（用于测试）

**参数**:
- `bEnable`: true启用虚拟编码器，false使用实际编码器
- `bEncDir`: 编码器计数方向 true:正向 false:负向
- `nClkFreq`: 虚拟编码器时钟频率

**使用场景**: 无实际编码器时的测试和调试

---

### 1.11 DEV_GetPrinterEncValue

```cpp
RYPRTCTLER_API unsigned int __stdcall DEV_GetPrinterEncValue();
```

**功能**: 查询当前打印机编码器位置值

**返回值**: 当前编码器值（32位）

**使用场景**:
- 运动控制中计算距离
- 打印位置精确控制
- 位置监控和记录

---

## 二、图像打印API

### 2.1 IDP_SartPrintJob

```cpp
RYPRTCTLER_API int __stdcall IDP_SartPrintJob(
    LPPRTJOB_ITEM pJobItem  // 打印作业参数
);
```

**功能**: 启动打印作业（关键API！）

**参数**: 
- `pJobItem`: 指向PRTJOB_ITEM结构的指针

**PRTJOB_ITEM结构**:
```cpp
typedef struct tag_PrtJobItem {
    unsigned int nJobID;           // 作业ID
    char szJobName[64];            // 作业名称
    char szJobFilePath[260];       // 文件路径（CLI文件等）
    unsigned int nFileType;        // 文件类型 0:直接传输图像数据
    float fPrtXPos;                // 打印X轴起始位置 (mm)
    float fPrtYPos;                // 打印Y轴位置 (mm)
    float fClipWidth;              // 打印区域宽度 (mm)
    float fClipHeight;             // 打印区域高度 (mm)
    float fOutXdpi;                // 输出X DPI
    float fOutYdpi;                // 输出Y DPI
    unsigned int nOutPixelBits;    // 输出灰度位数
    unsigned int nPrtCtl;          // 打印控制标志
                                   // bit0: 白色跳过
                                   // bit1: 循环打印偏移
                                   // bit2: 每次随机偏移
                                   // bit4: X镜像
                                   // bit5: Y镜像
} PRTJOB_ITEM;
```

**返回值**:
- `>= 0`: 启动成功
- `< 0`: 启动失败，返回错误码

**使用场景**: 
- **双线程架构中的PrintThreadSeparate**
- 在写入图像数据之前调用
- 每个打印作业开始前调用一次

**调用顺序**:
```
1. IDP_SartPrintJob()      ← 启动作业
2. IDP_WriteImgLayerData() ← 写入图层数据（循环）
3. IDP_StartLayerPrint()   ← 启动图层打印（循环）
4. IDP_DoPassPrint()       ← 执行PASS打印（循环）
5. IDP_StopPrintJob()      ← 停止作业
```

**注意**: 
- 必须在写入任何图像数据之前调用
- 全局只能有一个活动作业
- 作业参数在整个打印过程中保持有效

---

### 2.2 IDP_WriteImgLayerData

```cpp
RYPRTCTLER_API int __stdcall IDP_WriteImgLayerData(
    LPPRTIMG_LAYER lpLayerInfo,   // 图层信息
    unsigned char* pSrcBuf[],      // 图像数据指针数组
    int nBytes                     // 每个颜色通道的字节数
);
```

**功能**: 将图像图层数据写入设备缓冲区（生产者）

**参数**:
- `lpLayerInfo`: 图层配置信息
- `pSrcBuf`: 图像数据指针数组，每个元素对应一个颜色通道
- `nBytes`: 每个颜色通道的数据字节数

**PRTIMG_LAYER关键字段**:
```cpp
typedef struct tag_ImgLayer {
    long nLayerIndex;          // 图层索引（必填）
    int nStartJetOffset;       // 起始喷嘴偏移
    int nPrtDir;               // 打印方向 0:负方向 1:正方向
    float fPrtXOffet;          // X轴偏移 (mm)
    float fPrtYOffet;          // Y轴偏移 (mm)
    int nColorCnts;            // 颜色通道数量（必填）
    float nXDPI;               // X轴DPI（必填）
    long nYDPI;                // Y轴DPI（必填）
    long nBytesPerLine;        // 每行字节数（必填）
    long nHeight;              // 图像高度（像素）（必填）
    long nWidth;               // 图像宽度（像素）（必填）
    long nGrayBits;            // 灰度位数 1:1bit, 2:2bits（必填）
    unsigned int nFeatherMode; // 羽化方式 0:无 1-6:Level1-6 7:自定义
    float fLayerDensity;       // 图层密度 >=1:100% 0-1:0-100%
    unsigned int nValidClrMask;// 有效颜色掩码（必填，通常0xFFFF）
    unsigned int nPrtFlag;     // 打印标志
    // ... 更多字段
} PRTIMG_LAYER;
```

**返回值**:
- `1`: 数据写入成功
- `0`: 已存在的图层
- `-110001`: 图层信息为空
- `-110002`: 图层编号不在有效范围
- `-110004`: PC内存不足
- `-110005`: 图像预处理错误

**使用场景**: 
- **双线程架构中的PrintThreadSeparate/DataWriterThread**
- 在PrtBuffNum < 5时写入
- 循环写入所有图层数据

**示例**:
```cpp
// 准备图层信息
g_PrtImgLayer.nLayerIndex = 0;
g_PrtImgLayer.nColorCnts = 1;
g_PrtImgLayer.nXDPI = 720;
g_PrtImgLayer.nYDPI = 600;
g_PrtImgLayer.nWidth = 1000;
g_PrtImgLayer.nHeight = 1000;
g_PrtImgLayer.nBytesPerLine = 128;
g_PrtImgLayer.nGrayBits = 1;
g_PrtImgLayer.nValidClrMask = 0xFFFF;

// 准备图像数据
LPBYTE pBmpFile[MAX_COLORS];
for (int c = 0; c < g_PrtImgLayer.nColorCnts; c++) {
    pBmpFile[c] = imageData[c];
}

// 写入数据
int result = IDP_WriteImgLayerData(&g_PrtImgLayer, pBmpFile, dataSize);
if (result == 1) {
    PrtBuffNum++;  // 缓冲区计数增加
}
```

**注意事项**:
- **必须在互斥锁保护下调用**
- 图像数据必须是单色位图格式
- nLayerIndex从0开始连续编号
- 写入后PrtBuffNum应该递增
- 最多可缓存5个图层

---

### 2.3 IDP_WriteImgLayerFile

```cpp
RYPRTCTLER_API int __stdcall IDP_WriteImgLayerFile(
    LPPRTIMG_LAYER lpLayerInfo,  // 图层信息
    char* szFile                  // 图像文件路径
);
```

**功能**: 从文件写入图层数据

**参数**:
- `lpLayerInfo`: 图层配置信息
- `szFile`: 图像文件路径（支持BMP等格式）

**返回值**: 同IDP_WriteImgLayerData

**使用场景**: 直接从文件加载图像数据时使用

---

### 2.4 IDP_StartLayerPrint

```cpp
RYPRTCTLER_API int __stdcall IDP_StartLayerPrint(
    int nLayerIndex  // 图层索引
);
```

**功能**: 启动指定图层的打印处理（关键API！）

**参数**:
- `nLayerIndex`: 要打印的图层索引

**返回值**:
- `> 0`: 成功，返回PASS数量
- `-1`: 图像数据未准备好（需要重试）
- `-120000 ~ -120016`: 启动失败错误码
- `-121000`: 板卡内存分配失败
- `-122000`: 创建数据发送线程失败

**使用场景**:
- **双线程架构中的PrintThread/PrintExecutorThread**
- 在PrtBuffNum > 0时调用
- 必须在IDP_WriteImgLayerData之后调用

**典型用法**:
```cpp
// 等待数据就绪
while (PrtBuffNum <= 0) {
    Sleep(10);
}

// 获取互斥锁
WaitForSingleObject(g_PrtMutex, INFINITE);

// 启动图层打印
int nPassCount = IDP_StartLayerPrint(layerIndex);

// 重试机制（如果数据未就绪）
while (nPassCount == -1) {
    nPassCount = IDP_StartLayerPrint(layerIndex);
}

if (nPassCount > 0) {
    // 处理每个PASS
    for (int i = 0; i < nPassCount; i++) {
        // PASS打印逻辑
    }
}

// 释放互斥锁
ReleaseMutex(g_PrtMutex);
```

**注意**: 
- 返回-1表示数据未就绪，应该重试
- 返回的PASS数量用于后续的PASS循环

---

### 2.5 IDP_GetPassItem

```cpp
RYPRTCTLER_API LPPassDataItem __stdcall IDP_GetPassItem(
    unsigned int nLayerIndex,  // 图层索引
    int nPassID                // PASS索引
);
```

**功能**: 获取指定PASS的数据项（关键API！）

**参数**:
- `nLayerIndex`: 图层索引
- `nPassID`: PASS索引（0到nPassCount-1）

**返回值**: 指向PassDataItem结构的指针

**PassDataItem关键字段**:
```cpp
typedef struct tag_PassDataItem {
    unsigned int nLayerIndex;      // 所属图层
    unsigned int nLayerPassCount;  // 图层PASS总数
    unsigned int nLayerPassIndex;  // 当前PASS索引
    unsigned int nValidPassJets;   // PASS有效喷嘴数
    unsigned int nProcState;       // 处理状态
                                   // 0:数据处理阶段
                                   // 1:源数据处理完成
                                   // 2:格式数据转换完成
                                   // 3:数据等待写入硬件
                                   // 4:数据添加到打印队列
                                   // 5:打印已结束
                                   // 6:打印内存已释放
                                   // 7:ERROR
    float fYMinJetImgPos;          // Y轴偏移（最小喷嘴到Y定位的距离）mm
    bool bEncPrtDir;               // 打印方向 0:负 1:正
    bool bScanPrtDir;              // 喷头扫描方向
    bool bIsNullPass;              // 空PASS标志
    int nStpVector;                // 打印前需要的步进量(um) >0:Y正向 <0:Y负向
    int nValidStartCols;           // X轴有效起始列
    int nValidEndCols;             // X轴有效结束列
    int nImgColumns;               // 图像有效列数
    int nXPrtColumns;              // 实际打印列数
    unsigned int nStartEncPos;     // 起始编码器位置
    float fPrtPrecession;          // 打印精度值
    // ... 更多字段
} PassDataItem;
```

**使用场景**:
- 在PASS循环中获取每个PASS的详细信息
- 根据nProcState判断PASS是否就绪

**典型用法**:
```cpp
LPPassDataItem pPassItem = nullptr;

// 等待PASS就绪
do {
    pPassItem = IDP_GetPassItem(layerIndex, passIndex);
    if (pPassItem) {
        // 状态3,4,5都可以执行打印
        if ((pPassItem->nProcState > 2) && (pPassItem->nProcState < 6)) {
            break;
        }
    }
    // MFC中这里没有Sleep，紧密循环保证快速响应
} while (1);

// 检查是否为空PASS
if (pPassItem->bIsNullPass) {
    continue;  // 跳过空PASS
}

// 使用PASS数据进行运动控制和打印
```

**注意**: 
- **必须循环等待nProcState进入就绪状态（3-5）**
- 返回的指针指向SDK内部内存
- bIsNullPass为true时应跳过该PASS

---

### 2.6 IDP_DoPassPrint

```cpp
RYPRTCTLER_API int __stdcall IDP_DoPassPrint(
    LPPassDataItem pPassItem  // PASS数据项
);
```

**功能**: 执行指定PASS的打印（关键API！）

**参数**:
- `pPassItem`: 从IDP_GetPassItem获取的PASS数据项

**返回值**:
- `> 0`: 打印成功
- `<= 0`: 打印失败

**使用场景**:
- 在PASS就绪后调用
- 在运动控制完成后调用

**典型流程**:
```cpp
// 1. 获取PASS数据
LPPassDataItem pPassItem = IDP_GetPassItem(layerIndex, passIndex);

// 2. Y轴运动控制（如需要）
#ifdef RY_MOVE_CTL
if (passIndex == 0) {
    // 第一个PASS: Y轴定位
} else {
    // 后续PASS: Y轴步进
    int nYStep = MM_TO_DOT(pPassItem->nStpVector/1000.0, ...);
    DEM_MoveY(TRUE, nSpeed, nYStep, true, false, ...);
}

// 3. X轴运动控制
if (pPassItem->bEncPrtDir) {
    nDestPos = pPassItem->nStartEncPos + ZERO_POSITION - nMovBuf;
} else {
    nDestPos = pPassItem->nStartEncPos + ZERO_POSITION + nMovBuf;
}
DEM_MoveX(FALSE, nSpeed, ndistance, TRUE, ...);
#endif

// 4. 执行打印
int result = IDP_DoPassPrint(pPassItem);
if (result > 0) {
    // 打印成功
    
    // 5. X轴回退
    #ifdef RY_MOVE_CTL
    int nMoveLen = pPassItem->nXPrtColumns * pPassItem->fPrtPrecession + nMovBuf * 2;
    DEM_MoveX(!pPassItem->bEncPrtDir, nSpeed, nMoveLen, TRUE, ...);
    #endif
}
```

**注意**:
- 必须在运动控制后调用
- 打印期间硬件会自动控制喷头喷射
- 通常需要监控打印状态直到完成

---

### 2.7 IDP_GetPrintState

```cpp
RYPRTCTLER_API bool __stdcall IDP_GetPrintState(
    LPPrtRunInfo pRTinfo  // 接收运行信息的结构
);
```

**功能**: 获取打印运行状态

**参数**:
- `pRTinfo`: 输出参数，填充打印运行信息

**PrtRunInfo关键字段**:
```cpp
typedef struct tag_PrtRunInfo {
    bool bJobPrtRuning;            // 作业是否运行中
    bool bLayerPrtIsOver;          // 当前图层是否打印完成
    int nPrtState;                 // 打印状态
                                   // 0: IDLE(空闲)
                                   // 1: Print(打印中)
                                   // 2: Pause(暂停)
                                   // 3: Flash(冲洗)
    int nPrintLayerIndex;          // 当前打印图层索引
    int nLayerPassCount;           // 当前图层PASS数
    int nPrintPassIndex;           // 当前打印PASS索引
    int nCurPrtDir;                // 当前打印方向
    unsigned int nPrtEncoderValue; // 打印编码器值
    unsigned int nRevPrtCols;      // 剩余打印列数
    // ... 更多字段
} PrtRunInfo;
```

**返回值**:
- `true`: 获取成功
- `false`: 获取失败

**使用场景**:
- 监控打印进度
- 等待打印完成
- 诊断打印问题

**典型用法**:
```cpp
PrtRunInfo prtInfo;

// 监控打印状态，等待打印完成
do {
    IDP_GetPrintState(&prtInfo);
    if (prtInfo.nPrtState == 1) {
        // 正在打印，继续等待
        Sleep(10);
    } else {
        // 打印完成或其他状态
        break;
    }
} while (true);
```

**注意**: 
- nPrtState == 1 表示正在打印
- 可用于实时监控和UI更新

---

### 2.8 IDP_FreeImageLayer

```cpp
RYPRTCTLER_API int __stdcall IDP_FreeImageLayer(
    int nLayerIndex  // 图层索引，-1表示释放所有
);
```

**功能**: 释放图像图层资源

**参数**:
- `nLayerIndex`: 
  - `>= 0`: 释放指定图层
  - `-1`: 释放所有图层

**返回值**:
- `>= 0`: 释放成功
- `< 0`: 释放失败

**使用场景**:
- 每个图层打印完成后释放
- 打印作业结束时释放所有图层
- 错误处理时清理资源

**典型用法**:
```cpp
// 打印完一个图层后
IDP_FreeImageLayer(layerIndex);
PrtBuffNum--;  // 缓冲区计数减少

// 打印作业结束时
IDP_FreeImageLayer(-1);  // 释放所有
```

**注意**:
- **必须在图层打印完成后调用**
- 释放后PrtBuffNum应该递减
- 释放所有图层(-1)用于清理和错误恢复

---

### 2.9 IDP_StopPrintJob

```cpp
RYPRTCTLER_API bool __stdcall IDP_StopPrintJob();
```

**功能**: 停止当前打印作业

**返回值**:
- `true`: 停止成功
- `false`: 停止失败

**使用场景**:
- 用户点击停止按钮
- 打印完成后清理
- 错误发生时终止打印

**典型用法**:
```cpp
// 正常完成
IDP_StopPrintJob();
IDP_FreeImageLayer(-1);

// 用户中断
if (userClickedStop) {
    IDP_StopPrintJob();
    IDP_FreeImageLayer(-1);
}
```

**注意**: 
- 应该在释放图层前调用
- 调用后作业状态会重置

---

### 2.10 IDP_StartCalibration

```cpp
RYPRTCTLER_API int __stdcall IDP_StartCalibration(
    LPRYCalbrationParam pParam  // 校准参数
);
```

**功能**: 启动校准打印

**参数**:
- `pParam`: 校准参数结构

**RYCalbrationParam结构**:
```cpp
typedef struct tag_CalbrationParam {
    unsigned int nAdjType;      // 校准类型
                                // 0:打印状态图
                                // 1:垂直校准
                                // 2:角度
                                // 3:X双向颜色
                                // 4:X距离颜色
                                // 5:Y距离颜色
    unsigned int nCtrlValue;    // 校准控制值
    float fPrtXPos;             // 打印X位置
    float fPrtYPos;             // 打印Y位置
    float fxadjdpi;             // 校准打印DPI
    int nGrayBits;              // 灰度位数
    int nPrtDir;                // 打印方向 0:双向 1:单向
    float fStpSize;             // 步进距离(mm)
    float fXRunSpd;             // 喷头移动速度(mm/s)
    float fXMaxPrtWidth;        // 最大打印宽度
    float fYMaxPrtHeight;       // 最大打印高度
} RYCalbrationParam;
```

**使用场景**: 设备校准和测试

---

### 2.11 IDP_FlashPrtCtl

```cpp
RYPRTCTLER_API bool __stdcall IDP_FlashPrtCtl(
    bool bOpen  // true:开启冲洗 false:关闭冲洗
);
```

**功能**: 控制喷头冲洗

**参数**:
- `bOpen`: 冲洗开关

**返回值**:
- `true`: 操作成功
- `false`: 操作失败

**使用场景**: 
- 长时间不打印前冲洗喷头
- 保持喷嘴湿润
- 防止堵塞

---

### 2.12 IDP_SetCaliDataCallBack

```cpp
RYPRTCTLER_API void __stdcall IDP_SetCaliDataCallBack(
    LPRY_CALI_PHGDCall pFun  // 回调函数指针
);
```

**功能**: 设置校准数据回调函数

**回调函数定义**:
```cpp
typedef int (*LPRY_CALI_PHGDCall)(
    unsigned char* pPhgData,  // 喷头组数据
    int c,                    // 颜色索引
    int g,                    // 组索引
    int nWidth,               // 宽度
    int Height,               // 高度
    int nByteLines,           // 每行字节数
    int nGrayBits             // 灰度位数
);
```

**使用场景**: 获取校准打印生成的图像数据进行分析

---

## 三、喷头控制API

### 3.1 MVT_UpdatePhVoltage

```cpp
RYPRTCTLER_API bool __stdcall MVT_UpdatePhVoltage(
    float* fstdVcom,      // 标准电压数组
    unsigned int nPhID    // 喷头ID
);
```

**功能**: 更新喷头电压

**参数**:
- `fstdVcom`: 电压值数组（最多32个值）
- `nPhID`: 喷头ID

**返回值**:
- `true`: 更新成功
- `false`: 更新失败

**使用场景**: 调整喷头电压以优化打印质量

---

### 3.2 MVT_SetPhStdTemp

```cpp
RYPRTCTLER_API bool __stdcall MVT_SetPhStdTemp(
    float* fstdTmp,       // 标准温度数组
    unsigned int nPhID    // 喷头ID
);
```

**功能**: 设置喷头标准温度

**参数**:
- `fstdTmp`: 温度值数组（最多4个值）
- `nPhID`: 喷头ID

**返回值**:
- `true`: 设置成功
- `false`: 设置失败

**使用场景**: 温度控制，保证打印质量稳定性

---

### 3.3 MVT_CurPhVoltage

```cpp
RYPRTCTLER_API bool __stdcall MVT_CurPhVoltage(
    float* fCurVcom,      // 接收当前电压的数组
    unsigned int nPhID    // 喷头ID
);
```

**功能**: 获取喷头当前电压

**参数**:
- `fCurVcom`: 输出参数，接收当前电压值
- `nPhID`: 喷头ID

**返回值**:
- `true`: 获取成功
- `false`: 获取失败

**使用场景**: 监控喷头电压状态

---

### 3.4 MVT_GetCurPhTemp

```cpp
RYPRTCTLER_API bool __stdcall MVT_GetCurPhTemp(
    float* fCurTemp,      // 接收当前温度的数组
    unsigned int nPhID    // 喷头ID
);
```

**功能**: 获取喷头当前温度

**参数**:
- `fCurTemp`: 输出参数，接收当前温度值
- `nPhID`: 喷头ID

**返回值**:
- `true`: 获取成功
- `false`: 获取失败

**使用场景**: 
- 温度监控
- 显示到UI
- 过热保护

---

### 3.5 MVT_SetAirPressCtlVal

```cpp
RYPRTCTLER_API bool __stdcall MVT_SetAirPressCtlVal(
    unsigned int nPrtIndex  // 打印控制器索引
);
```

**功能**: 设置气压控制值

**参数**:
- `nPrtIndex`: 打印控制器索引

**返回值**:
- `true`: 设置成功
- `false`: 设置失败

**使用场景**: 气压辅助喷射系统

---

### 3.6 MVT_SetOutPut

```cpp
RYPRTCTLER_API bool __stdcall MVT_SetOutPut(
    int nPrtIndex,     // 打印控制器索引
    int nOutMask,      // 输出掩码
    bool bEnable       // 启用/禁用
);
```

**功能**: 手动控制输出

**参数**:
- `nPrtIndex`: 打印控制器索引
- `nOutMask`: 输出通道掩码
- `bEnable`: true启用，false禁用

**使用场景**: 手动控制外设（UV灯、风扇等）

---

### 3.7 MVT_SetOutPutConfig

```cpp
RYPRTCTLER_API bool __stdcall MVT_SetOutPutConfig(
    int nPrtIndex  // 打印控制器索引
);
```

**功能**: 设置输出配置

**参数**:
- `nPrtIndex`: 打印控制器索引

**返回值**:
- `true`: 设置成功
- `false`: 设置失败

---

### 3.8 DEV_AdibControl

```cpp
RYPRTCTLER_API bool __stdcall DEV_AdibControl(
    LPADIB_PARAM lParam,      // ADIB参数
    unsigned int nOption,     // 选项
    bool bSetParam,           // true:设置 false:获取
    bool* bAbort,             // 中止标志
    unsigned int nPrtIndex    // 打印控制器索引
);
```

**功能**: ADIB板卡控制（墨水管理）

**参数**:
- `lParam`: ADIB参数结构
- `nOption`: 操作选项
- `bSetParam`: true设置参数，false获取参数
- `bAbort`: 中止标志
- `nPrtIndex`: 打印控制器索引

**ADIB_PARAM结构**:
```cpp
typedef struct tag_ADIB_PARAM {
    UINT nFMver;                // 固件版本号
    float fcurvoltage[8];       // 当前电压
    float fcurInkTankTemp[8];   // 当前墨水温度
    float fcurAirPress[8];      // 当前气压
    UINT nLgStatus;             // 液位状态
    UINT nVolState;             // 电压状态
    struct I2C_DATA {
        float fvoltage[8];
        float fInkTankTemp[8];
        float fAirPress[8];
    } I2C;
    float fAirThreshold;        // 气压阈值
    float fPressInkTime;        // 加压供墨时间
    UINT nStatus;               // 板卡状态
    float fInkSupplyTime;       // 供墨时间
} ADIB_PARAM;
```

**使用场景**: 墨水供应系统管理

---

### 3.9 DEV_EnableInkWatch

```cpp
RYPRTCTLER_API bool __stdcall DEV_EnableInkWatch(
    bool bOpen,                // 开启/关闭
    unsigned int nPrtIndex,    // 打印控制器索引
    unsigned int nDrvIndex     // 驱动器索引
);
```

**功能**: 启用墨水观察（检测喷射状态）

**参数**:
- `bOpen`: true开启，false关闭
- `nPrtIndex`: 打印控制器索引
- `nDrvIndex`: 驱动器索引

**使用场景**: 喷射状态监测和缺喷检测

---

### 3.10 DEV_GenAllRegReport

```cpp
RYPRTCTLER_API void __stdcall DEV_GenAllRegReport(
    const char* szFilePath  // 报告文件路径
);
```

**功能**: 生成所有寄存器报告

**参数**:
- `szFilePath`: 报告保存路径

**使用场景**: 系统诊断和故障排查

---

### 3.11 DEV_UpdateAppFile

```cpp
RYPRTCTLER_API int __stdcall DEV_UpdateAppFile(
    const char* szFilePath,     // APP文件路径
    unsigned int nPrtIndex      // 打印控制器索引
);
```

**功能**: 更新设备应用程序固件

**参数**:
- `szFilePath`: 固件文件路径
- `nPrtIndex`: 打印控制器索引

**返回值**:
- `>= 0`: 更新成功
- `< 0`: 更新失败

**使用场景**: 固件升级

---

### 3.12 DEV_UpdateFpgaFile

```cpp
RYPRTCTLER_API int __stdcall DEV_UpdateFpgaFile(
    const char* szFilePath,     // FPGA文件路径
    unsigned int nPrtIndex      // 打印控制器索引
);
```

**功能**: 更新FPGA固件

**参数**:
- `szFilePath`: FPGA文件路径
- `nPrtIndex`: 打印控制器索引

**返回值**:
- `>= 0`: 更新成功
- `< 0`: 更新失败

**使用场景**: FPGA固件升级

---

## 四、运动控制API

### 4.1 DEM_Initialize

```cpp
RYPRTCTLER_API int _stdcall DEM_Initialize(
    char* szDllFile  // 运动控制DLL文件路径
);
```

**功能**: 初始化运动控制系统

**参数**:
- `szDllFile`: 运动控制DLL路径

**返回值**:
- `>= 0`: 初始化成功
- `< 0`: 初始化失败

**使用场景**: 在DEV_OpenDevice之后调用

---

### 4.2 DEM_UpdateXMovCfg

```cpp
RYPRTCTLER_API void _stdcall DEM_UpdateXMovCfg(
    int nXMovAcc,     // X轴加速度
    int nIoOption     // IO选项
);
```

**功能**: 更新X轴运动配置

**参数**:
- `nXMovAcc`: X轴加速时间
- `nIoOption`: IO控制选项

**使用场景**: 修改X轴运动参数后

---

### 4.3 DEM_UpdateYMovCfg

```cpp
RYPRTCTLER_API void _stdcall DEM_UpdateYMovCfg(
    int nYMovAcc,     // Y轴加速度
    int nIoOption     // IO选项
);
```

**功能**: 更新Y轴运动配置

**参数**:
- `nYMovAcc`: Y轴加速时间
- `nIoOption`: IO控制选项

**使用场景**: 修改Y轴运动参数后

---

### 4.4 DEM_EnableRun

```cpp
RYPRTCTLER_API void _stdcall DEM_EnableRun();
```

**功能**: 启用运动控制

**使用场景**: 在开始运动前调用

---

### 4.5 DEM_MoveX

```cpp
RYPRTCTLER_API bool _stdcall DEM_MoveX(
    bool bDir,              // 运动方向 true:正向 false:负向
    unsigned int nSpeed,    // 速度（脉冲/秒）
    float nStep,            // 步进距离（脉冲数）
    bool bWait,             // 是否等待运动完成
    float fXMoveRate        // X轴速率系数
);
```

**功能**: 控制X轴运动

**参数**:
- `bDir`: 运动方向
- `nSpeed`: 运动速度
- `nStep`: 运动距离（脉冲）
- `bWait`: true等待完成，false立即返回
- `fXMoveRate`: 速率调整系数

**返回值**:
- `true`: 运动启动成功
- `false`: 运动启动失败

**使用场景**:
- PASS打印前的X轴定位
- PASS打印后的X轴回退

**典型用法**:
```cpp
// 计算目标位置
int nMovBuf = MM_TO_DOT(g_movConfig.fMovBuf, g_movConfig.fxSysdpi);
int nDestPos;
int ndistance;

if (pPassItem->bEncPrtDir) {
    // 正向打印
    nDestPos = pPassItem->nStartEncPos + ZERO_POSITION - nMovBuf;
    ndistance = nDestPos - DEV_GetPrinterEncValue();
} else {
    // 反向打印
    nDestPos = pPassItem->nStartEncPos + ZERO_POSITION + nMovBuf;
    ndistance = nDestPos - DEV_GetPrinterEncValue();
}

// 执行X轴运动
int nXMoveSpd = MM_TO_DOT(g_movConfig.fxMovSpd, g_movConfig.fxSysdpi);
DEM_MoveX(FALSE, nXMoveSpd, ndistance, TRUE, g_movConfig.fxMovRate);
```

---

### 4.6 DEM_MoveY

```cpp
RYPRTCTLER_API bool _stdcall DEM_MoveY(
    bool bDir,              // 运动方向
    unsigned int nSpeed,    // 速度
    float nStep,            // 步进距离
    bool bWait,             // 是否等待完成
    bool bSynCtl,           // 同步控制
    float fYMoveRate        // Y轴速率系数
);
```

**功能**: 控制Y轴运动

**参数**:
- `bDir`: 运动方向
- `nSpeed`: 运动速度
- `nStep`: 运动距离
- `bWait`: 是否等待完成
- `bSynCtl`: 双Y轴同步控制
- `fYMoveRate`: 速率调整系数

**返回值**:
- `true`: 运动启动成功
- `false`: 运动启动失败

**使用场景**:
- PASS之间的Y轴步进
- 第一个PASS前的Y轴定位

**典型用法**:
```cpp
int nYMoveSpd = MM_TO_DOT(g_movConfig.fyMovSpd, g_movConfig.fySysdpi);

if (passIndex == 0) {
    // 第一个PASS: Y轴定位
    DEM_MoveY(TRUE, nYMoveSpd, pPassItem->fYMinJetImgPos, 
              true, false, g_movConfig.fyMovRate);
} else {
    // 后续PASS: Y轴步进
    int nYStep = MM_TO_DOT(pPassItem->nStpVector / 1000.0, 
                            g_movConfig.fySysdpi) * 48;
    DEM_MoveY(TRUE, nYMoveSpd, nYStep, 
              true, false, g_movConfig.fyMovRate);
}
```

---

### 4.7 DEM_GetXPosition

```cpp
RYPRTCTLER_API unsigned int _stdcall DEM_GetXPosition();
```

**功能**: 获取当前X轴位置

**返回值**: X轴当前位置（脉冲值）

---

### 4.8 DEM_GetYPosition

```cpp
RYPRTCTLER_API void _stdcall DEM_GetYPosition(
    int& nAxis1Pos,  // Y轴1位置
    int& nAxis2Pos   // Y轴2位置
);
```

**功能**: 获取当前Y轴位置

**参数**:
- `nAxis1Pos`: 输出Y轴1位置
- `nAxis2Pos`: 输出Y轴2位置（如果是双Y轴系统）

---

### 4.9 DEM_StopXMove

```cpp
RYPRTCTLER_API void _stdcall DEM_StopXMove(
    bool bImmedStop  // true:立即停止 false:减速停止
);
```

**功能**: 停止X轴运动

**参数**:
- `bImmedStop`: 停止方式

---

### 4.10 DEM_StopYMove

```cpp
RYPRTCTLER_API void _stdcall DEM_StopYMove(
    bool bImmedStop  // true:立即停止 false:减速停止
);
```

**功能**: 停止Y轴运动

**参数**:
- `bImmedStop`: 停止方式

---

### 4.11 DEM_StopAll

```cpp
RYPRTCTLER_API void _stdcall DEM_StopAll(
    bool bImmedStop  // true:立即停止 false:减速停止
);
```

**功能**: 停止所有轴运动

**参数**:
- `bImmedStop`: 停止方式

**使用场景**: 紧急停止

---

### 4.12 DEM_WaitUntilXStop

```cpp
RYPRTCTLER_API bool _stdcall DEM_WaitUntilXStop();
```

**功能**: 等待X轴停止运动

**返回值**:
- `true`: X轴已停止
- `false`: 超时或错误

---

### 4.13 DEM_WaitUntilYStop

```cpp
RYPRTCTLER_API bool _stdcall DEM_WaitUntilYStop();
```

**功能**: 等待Y轴停止运动

**返回值**:
- `true`: Y轴已停止
- `false`: 超时或错误

---

### 4.14 DEM_XIsMoving

```cpp
RYPRTCTLER_API bool _stdcall DEM_XIsMoving(
    bool* pDir,          // 输出运动方向
    unsigned int* pRPos  // 输出剩余位置
);
```

**功能**: 判断X轴是否正在运动

**参数**:
- `pDir`: 输出运动方向
- `pRPos`: 输出剩余距离

**返回值**:
- `true`: X轴正在运动
- `false`: X轴已停止

---

### 4.15 DEM_YIsMoving

```cpp
RYPRTCTLER_API bool _stdcall DEM_YIsMoving(
    bool* pDir,          // 输出运动方向
    unsigned int* pRPos  // 输出剩余位置
);
```

**功能**: 判断Y轴是否正在运动

**参数**:
- `pDir`: 输出运动方向
- `pRPos`: 输出剩余距离

**返回值**:
- `true`: Y轴正在运动
- `false`: Y轴已停止

---

### 4.16 DEM_GetXZeroSig

```cpp
RYPRTCTLER_API bool _stdcall DEM_GetXZeroSig();
```

**功能**: 获取X轴零位信号状态

**返回值**:
- `true`: 在零位
- `false`: 不在零位

---

### 4.17 DEM_GetXPosLimt

```cpp
RYPRTCTLER_API bool _stdcall DEM_GetXPosLimt(
    BOOL bXReverseLmt  // 是否反向限位
);
```

**功能**: 获取X轴正限位状态

**参数**:
- `bXReverseLmt`: 反向限位开关

**返回值**:
- `true`: 触发限位
- `false`: 未触发

---

### 4.18 DEM_GetXNegLimt

```cpp
RYPRTCTLER_API bool _stdcall DEM_GetXNegLimt(
    BOOL bXReverseLmt  // 是否反向限位
);
```

**功能**: 获取X轴负限位状态

---

### 4.19 DEM_GetYZeroSig

```cpp
RYPRTCTLER_API bool _stdcall DEM_GetYZeroSig();
```

**功能**: 获取Y轴零位信号状态

---

### 4.20 DEM_GetYPosLimt

```cpp
RYPRTCTLER_API bool _stdcall DEM_GetYPosLimt(
    BOOL bYReverseLmt  // 是否反向限位
);
```

**功能**: 获取Y轴正限位状态

---

### 4.21 DEM_GetYNegLimt

```cpp
RYPRTCTLER_API bool _stdcall DEM_GetYNegLimt(
    BOOL bYReverseLmt  // 是否反向限位
);
```

**功能**: 获取Y轴负限位状态

---

## 五、数据结构定义

### 5.1 常量定义

```cpp
#define MAX_MCP_CNT         8    // 最大主控卡数量
#define MAX_PRTCTL_CNT      8    // 最大打印控制器数量
#define MAX_COLORS         16    // 最大颜色通道数
#define MAX_GROUP          16    // 最大喷头组数
#define MAX_JETROW         32    // 最大喷嘴行数
#define MAX_DRVCHCNT       32    // 最大驱动通道数
#define MAX_PH_CNT        256    // 最大喷头数
```

### 5.2 宏定义

```cpp
// 毫米转换为点（脉冲）
#define MM_TO_DOT(X,DPI) (int)(((float)(X*DPI))/25.4+0.45f)

// 点（脉冲）转换为毫米
static float DOT_TO_MM3(int X, float DPI) {
    return ((float)X * 25.4f) / DPI;
}
```

**使用示例**:
```cpp
// 将10mm转换为600DPI下的脉冲数
int pulses = MM_TO_DOT(10.0, 600);  // 约236脉冲

// 将236脉冲转换为毫米
float mm = DOT_TO_MM3(236, 600);    // 约10mm
```

---

## 六、设备类型定义

### 6.1 支持的喷头类型

```cpp
// 常见喷头类型
#define PT_X1001              8   // XAAR 1001喷头
#define PT_X2001             19   // XAAR 2001喷头
#define PT_RICOH_G5          10   // 理光G5喷头
#define PT_RICOH_G6          41   // 理光G6喷头
#define PT_KM1024I           13   // 柯尼卡1024i喷头
#define PT_KM1024A           24   // 柯尼卡1024A喷头
#define PT_EPSON_S3200       28   // 爱普生S3200喷头
#define PT_EPSON_I3200       29   // 爱普生I3200喷头
#define PT_EPSON_I1600       37   // 爱普生I1600喷头
// ... 更多喷头类型
```

### 6.2 支持的驱动卡类型

```cpp
#define HW_DRV_G5_1H          1   // G5 1头驱动卡
#define HW_DRV_X2001_1H       7   // X2001 1头驱动卡
#define HW_DRV_3200_1H       15   // 3200 1头驱动卡
#define HW_DRV_4720_1H       16   // 4720 1头驱动卡
// ... 更多驱动卡类型
```

---

## 七、完整的打印流程API调用顺序

### 7.1 初始化阶段

```cpp
// 1. 打开设备
int result = DEV_OpenDevice(NULL, szDllPath);

// 2. 初始化设备
DEV_InitDevice(0);

// 3. 初始化运动控制（如果需要）
#ifdef RY_MOVE_CTL
DEM_Initialize(szMoveDllPath);
DEM_EnableRun();
#endif

// 4. 加载参数
ParameterManager::loadUserParam(g_sysParam);
ParameterManager::loadJobParam(g_testJob);
ParameterManager::loadLayerParam(g_PrtImgLayer);

// 5. 更新设备参数
DEV_UpdateParam(&g_sysParam);
```

### 7.2 打印作业执行（双线程）

**线程1: DataWriterThread (生产者)**
```cpp
// 1. 启动打印作业
int result = IDP_SartPrintJob(&g_testJob);

// 2. 初始化缓冲区
PrtBuffNum = 0;

// 3. 循环写入图层
for (int n = 0; n < layerCount; n++) {
    // 等待缓冲区有空间
    while (PrtBuffNum >= 5) {
        msleep(1);
    }
    
    // 准备数据
    g_PrtImgLayer.nLayerIndex = n;
    g_PrtImgLayer.nValidClrMask = 0xFFFF;
    
    // 写入数据
    PrtBuffNum++;
    WaitForSingleObject(g_PrtMutex, INFINITE);
    IDP_WriteImgLayerData(&g_PrtImgLayer, pBmpFile, dataSize);
    ReleaseMutex(g_PrtMutex);
}
```

**线程2: PrintExecutorThread (消费者)**
```cpp
// 1. 循环处理图层
for (int n = 0; n < layerCount; n++) {
    // 等待数据就绪
    while (PrtBuffNum <= 0) {
        msleep(10);
    }
    
    // 获取锁并启动图层打印
    WaitForSingleObject(g_PrtMutex, INFINITE);
    int nPassCount = IDP_StartLayerPrint(n);
    
    // 重试机制
    while (nPassCount == -1) {
        nPassCount = IDP_StartLayerPrint(n);
    }
    
    // 2. PASS循环
    if (nPassCount > 0) {
        for (int i = 0; i < nPassCount; i++) {
            // 2.1 等待PASS就绪
            LPPassDataItem pPassItem;
            do {
                pPassItem = IDP_GetPassItem(n, i);
                if (pPassItem &&
                    (pPassItem->nProcState > 2) &&
                    (pPassItem->nProcState < 6)) {
                    break;
                }
            } while (true);
            
            // 跳过空PASS
            if (pPassItem->bIsNullPass) continue;
            
            // 2.2 运动控制（可选）
            #ifdef RY_MOVE_CTL
            if (i == 0) {
                // 第一个PASS: Y轴定位
            } else {
                // 后续PASS: Y轴步进
                int nYStep = MM_TO_DOT(pPassItem->nStpVector/1000.0, ...);
                DEM_MoveY(TRUE, nSpeed, nYStep, true, false, ...);
            }
            
            // X轴定位
            if (pPassItem->bEncPrtDir) {
                nDestPos = pPassItem->nStartEncPos + ZERO_POSITION - nMovBuf;
            } else {
                nDestPos = pPassItem->nStartEncPos + ZERO_POSITION + nMovBuf;
            }
            ndistance = nDestPos - DEV_GetPrinterEncValue();
            DEM_MoveX(FALSE, nSpeed, ndistance, TRUE, ...);
            #endif
            
            // 2.3 执行打印
            IDP_DoPassPrint(pPassItem);
            
            // 2.4 X轴回退（可选）
            #ifdef RY_MOVE_CTL
            int nMoveLen = pPassItem->nXPrtColumns * 
                           pPassItem->fPrtPrecession + nMovBuf * 2;
            DEM_MoveX(!pPassItem->bEncPrtDir, nSpeed, nMoveLen, TRUE, ...);
            #endif
        }
    }
    
    // 3. 释放锁和资源
    ReleaseMutex(g_PrtMutex);
    IDP_FreeImageLayer(n);
    PrtBuffNum--;
}
```

### 7.3 清理阶段

```cpp
// 1. 停止打印
IDP_StopPrintJob();

// 2. 释放所有图层
IDP_FreeImageLayer(-1);

// 3. 关闭互斥锁
if (g_PrtMutex != INVALID_HANDLE_VALUE) {
    CloseHandle(g_PrtMutex);
    g_PrtMutex = INVALID_HANDLE_VALUE;
}

// 4. 保存参数
ParameterManager::saveUserParam(g_sysParam);
ParameterManager::saveJobParam(g_testJob);

// 5. 关闭设备
DEV_CloseDevice();
```

---

## 八、常见错误码

### 设备错误码
- `-2`: pcs文件加载失败
- `-3`: 打开卡连接失败
- `-4`: graycfg文件加载失败
- `-5`: 设备数量超过许可

### 图层写入错误码
- `0`: 已存在的图层
- `-110001`: 图层信息为空
- `-110002`: 图层编号不在有效范围
- `-110004`: PC内存不足
- `-110005`: 图像预处理错误

### 图层打印错误码
- `-1`: 图像数据未准备好
- `-120000 ~ -120016`: 启动失败
- `-121000`: 板卡内存分配失败
- `-122000`: 创建数据发送线程失败

---

## 九、使用建议

### 9.1 线程安全

- **所有写入和打印API必须在互斥锁保护下调用**
- 使用Windows互斥锁：`CreateMutex()`, `WaitForSingleObject()`, `ReleaseMutex()`
- 确保所有代码路径都释放互斥锁

### 9.2 缓冲区管理

- 使用`PrtBuffNum`协调生产者和消费者
- 限制最大缓冲区为5个图层
- 生产者：写入后`PrtBuffNum++`
- 消费者：完成后`PrtBuffNum--`

### 9.3 错误处理

- 检查所有API返回值
- 对-1返回值实施重试机制
- 记录错误日志用于诊断
- 清理资源（释放图层、关闭互斥锁）

### 9.4 性能优化

- 使用双线程架构实现流水线处理
- PASS等待循环不要sleep（快速响应）
- 图像数据提前准备到内存
- 合理设置运动速度和加速度

---

## 十、附录

### A. 参考资料

- 单卡网络版扫描系统打印开发手册.pdf
- MFC源代码（NetScanDemo项目）
- Qt移植项目

### B. 联系信息

- SDK版本：单卡网络版
- 技术支持：融跃科技

---

**文档版本**: v1.0  
**最后更新**: 2024年12月8日  
**作者**: AI Assistant  
**状态**: 完整版

