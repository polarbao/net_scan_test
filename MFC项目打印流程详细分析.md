# MFC项目打印流程详细分析

## 一、整体流程概览

从用户点击"开始打印"按钮到完成打印，整个流程涉及多个阶段、多个线程和多个API调用。以下是完整的流程分析。

---

## 二、完整流程步骤

### 阶段1：用户操作和准备阶段

#### 1.1 加载图像文件（可选，在打印前完成）

**触发点**: 用户点击"加载图像"按钮

**函数调用链**:
```
OnBnClickedButtonLoadimg()
    ↓
CImgLayerSetDlg::DoModal()  // 图像图层设置对话框
    ↓
SaveLayerParam()  // 保存图层参数
```

**关键操作**:
- 弹出图像图层设置对话框
- 配置图层参数（DPI、打印方向、羽化模式等）
- 保存图层参数到 `pmc.dat`

**注意**: 图像数据本身可能在此阶段加载，也可能在打印时加载。

---

#### 1.2 点击"开始打印"按钮

**触发点**: 用户点击"开始打印"按钮 (`IDC_BUTTON_STARTJOB`)

**函数**: `CNetScanDemoDlg::OnBnClickedButtonStartjob()`

**代码位置**: `NetScanDemoDlg.cpp:590`

---

### 阶段2：打印前验证和准备

#### 2.1 图像数据验证和加载

```cpp
// 代码位置: NetScanDemoDlg.cpp:594-611
if(1)
{
    if (g_PrtImgLayer.nImgType == 0) 
    {
        // 如果图像类型为0（BMP格式），需要加载图像数据
        int nRet = GetSrcData(m_szPrtFile);
        if (nRet < 0) {
            // 错误处理
            if (nRet == -1)
                AfxMessageBox(_T("请确认图像信息"));
            else
                AfxMessageBox(_T("图片加载未成功加载"));
            return;  // 打印终止
        }  
    }
    if(!m_bImageLoaded)
    {
        AfxMessageBox(_T("未加载图像文件"));
        return;  // 打印终止
    }
}
```

**GetSrcData() 函数详细流程** (`NetScanDemoDlg.cpp:645`):

1. **文件路径处理**
   - 根据颜色数量（`g_PrtImgLayer.nColorCnts`）构建文件路径
   - 格式: `路径\颜色索引.bmp`（如 `\1.bmp`, `\2.bmp`）

2. **文件读取**
   ```cpp
   CFile file;
   if (!file.Open(lpSrcFile, CFile::modeRead))
       return -2;  // 文件打开失败
   m_nPrtDataSize = file.GetLength();
   ```

3. **内存分配**
   ```cpp
   LPBYTE pDataBuf = new BYTE[m_nPrtDataSize + 0x100000];
   file.Read(pDataBuf, m_nPrtDataSize);
   ```

4. **BMP文件解析**
   - 解析 `BITMAPFILEHEADER` 和 `BITMAPINFOHEADER`
   - 验证位深度（必须为1位，单色图像）
   - 计算图像数据偏移量
   - 处理颜色反转（如果零颜色为黑色）

5. **数据存储**
   ```cpp
   m_pPrtData[c] = new BYTE[m_nPrtDataSize + 0x100000];
   CopyMemory(m_pPrtData[c], pSrcData, m_nPrtDataSize);
   ```

6. **图层信息更新**
   ```cpp
   g_PrtImgLayer.nBytesPerLine = ...;  // 每行字节数
   g_PrtImgLayer.nHeight = pbmpheader->biHeight;
   g_PrtImgLayer.nWidth = pbmpheader->biWidth / g_PrtImgLayer.nGrayBits;
   m_bImageLoaded = TRUE;
   ```

**关键数据结构**:
- `m_pPrtData[]`: 打印数据缓冲区数组（每个颜色一个缓冲区）
- `g_PrtImgLayer`: 全局图层信息结构

---

#### 2.2 打印作业参数配置

