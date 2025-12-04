# MFC打印流程深度解析、Qt移植指南及学习路线

本文档旨在为 C++ 程序员，特别是熟悉 Qt 的开发者，提供一个关于 `net_scam_soft_demo` 项目的完整指南。内容分为三部分：对现有 MFC 应用打印流程的深度解析，如何将其核心功能迁移到 Qt 的具体建议，以及一套高效学习此项目的流程。

---

## 一、 MFC 打印流程深度解析

通过对 `NetScanDemoDlg.cpp` 和相关文档的分析，我们可以将打印流程总结为一个“一个监控，两条流水线”的多线程模型。

### 1. 核心线程架构

整个打印过程由 **4 个主要线程** 协同完成：

1.  **UI 线程**: 负责响应用户交互，启动和停止打印任务，并接收来自其他线程的消息以更新界面。
2.  **监控线程 (`MonitorThread`)**: 一个独立的后台线程，持续轮询硬件状态（如温度、电压、连接状态），并更新到UI。它独立于打印任务，是常驻的。
3.  **数据写入线程 (`PrintThreadSeparate`)**: 打印流水线的**生产者**。负责将图像数据从内存写入到打印控制器的硬件缓冲区。
4.  **打印执行线程 (`PrintThread`)**: 打印流水线的**消费者**。负责协调物理打印动作，包括 XY 轴移动和喷墨。

### 2. 线程同步与“双缓冲”流水线

线程间的同步是此流程的核心。它依赖两个关键的全局变量：

*   `HANDLE g_PrtMutex`: 一个互斥锁，用于保护对 `RYPrtCtler.dll` API 的关键调用。由于 DLL 内部可能不是完全线程安全的，此锁确保了在任何时刻只有一个线程在调用如 `IDP_WriteImgLayerData` 或 `IDP_StartLayerPrint` 等核心函数。
*   `int PrtBuffNum`: 一个整数计数器，扮演**信号量**的角色。它代表硬件缓冲区中已准备好但尚未打印的图层数量。
    *   `PrintThreadSeparate` 每成功写入一层数据，就执行 `PrtBuffNum++`。
    *   `PrintThread` 每成功打印完一层数据，就执行 `PrtBuffNum--`。
    *   此机制形成了一个经典的“生产者-消费者”模型，实现了数据准备和物理打印的并行执行，大大提高了效率。

### 3. 详细工作流

**[启动阶段]**

1.  **UI 线程**: 用户点击“开始打印”，触发 `OnBnClickedButtonStartjob()`。
2.  **UI 线程**: 函数内弹出 `CPrtJobDlg` 对话框，用户设置打印参数。参数被保存在全局结构体 `g_testJob` 中。
3.  **UI 线程**: 创建互斥锁 `g_PrtMutex`，并将 `PrtBuffNum` 初始化为 0。
4.  **UI 线程**: 使用 `AfxBeginThread` 同时启动 `PrintThreadSeparate` 和 `PrintThread` 两个工作线程。

**[执行阶段 - 流水线并行工作]**