```cpp
// 代码位置: NetScanDemoDlg.cpp:613-635
if (!m_bJobStarted) 
{
    CPrtJobDlg dlg;  // 创建打印作业对话框
    dlg.m_nImageLayerCount = m_nJobImgLayerCnts;
    if (dlg.DoModal() == IDOK)  // 显示模态对话框
    {
        // 用户确认后，获取配置的参数
        m_nJobImgLayerCnts = dlg.m_nImageLayerCount;
        
        if(m_nJobImgLayerCnts > 0)
        {
            // 准备启动打印
            m_bJobStarted = true;
            GetDlgItem(IDC_BUTTON_LOADIMG)->EnableWindow(true);
            GetDlgItem(IDC_BUTTON_STARTJOB)->SetWindowText(_T("停止打印"));
            g_PrtImgLayer.nLayerIndex = -1;
            
            // 创建互斥锁
            g_PrtMutex = CreateMutex(NULL, FALSE, "Mutex");
            
            // 启动两个线程
            AfxBeginThread(PrintThreadSeparate, this);  // 数据写入线程
            AfxBeginThread(PrintThread, this);          // 打印执行线程
            
            // 保存作业参数
            CNetScanDemoApp* pApp = (CNetScanDemoApp*)AfxGetApp();
            if (pApp) {
                pApp->SaveJobParam();
            }
        }
    }
}
```

**CPrtJobDlg 对话框参数** (`CPrtJobDlg.cpp:73`):
- 打印位置 (`m_fXPrtPos`, `m_fYPrtPos`)
- 输出DPI (`m_fXOutDPI`, `m_fYOutDPI`)
- 裁剪尺寸 (`m_fClipWidth`, `m_fClipHeight`)
- 图像层数 (`m_nImageLayerCount`)
- 打印控制标志:
  - `m_bWhiteJump`: 白色跳过
  - `m_bCycleOff`: 循环关闭
  - `m_bRadomJetOff`: 随机喷头关闭
  - `m_bXMirror`: X镜像
  - `m_bYMirror`: Y镜像

**参数保存到全局变量**:
```cpp
g_testJob.fPrtXPos = m_fXPrtPos;
g_testJob.nPrtCtl |= 0x1;  // 根据复选框设置控制位
```

---

### 阶段3：多线程打印执行

#### 3.1 线程架构

系统启动**两个工作线程**并行工作：

1. **PrintThreadSeparate** (数据写入线程)
   - 负责将图像数据写入打印系统
   - 与打印执行线程并行工作

2. **PrintThread** (打印执行线程)
   - 负责实际的打印执行
   - 等待数据就绪后执行打印

**同步机制**:
- 使用Windows互斥锁 `g_PrtMutex` 保护共享资源
- 使用全局变量 `PrtBuffNum` 控制缓冲区状态

---

#### 3.2 PrintThreadSeparate 线程（数据写入线程）

**函数**: `CNetScanDemoDlg::PrintThreadSeparate()`

**代码位置**: `NetScanDemoDlg.cpp:816`

**详细流程**:

```cpp
UINT CNetScanDemoDlg::PrintThreadSeparate(LPVOID pvoid)
{
    CNetScanDemoDlg* pdlg = (CNetScanDemoDlg*)pvoid;
    
    // 步骤1: 启动打印作业
    int nResult = IDP_SartPrintJob(&g_testJob);
    if (nResult < 0) {
        // 启动失败，显示错误并退出
        AfxMessageBox(_T("启动打印失败，错误码为：%d"), nResult);
        return -1;
    }
    
    // 步骤2: 初始化缓冲区计数
    PrtBuffNum = 0;
    
    // 步骤3: 遍历所有图层
    for(int n = 0; n < pdlg->m_nJobImgLayerCnts; n++)
    {
        // 步骤3.1: 准备图像数据指针
        for (int i = 0; i < g_PrtImgLayer.nColorCnts; i++) 
        {
            pBmpFile[i] = pdlg->m_pPrtData[i];  // 获取图像数据缓冲区
        }
        
        // 步骤3.2: 设置图层信息
        pLayer->nLayerIndex = n;
        pLayer->nValidClrMask = 0xFFFF;
        
        // 步骤3.3: 缓冲区管理（最多缓存5个图层）
        if(PrtBuffNum < 5)
        {
            PrtBuffNum++;  // 增加缓冲区计数
            
            // 步骤3.4: 获取互斥锁
            WaitForSingleObject(g_PrtMutex, INFINITE);
            
            // 步骤3.5: 写入图像图层数据
            Result = IDP_WriteImgLayerData(pLayer, pBmpFile, pdlg->m_nPrtDataSize);
            
            // 步骤3.6: 释放互斥锁
            ReleaseMutex(g_PrtMutex);
            
            if(Result != 1)
            {
                // 写入失败处理
                AfxMessageBox("111111");
            }
        }
        else
        {
            // 缓冲区已满，等待
            while(PrtBuffNum >= 5){
                Sleep(1);
            }
            n--;  // 重试当前图层
        }
    }
    
    // 步骤4: 线程结束
    AfxEndThread(0, TRUE);
}
```

**关键API调用**:
- `IDP_SartPrintJob(&g_testJob)`: 启动打印作业
- `IDP_WriteImgLayerData()`: 写入图像图层数据到打印系统

**缓冲区管理**:
- `PrtBuffNum`: 全局缓冲区计数，最多缓存5个图层
- 当缓冲区满时，线程等待直到有空闲位置

---

#### 3.3 PrintThread 线程（打印执行线程）

**函数**: `CNetScanDemoDlg::PrintThread()`

**代码位置**: `NetScanDemoDlg.cpp:895`

**详细流程**:

```cpp
UINT CNetScanDemoDlg::PrintThread(LPVOID pvoid)
{
    CNetScanDemoDlg* pdlg = (CNetScanDemoDlg*)pvoid;
    pdlg->m_bPrinting = true;
    
    // 步骤1: 遍历所有图层
    for(int n = 0; n < pdlg->m_nJobImgLayerCnts; n++)
    {
        // 步骤1.1: 等待缓冲区就绪
        while (PrtBuffNum <= 0)
        {
            Sleep(10);  // 等待数据写入线程准备好数据
        }
        
        // 步骤1.2: 获取互斥锁
        WaitForSingleObject(g_PrtMutex, INFINITE);
        
        // 步骤1.3: 启动图层打印
        nPassCount = IDP_StartLayerPrint(n);
        
        // 步骤1.4: 等待图层数据就绪（重试机制）
        while(nPassCount == -1)
        {
            ReleaseMutex(g_PrtMutex);
            Sleep(10);
            WaitForSingleObject(g_PrtMutex, INFINITE);
            nPassCount = IDP_StartLayerPrint(n);
        }
        
        // 步骤1.5: 处理PASS循环
        if(nPassCount > 0)
        {
            for(int i = 0; i < nPassCount; i++)
            {
                // 步骤1.5.1: 等待PASS数据就绪
                do
                {
                    pPrtPassDes = IDP_GetPassItem(n, i);
                    if(pPrtPassDes)
                    {
                        // 状态3,4,5都可以执行打印
                        if((pPrtPassDes->nProcState > 2) && 
                           (pPrtPassDes->nProcState < 6))
                        {
                            break;  // PASS就绪，退出等待循环
                        }
                    }
                    if(!pdlg->m_bJobStarted)
                    {
                        // 作业已停止
                        return -1;
                    }
                } while(1);  // 无限循环等待PASS就绪
                
                // 步骤1.5.2: 检查是否为空PASS
                if (pPrtPassDes->bIsNullPass)
                    continue;  // 跳过空PASS
                
                // 步骤1.5.3: Y轴定位（第一个PASS）
                if(0 == i)
                {
                    // 第一个PASS Y位置定位
                    // pPrtPassDes->fYMinJetImgPos 为Y开始打印位置到Y定位的距离
                }
                else
                {
                    // 打印之前需要Y轴移动
                    #ifdef RY_MOVE_CTL
                    int nYStep = MM_TO_DOT(pPrtPassDes->nStpVector/1000.0, ...);
                    DEM_MoveY(TRUE, nSpd, nYStep, true, false, ...);
                    #endif
                }
                
                // 步骤1.5.4: X轴定位
                #ifdef RY_MOVE_CTL
                if (pPrtPassDes->bEncPrtDir) {
                    nDestPos = (int)pPrtPassDes->nStartEncPos + ZERO_POSITION - nMovBuf;
                }
                else {
                    nDestPos = (int)pPrtPassDes->nStartEncPos + ZERO_POSITION + nMovBuf;
                }
                DEM_MoveX(FALSE, nSpd, ndistance, TRUE, ...);
                #endif
                
                // 步骤1.5.5: 执行PASS打印
                if(IDP_DoPassPrint(pPrtPassDes) > 0)
                {
                    // 打印成功
                    // X轴回退运动
                    #ifdef RY_MOVE_CTL
                    DEM_MoveX(!pPrtPassDes->bEncPrtDir, nSpd, nMoveLen, TRUE, ...);
                    #endif
                }
                else
                {
                    // 打印失败处理
                }
            }  // end for PASS循环
        }
        else
        {
            // 图层处理失败
            ReleaseMutex(g_PrtMutex);
            AfxMessageBox(_T("图层处理错误，错误码：%d"), nPassCount);
            break;
        }
        
        // 步骤1.6: 释放互斥锁
        ReleaseMutex(g_PrtMutex);
        
        // 步骤1.7: 释放图层资源
        IDP_FreeImageLayer(n);
        PrtBuffNum--;  // 减少缓冲区计数
    }  // end for 图层循环
    
    // 步骤2: 打印完成处理
    AfxMessageBox(_T("打印完成"));
    IDP_StopPrintJob();
    IDP_FreeImageLayer(-1);
    CloseHandle(g_PrtMutex);
    
    // 步骤3: 更新状态
    pdlg->m_bPrinting = false;
    pdlg->m_bJobStarted = false;
    pdlg->UpdateButtonState();
    
    // 步骤4: 线程结束
    AfxEndThread(0, TRUE);
    return 0;
}
```

**关键API调用**:
- `IDP_StartLayerPrint(n)`: 启动指定图层的打印
  - 返回值: >0 返回PASS数量, -1 图像数据未准备好, 其他负数为错误码
- `IDP_GetPassItem(n, i)`: 获取PASS数据项
  - 返回PASS数据指针，包含打印位置、步进向量等信息
- `IDP_DoPassPrint(pPrtPassDes)`: 执行指定PASS的打印
- `IDP_GetPrintState(&prtInfo)`: 获取打印运行状态
- `IDP_FreeImageLayer(n)`: 释放图层资源
- `IDP_StopPrintJob()`: 停止打印作业

**PASS状态说明**:
- 状态 0-2: 未就绪
- 状态 3-5: 可以执行打印
- 状态 6+: 其他状态

**运动控制** (如果启用 `RY_MOVE_CTL`):
- `DEM_MoveY()`: Y轴移动（用于PASS之间的步进）
- `DEM_MoveX()`: X轴移动（用于打印位置定位和回退）

---

### 阶段4：打印监控（并行进行）

#### 4.1 MonitorThread 线程

**函数**: `CNetScanDemoDlg::MonitorThread()`

**代码位置**: `NetScanDemoDlg.cpp:527`

**功能**:
- 持续监控设备状态（温度、电压、连接状态）
- 更新界面显示
- 检查打印状态