| **数据写入线程 (`PrintThreadSeparate`)** | **打印执行线程 (`PrintThread`)** |
| :--- | :--- |
| 1. 调用 `IDP_SartPrintJob(&g_testJob)` **一次**，正式开启打印作业。 | 1. 循环等待 `PrtBuffNum > 0`。这表示至少有一层数据准备好了。 |
| 2. 进入 `for` 循环，遍历所有待打印的图层。 | 2. 进入 `for` 循环，遍历所有待打印的图层。 |
| 3. **检查缓冲区**: 如果 `PrtBuffNum >= 5`（缓冲区已满），则 `Sleep(1)` 等待。 | 3. **获取互斥锁**: `WaitForSingleObject(g_PrtMutex, ...)` |
| 4. **获取互斥锁**: `WaitForSingleObject(g_PrtMutex, ...)` | 4. 调用 `IDP_StartLayerPrint(n)`，通知硬件准备打印第 `n` 层，并获取该层包含的 Pass 总数 (`nPassCount`)。 |
| 5. **写入数据**: 调用 `IDP_WriteImgLayerData()` 将当前图层的图像数据写入硬件。 | 5. 进入内层 `for` 循环，遍历当前图层的所有 Pass。 |
| 6. **释放互斥锁**: `ReleaseMutex(g_PrtMutex)` | 6. **获取 Pass 数据**: 循环调用 `IDP_GetPassItem(n, i)`，直到获取到有效的 Pass 数据（包含 XY 坐标、步进等信息）。 |
| 7. **更新计数**: `PrtBuffNum++`，通知消费者有新数据可用。 | 7. **运动控制**: 调用 `DEM_MoveX()` / `DEM_MoveY()` 控制平台移动到指定位置。 |
| 8. 循环，直到所有图层数据写入完毕。 | 8. **执行打印**: 调用 `IDP_DoPassPrint()` 执行当前 Pass 的喷墨。 |
| | 9. **释放图层**: 一层的所有 Pass 打印完毕后，调用 `IDP_FreeImageLayer(n)` 释放硬件资源。 |
| | 10. **释放互斥锁**: `ReleaseMutex(g_PrtMutex)` |
| | 11. **更新计数**: `PrtBuffNum--`，通知生产者缓冲区有新空间。 |
| | 12. 循环，直到所有图层打印完毕。 |

**[结束阶段]**

1.  **打印执行线程 (`PrintThread`)**: 打印完成后，显示“打印完成”消息，并调用 `IDP_StopPrintJob()` 结束作业。
2.  **打印执行线程 (`PrintThread`)**: 清理资源，如关闭互斥锁句柄 `CloseHandle(g_PrtMutex)`，并重置UI状态。

---

## 二、 Qt 移植指南

对于 Qt 程序员，将此项目迁移的关键在于**解耦 UI 和硬件控制逻辑**。MFC 版本将大量逻辑耦合在 `CNetScanDemoDlg` 类中，这在 Qt 中是不推荐的。

### 1. 推荐架构：模型-视图-控制器 (MVC) 思想

*   **模型 (Model)**: 封装所有与硬件交互的逻辑。
*   **视图 (View)**: 你的 Qt 窗口和控件 (`QMainWindow`, `QDialog` 等)。
*   **控制器 (Controller)**: 负责连接视图和模型，即 Qt 的信号和槽机制。

### 2. 迁移步骤

#### 步骤 1: 创建 `HardwareController` 单例类

这是最关键的一步。创建一个名为 `HardwareController` 的 C++ 类，并将其设计为单例模式。

*   **职责**:
    *   它将是项目中**唯一** `#include "ryprtapi.h"` 和 `#include "MoveCtl.h"` 的地方。
    *   封装所有对 `DEV_...`, `IDP_...`, `DEM_...` 函数的调用。
    *   管理设备状态（如 `bool m_isConnected`）、打印状态、全局参数（如 `g_testJob`，应变为其成员变量 `m_currentJob`）。
*   **示例 (`HardwareController.h`)**:
    ```cpp
    class HardwareController : public QObject {
        Q_OBJECT
    public:
        static HardwareController* instance();
        void connectDevice();
        void startPrintJob(const PRTJOB_ITEM& job);
        // ... 其他方法
    signals:
        void deviceStatusUpdated(const QString& status);
        void printJobFinished();
        void errorOccurred(const QString& message);
    private:
        // ... 封装所有 API 调用
        // ... 管理所有状态变量和参数
    };
    ```

#### 步骤 2: 使用 `QThread` 重构线程

将 MFC 的三个线程逻辑迁移到 Qt 的现代线程模型中。

1.  **创建 `QObject` 工作类**: 为每个线程（Monitor, PrintDataWriter, PrintExecutor）创建一个继承自 `QObject` 的工作类。
2.  **移动逻辑**: 将原 `MonitorThread`、`PrintThreadSeparate`、`PrintThread` 函数中的 `while` 或 `for` 循环逻辑，移动到对应工作类的槽函数中（例如 `doMonitoring()`, `doWriting()`, `doPrinting()`）。
3.  **使用 `moveToThread`**: 在 `HardwareController` 中，为每个工作类创建 `QThread` 实例，并使用 `worker->moveToThread(thread)` 将工作对象移到新线程中。通过信号槽触发其开始工作。