**执行流程**:
```cpp
while (!pCtlDlg->m_bStopMonitor)
{
    // 检查设备连接
    DEV_DeviceIsConnected();
    
    // 获取打印状态
    IDP_GetPrintState(&prtInfo);
    
    // 更新设备信息（在空闲或完成状态）
    if (prtInfo.nPrtState == 0 || prtInfo.nPrtState == 3)
    {
        if ((GetTickCount() - dwChkTick) > 200)
        {
            if (!pCtlDlg->m_bCloseAutoCheck)
            {
                g_pSysInfo = DEV_GetDeviceInfo();
                pCtlDlg->UpdateVTListInfo();  // 更新界面
            }
            dwChkTick = GetTickCount();
        }
    }
    
    // 更新打印状态
    pCtlDlg->UpdatePrtState(prtInfo.nPrtState);
    
    Sleep(100);  // 休眠100ms
}
```

---

### 阶段5：打印完成和清理

#### 5.1 打印完成处理

**在PrintThread中**:
```cpp
// 所有图层打印完成
AfxMessageBox(_T("打印完成"));
IDP_StopPrintJob();           // 停止打印作业
IDP_FreeImageLayer(-1);       // 释放所有图层资源
CloseHandle(g_PrtMutex);      // 关闭互斥锁

// 更新状态
pdlg->m_bPrinting = false;
pdlg->m_bJobStarted = false;
pdlg->UpdateButtonState();    // 更新按钮状态
```

#### 5.2 用户停止打印

**触发点**: 用户再次点击"停止打印"按钮

**处理**:
```cpp
else  // m_bJobStarted == true
{
    IDP_StopPrintJob();        // 立即停止打印作业
    m_bJobStarted = false;
    GetDlgItem(IDC_BUTTON_STARTJOB)->SetWindowText(_T("开始打印"));
}
```

---

## 三、数据流图

```
用户操作
    ↓
[1] 点击"开始打印"按钮
    ↓
[2] OnBnClickedButtonStartjob()
    ├─→ 验证图像是否加载
    │   └─→ GetSrcData() [如果未加载]
    │       ├─→ 读取BMP文件
    │       ├─→ 解析BMP头
    │       └─→ 存储到 m_pPrtData[]
    │
    ├─→ 弹出CPrtJobDlg配置参数
    │   └─→ 保存到 g_testJob
    │
    └─→ 创建互斥锁 g_PrtMutex
        │
        ├─→ [线程1] PrintThreadSeparate
        │   │
        │   ├─→ IDP_SartPrintJob(&g_testJob)
        │   │
        │   └─→ 循环处理每个图层
        │       ├─→ 等待缓冲区空间 (PrtBuffNum < 5)
        │       ├─→ 获取互斥锁
        │       ├─→ IDP_WriteImgLayerData()
        │       ├─→ 释放互斥锁
        │       └─→ PrtBuffNum++
        │
        └─→ [线程2] PrintThread
            │
            └─→ 循环处理每个图层
                ├─→ 等待缓冲区就绪 (PrtBuffNum > 0)
                ├─→ 获取互斥锁
                ├─→ IDP_StartLayerPrint(n)
                │   └─→ 返回PASS数量
                │
                └─→ 循环处理每个PASS
                    ├─→ IDP_GetPassItem(n, i)
                    │   └─→ 等待状态3-5
                    ├─→ 运动控制 (Y轴定位/步进, X轴定位)
                    ├─→ IDP_DoPassPrint(pPrtPassDes)
                    │   └─→ 实际打印执行
                    ├─→ 运动控制 (X轴回退)
                    └─→ IDP_GetPrintState() 监控状态
                        │
                        └─→ 打印完成
                            ├─→ IDP_StopPrintJob()
                            ├─→ IDP_FreeImageLayer(-1)
                            ├─→ CloseHandle(g_PrtMutex)
                            └─→ 更新界面状态
```

---

## 四、关键数据结构

### 4.1 全局变量

```cpp
// 打印作业参数
PRTJOB_ITEM g_testJob;

// 图像图层参数
PRTIMG_LAYER g_PrtImgLayer;

// 系统信息
LPPRINTER_INFO g_pSysInfo;

// 互斥锁
HANDLE g_PrtMutex;

// 缓冲区计数
int PrtBuffNum;
```

### 4.2 主对话框成员变量

```cpp
// 打印数据缓冲区（每个颜色一个）
LPBYTE m_pPrtData[MAX_COLORS];

// 状态标志
bool m_bJobStarted;      // 作业是否已启动
bool m_bPrinting;         // 是否正在打印
bool m_bImageLoaded;      // 图像是否已加载

// 作业参数
int m_nJobImgLayerCnts;   // 图像图层数量
int m_nPrtDataSize;       // 打印数据大小
```

---

## 五、线程同步机制

### 5.1 互斥锁使用

**创建**:
```cpp
g_PrtMutex = CreateMutex(NULL, FALSE, "Mutex");
```

**使用场景**:
1. PrintThreadSeparate写入数据时
2. PrintThread启动图层打印时

**释放**:
```cpp
ReleaseMutex(g_PrtMutex);
CloseHandle(g_PrtMutex);  // 打印完成后
```

### 5.2 缓冲区计数

**PrtBuffNum**:
- PrintThreadSeparate写入后: `PrtBuffNum++`
- PrintThread处理完成后: `PrtBuffNum--`
- PrintThread等待: `while (PrtBuffNum <= 0) Sleep(10);`
- PrintThreadSeparate等待: `while (PrtBuffNum >= 5) Sleep(1);`

---

## 六、错误处理

### 6.1 图像加载错误
- 文件不存在: 返回-2
- 文件格式错误: 返回-3
- 图像信息错误: 返回-1

### 6.2 打印启动错误
- `IDP_SartPrintJob()` 返回负值: 显示错误码并退出

### 6.3 图层处理错误
- `IDP_StartLayerPrint()` 返回负值: 显示错误信息
- `IDP_DoPassPrint()` 返回负值: 打印失败处理

### 6.4 超时处理
- PASS等待超时（60秒）: 显示警告信息

---

## 七、性能优化点

### 7.1 缓冲区管理
- 最多缓存5个图层，实现流水线处理
- 数据写入和打印执行并行进行

### 7.2 等待策略
- 使用 `Sleep()` 避免CPU占用过高
- 等待时间: 10ms（缓冲区等待）、1ms（缓冲区满等待）

### 7.3 资源管理
- 及时释放图层资源 (`IDP_FreeImageLayer`)
- 打印完成后关闭互斥锁句柄

---

## 八、与Qt版本的对应关系

| MFC阶段 | Qt对应 |
|---------|--------|
| OnBnClickedButtonStartjob() | MainWindow::onButtonStartJobClicked() |
| GetSrcData() | ImageLoader类（待实现） |
| CPrtJobDlg | PrintJobDialog |
| PrintThreadSeparate | 可集成到PrintThread或单独线程 |
| PrintThread | PrintThread类 |
| MonitorThread | MonitorThread类 |
| g_PrtMutex | QMutex + Windows HANDLE |
| PrtBuffNum | 可使用QSemaphore管理 |

---

## 九、总结

### 9.1 流程特点

1. **双线程架构**: 数据写入和打印执行并行，提高效率
2. **流水线处理**: 缓冲区管理实现图层处理的流水线
3. **状态驱动**: 通过PASS状态控制打印执行
4. **运动控制集成**: 支持X/Y轴运动控制（可选）

### 9.2 关键成功因素

1. **图像数据准备**: 必须正确加载和解析图像文件
2. **参数配置**: 作业参数和图层参数必须正确设置
3. **线程同步**: 互斥锁和缓冲区计数确保数据一致性
4. **设备状态**: 设备必须连接且初始化完成

### 9.3 潜在问题

1. **死锁风险**: 互斥锁使用不当可能导致死锁
2. **资源泄漏**: 如果异常退出，资源可能未释放
3. **超时处理**: PASS等待可能无限等待
4. **错误恢复**: 部分错误情况下恢复机制不完善

---

**文档版本**: 1.0  
**最后更新**: 2024年