#### 步骤 3: 使用信号和槽进行通信

这是 Qt 的精髓，用于替代 MFC 中的 `PostMessage`、全局变量或直接成员访问。

*   **工作线程 -> `HardwareController`**: 当工作线程中发生事件（如状态更新、打印完一层），它会 `emit` 一个信号。`HardwareController` 接收此信号并更新自己的内部状态。
*   **`HardwareController` -> UI**: 当 `HardwareController` 的状态改变时（例如收到了工作线程的状态更新），它会 `emit` 一个信号（如 `deviceStatusUpdated`）。
*   **UI -> `HardwareController`**: 当用户点击按钮时，UI 会调用 `HardwareController` 的公共槽函数（如 `startPrintJob`）。

**示例流程**:
1.  `MainWindow` 点击按钮 -> 调用 `HardwareController::instance()->startPrintJob(job)`。
2.  `HardwareController` 启动内部的打印 `QThread`。
3.  打印线程 `emit printProgress(25)`。
4.  `HardwareController` 接收到 `printProgress(25)`，然后 `emit` `jobProgressUpdated(25)`。
5.  `MainWindow` 的 `QProgressBar` 连接了 `jobProgressUpdated` 信号，自动更新进度条。

#### 步骤 4: 重建 UI

使用 Qt Designer 或纯代码方式，重新创建 `CAdjParamDlg`、`CPrtJobDlg` 等对话框，并将它们的逻辑连接到 `HardwareController`。

---

## 三、 项目学习流程建议

对于刚接触此项目的新人，建议遵循以下自顶向下的学习路径：

#### 第 1 步: 理解“是什么” - 阅读官方文档

*   **目标**: 了解系统的宏观概念和能力。
*   **材料**: `单卡网络版扫描系统打印开发手册 .pdf`
*   **关键点**: 不要一开始就陷入代码细节。先理解硬件系统构成、什么是“层(Layer)”和“趟(Pass)”，以及 SDK 的设计目标。

#### 第 2 步: 理解“语言” - 熟悉核心 API 和数据结构

*   **目标**: 学习如何与硬件“对话”。
*   **材料**: `Inc/ryprtapi.h`, `Inc/MoveCtl.h`
*   **关键点**:
    *   重点研究 `PRTJOB_ITEM`、`PRTIMG_LAYER`、`RYUSR_SYSPARAM` 等关键**数据结构**。它们是命令和数据的载体。
    *   浏览一遍所有 `IDP_...` 和 `DEV_...` 函数名，对其功能有一个大致印象。

#### 第 3 步: 理解“范例” - 分析 MFC 核心实现

*   **目标**: 掌握一个完整的、实际的工作流程。
*   **材料**: `NetScanDemo/NetScanDemoDlg.cpp`
*   **关键点**:
    1.  从 `InitSystem()` 开始，理解设备如何初始化。
    2.  以 `OnBnClickedButtonStartjob()` 为入口，跟踪一个打印任务的完整生命周期。
    3.  **重点分析** `MonitorThread`, `PrintThreadSeparate`, 和 `PrintThread` 三个线程函数。理解它们的职责、交互方式以及同步机制（互斥锁和计数器）。这是整个项目的技术核心。

#### 第 4 步: 理解“交互” - 关联 UI 与逻辑

*   **目标**: 明白用户输入如何转化为 API 调用。
*   **材料**: `NetScanDemo` 目录下的各个 `C...Dlg.cpp` 文件。
*   **关键点**: 查看各个对话框的代码，了解它们如何收集用户输入（如DPI、打印位置），并填充到 `g_testJob` 等核心数据结构中。

#### 第 5 步: 理解“未来” - 审视 Qt 移植尝试

*   **目标**: 了解已有的移植工作和思路。
*   **材料**: `qt/` 目录。
*   **关键点**: 在完成以上步骤后，再去审视 `qt` 目录的代码。你会更容易理解其设计意图、现有成果以及未完成的部分，为真正的重构和移植打下坚实基础。
