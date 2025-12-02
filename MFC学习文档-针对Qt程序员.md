# MFC 学习文档 - 针对 Qt 程序员的快速入门指南

## 一、前言

本文档面向具有 **Qt 开发经验** 的 C++ 程序员，帮助快速理解 MFC 框架的核心概念和使用方法。通过对比 Qt 和 MFC 的差异，让 Qt 程序员能够快速上手 MFC 项目开发。

---

## 二、Qt vs MFC 核心概念对比

### 2.1 应用程序入口

#### Qt 方式
```cpp
#include <QApplication>
#include <QMainWindow>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QMainWindow window;
    window.show();
    return app.exec();
}
```

#### MFC 方式
```cpp
// NetScanDemo.h
class CNetScanDemoApp : public CWinApp
{
public:
    virtual BOOL InitInstance();
};

// NetScanDemo.cpp
CNetScanDemoApp theApp;  // 全局应用程序对象

BOOL CNetScanDemoApp::InitInstance()
{
    CNetScanDemoDlg dlg;
    m_pMainWnd = &dlg;
    dlg.DoModal();  // 模态对话框
    return FALSE;
}
```

**关键差异**:
- **Qt**: 使用 `QApplication`，通过 `exec()` 进入事件循环
- **MFC**: 使用 `CWinApp`，通过 `InitInstance()` 初始化，全局对象 `theApp` 自动创建
- **Qt**: 主窗口通常继承 `QMainWindow` 或 `QWidget`
- **MFC**: 主窗口通常是 `CDialog` 或 `CDialogEx`，使用 `DoModal()` 显示模态对话框

---

### 2.2 窗口/对话框类

#### Qt 方式
```cpp
class MainWindow : public QMainWindow
{
    Q_OBJECT  // 必须的宏，用于信号槽机制
public:
    MainWindow(QWidget *parent = nullptr);
private slots:
    void onButtonClicked();
private:
    QPushButton *m_button;
};
```

#### MFC 方式
```cpp
// NetScanDemoDlg.h
class CNetScanDemoDlg : public CDialogEx
{
    DECLARE_MESSAGE_MAP()  // 消息映射宏
public:
    CNetScanDemoDlg(CWnd* pParent = NULL);
    enum { IDD = IDD_NETSCANDEMO_DIALOG };  // 对话框资源ID
    
protected:
    virtual void DoDataExchange(CDataExchange* pDX);  // DDX/DDV支持
    
    // 消息处理函数
    afx_msg void OnBnClickedButtonStartjob();
    afx_msg BOOL OnInitDialog();
    
    DECLARE_MESSAGE_MAP()
};

// NetScanDemoDlg.cpp
BEGIN_MESSAGE_MAP(CNetScanDemoDlg, CDialogEx)
    ON_BN_CLICKED(IDC_BUTTON_STARTJOB, &CNetScanDemoDlg::OnBnClickedButtonStartjob)
    ON_WM_INITDIALOG()
END_MESSAGE_MAP()
```

**关键差异**:
- **Qt**: 使用 `Q_OBJECT` 宏和信号槽机制 (`signals`, `slots`)
- **MFC**: 使用消息映射 (`DECLARE_MESSAGE_MAP`, `BEGIN_MESSAGE_MAP`)
- **Qt**: 信号槽是类型安全的，编译时检查
- **MFC**: 消息映射是宏展开，需要手动维护映射关系
- **Qt**: 使用 `ui->button` 访问界面元素
- **MFC**: 使用资源ID (`IDC_BUTTON_STARTJOB`) 和 `GetDlgItem()` 访问控件

---

### 2.3 事件/消息处理

#### Qt 方式
```cpp
// 方式1: 信号槽连接
connect(button, &QPushButton::clicked, this, &MainWindow::onButtonClicked);

// 方式2: 命名约定 (on_控件名_信号名)
void MainWindow::on_button_clicked() { }

// 方式3: 事件过滤器
bool MainWindow::eventFilter(QObject *obj, QEvent *event) { }
```

#### MFC 方式
```cpp
// 消息映射表
BEGIN_MESSAGE_MAP(CNetScanDemoDlg, CDialogEx)
    ON_BN_CLICKED(IDC_BUTTON_STARTJOB, &CNetScanDemoDlg::OnBnClickedButtonStartjob)
    ON_EN_CHANGE(IDC_EDIT_PATH, &CNetScanDemoDlg::OnEnChangeEditPath)
    ON_WM_PAINT()
    ON_WM_DESTROY()
END_MESSAGE_MAP()

// 消息处理函数
void CNetScanDemoDlg::OnBnClickedButtonStartjob()
{
    // 按钮点击处理
}
```

**关键差异**:
- **Qt**: 信号槽是松耦合的，可以在运行时连接
- **MFC**: 消息映射是编译时确定的，静态绑定
- **Qt**: 支持自定义信号和槽
- **MFC**: 只能处理预定义的 Windows 消息
- **Qt**: 事件处理更灵活，可以过滤和转发
- **MFC**: 消息处理是固定的映射关系

---

### 2.4 数据绑定 (DDX/DDV)

#### Qt 方式
```cpp
// Qt 没有内置的数据绑定机制，需要手动同步
void MainWindow::updateUI()
{
    ui->lineEdit->setText(m_data);
}

void MainWindow::onLineEditChanged()
{
    m_data = ui->lineEdit->text();
}
```

#### MFC 方式
```cpp
// NetScanDemoDlg.h
class CNetScanDemoDlg : public CDialogEx
{
    CString m_szPrtFile;  // 成员变量
    BOOL m_bEnableInkWacth;
    
protected:
    virtual void DoDataExchange(CDataExchange* pDX);
};

// NetScanDemoDlg.cpp
void CNetScanDemoDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_MFC_PRT_FILE_PATH, m_szPrtFile);  // 文本控件
    DDX_Check(pDX, IDC_CHECK_INK_WATCH, m_bEnableInkWacth);  // 复选框
    
    // DDV 数据验证
    DDV_MaxChars(pDX, m_szPrtFile, 260);
}

// 使用方式
void CNetScanDemoDlg::OnBnClickedOk()
{
    UpdateData(TRUE);   // 从控件读取数据到成员变量
    // 使用 m_szPrtFile, m_bEnableInkWacth
    UpdateData(FALSE);  // 从成员变量写入数据到控件
}
```

**关键差异**:
- **Qt**: 没有内置数据绑定，需要手动同步或使用 Model/View
- **MFC**: DDX/DDV 提供自动数据交换和验证
- **Qt**: 可以使用 Model/View 框架实现数据绑定
- **MFC**: DDX/DDV 是简单的值绑定，不支持复杂对象

---

### 2.5 字符串处理

#### Qt 方式
```cpp
QString str = "Hello";
QString path = str + "/world";
int len = str.length();
QString num = QString::number(123);
```

#### MFC 方式
```cpp
CString str = _T("Hello");  // _T 宏支持 Unicode/MultiByte
CString path = str + _T("/world");
int len = str.GetLength();
CString num;
num.Format(_T("%d"), 123);
```

**关键差异**:
- **Qt**: `QString` 始终是 Unicode，UTF-16 编码
- **MFC**: `CString` 根据项目设置可能是 Unicode 或 MultiByte
- **Qt**: 使用 `+` 操作符连接字符串
- **MFC**: 也支持 `+`，但更常用 `Format()` 格式化
- **Qt**: 字符串操作更现代，支持正则表达式
- **MFC**: 字符串操作更接近 C 风格

---

### 2.6 文件操作

#### Qt 方式
```cpp
QFile file("data.dat");
if (file.open(QIODevice::ReadWrite))
{
    QDataStream out(&file);
    out << data;
    file.close();
}
```

#### MFC 方式
```cpp
// NetScanDemo.cpp
int CNetScanDemoApp::SaveParam(LPRYUSR_PARAM lparam)
{
    CFile file;
    CString szParamName = m_szAppPath + _T("\\userparam.dat");
    CFileException pError;
    
    if (!file.Open(szParamName, CFile::modeCreate | CFile::modeReadWrite))
    {
        return -1;
    }
    
    file.Write(&g_sysParam, sizeof(RYUSR_PARAM));
    file.Close();
    return 0;
}
```

**关键差异**:
- **Qt**: 使用 `QFile` 和 `QDataStream`，支持类型安全的序列化
- **MFC**: 使用 `CFile`，直接二进制读写
- **Qt**: 异常处理更现代
- **MFC**: 使用 `CFileException` 处理异常

---

### 2.7 线程管理

#### Qt 方式
```cpp
// 方式1: QThread
class WorkerThread : public QThread
{
protected:
    void run() override { /* 工作代码 */ }
};

WorkerThread *thread = new WorkerThread;
thread->start();

// 方式2: moveToThread
QThread *thread = new QThread;
Worker *worker = new Worker;
worker->moveToThread(thread);
connect(thread, &QThread::started, worker, &Worker::doWork);
thread->start();
```

#### MFC 方式
```cpp
// NetScanDemoDlg.cpp
// 静态线程函数
static UINT PrintThread(LPVOID pvoid)
{
    CNetScanDemoDlg* pDlg = (CNetScanDemoDlg*)pvoid;
    // 工作代码
    return 0;
}

// 启动线程
void CNetScanDemoDlg::OnBnClickedButtonStartjob()
{
    AfxBeginThread(PrintThread, this);
}
```

**关键差异**:
- **Qt**: 使用 `QThread` 类，面向对象设计
- **MFC**: 使用静态函数 + `AfxBeginThread()`，C 风格
- **Qt**: 线程间通信使用信号槽
- **MFC**: 线程间通信使用 Windows 消息或事件对象
- **Qt**: 线程管理更安全，自动清理
- **MFC**: 需要手动管理线程生命周期

---

### 2.8 自定义控件

#### Qt 方式
```cpp
class CustomWidget : public QWidget
{
    Q_OBJECT
public:
    CustomWidget(QWidget *parent = nullptr);
protected:
    void paintEvent(QPaintEvent *event) override;
};
```

#### MFC 方式
```cpp
// PicScreen.h
class CPicScreen : public CStatic
{
    DECLARE_MESSAGE_MAP()
public:
    CPicScreen();
    virtual ~CPicScreen();
    
protected:
    afx_msg void OnPaint();
    DECLARE_MESSAGE_MAP()
};

// PicScreen.cpp
BEGIN_MESSAGE_MAP(CPicScreen, CStatic)
    ON_WM_PAINT()
END_MESSAGE_MAP()

void CPicScreen::OnPaint()
{
    CPaintDC dc(this);  // 设备上下文
    // 绘制代码
}
```

**关键差异**:
- **Qt**: 继承 `QWidget`，使用 `paintEvent()` 绘制
- **MFC**: 继承 MFC 控件类（如 `CStatic`），使用 `OnPaint()` 和 `CPaintDC`
- **Qt**: 使用 `QPainter` 进行绘制
- **MFC**: 使用 `CDC` (设备上下文) 进行绘制
- **Qt**: 绘制系统更现代，支持抗锯齿等
- **MFC**: 绘制系统更底层，直接使用 GDI

---

## 三、MFC 核心知识点详解

### 3.1 消息映射机制

#### 3.1.1 消息映射原理
MFC 使用宏展开实现消息映射，将 Windows 消息映射到 C++ 成员函数。

```cpp
// 头文件声明
class CNetScanDemoDlg : public CDialogEx
{
    DECLARE_MESSAGE_MAP()  // 声明消息映射表
    afx_msg void OnBnClickedButtonStartjob();  // 消息处理函数
};

// 实现文件
BEGIN_MESSAGE_MAP(CNetScanDemoDlg, CDialogEx)
    ON_BN_CLICKED(IDC_BUTTON_STARTJOB, &CNetScanDemoDlg::OnBnClickedButtonStartjob)
END_MESSAGE_MAP()
```

**常用消息映射宏**:
- `ON_BN_CLICKED(id, handler)`: 按钮点击
- `ON_EN_CHANGE(id, handler)`: 编辑框内容改变
- `ON_WM_PAINT()`: 绘制消息
- `ON_WM_DESTROY()`: 窗口销毁
- `ON_WM_TIMER()`: 定时器消息
- `ON_COMMAND(id, handler)`: 命令消息

#### 3.1.2 对比 Qt 信号槽
```cpp
// Qt 方式
connect(button, &QPushButton::clicked, this, &MainWindow::onButtonClicked);

// MFC 方式
ON_BN_CLICKED(IDC_BUTTON, &CMainDlg::OnBnClickedButton)
```

---

### 3.2 DDX/DDV 数据交换

#### 3.2.1 DDX (Dialog Data Exchange)
自动在控件和成员变量之间交换数据。

```cpp
void CNetScanDemoDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_EDIT_PATH, m_szPrtFile);      // 文本
    DDX_Check(pDX, IDC_CHECK_ENABLE, m_bEnable);    // 复选框
    DDX_Radio(pDX, IDC_RADIO1, m_nOption);           // 单选按钮
    DDX_Control(pDX, IDC_LIST, m_ListCtrl);          // 控件对象
}
```

#### 3.2.2 DDV (Dialog Data Validation)
数据验证。

```cpp
void CNetScanDemoDlg::DoDataExchange(CDataExchange* pDX)
{
    DDX_Text(pDX, IDC_EDIT_PATH, m_szPrtFile);
    DDV_MaxChars(pDX, m_szPrtFile, 260);  // 最大长度验证
}
```

#### 3.2.3 使用方式
```cpp
// 从控件读取数据到成员变量
UpdateData(TRUE);

// 从成员变量写入数据到控件
UpdateData(FALSE);
```

---

### 3.3 资源管理

#### 3.3.1 资源文件 (.rc)
MFC 使用资源文件定义界面元素：
- 对话框布局
- 控件ID
- 图标、位图
- 字符串表

```cpp
// resource.h
#define IDD_NETSCANDEMO_DIALOG     101
#define IDC_BUTTON_STARTJOB        1001
#define IDC_EDIT_PATH              1002
```

#### 3.3.2 对话框资源
在 Visual Studio 资源编辑器中设计对话框，生成 `.rc` 文件。

#### 3.3.3 对比 Qt
- **Qt**: 使用 `.ui` 文件（XML格式），`uic` 工具生成代码
- **MFC**: 使用 `.rc` 文件（二进制+文本），资源编辑器管理

---

### 3.4 预编译头 (PCH)

#### 3.4.1 stdafx.h
MFC 项目使用预编译头加速编译。

```cpp
// stdafx.h
#pragma once
#include <afxwin.h>         // MFC 核心
#include <afxext.h>         // MFC 扩展
#include <afxcmn.h>         // 通用控件
```

#### 3.4.2 使用方式
```cpp
// 每个 .cpp 文件开头
#include "stdafx.h"  // 必须在最前面
#include "其他头文件.h"
```

#### 3.4.3 对比 Qt
- **Qt**: 没有强制使用预编译头，但可以配置
- **MFC**: 强烈建议使用，显著加速编译

---

### 3.5 应用程序类 (CWinApp)

#### 3.5.1 应用程序对象
```cpp
// NetScanDemo.h
class CNetScanDemoApp : public CWinApp
{
public:
    virtual BOOL InitInstance();
    CString m_szAppPath;
};

// NetScanDemo.cpp
CNetScanDemoApp theApp;  // 全局唯一对象

BOOL CNetScanDemoApp::InitInstance()
{
    // 初始化代码
    CNetScanDemoDlg dlg;
    m_pMainWnd = &dlg;
    dlg.DoModal();
    return FALSE;
}
```

#### 3.5.2 获取应用程序对象
```cpp
CNetScanDemoApp* pApp = (CNetScanDemoApp*)AfxGetApp();
CString path = pApp->m_szAppPath;
```

#### 3.5.3 对比 Qt
- **Qt**: `QApplication::instance()` 获取应用对象
- **MFC**: `AfxGetApp()` 获取应用对象

---

### 3.6 对话框类

#### 3.6.1 CDialog vs CDialogEx
- **CDialog**: 基础对话框类
- **CDialogEx**: 扩展对话框类，支持更多特性（如背景图片）

```cpp
class CNetScanDemoDlg : public CDialogEx
{
    enum { IDD = IDD_NETSCANDEMO_DIALOG };  // 资源ID
public:
    CNetScanDemoDlg(CWnd* pParent = NULL);
protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    virtual BOOL OnInitDialog();
};
```

#### 3.6.2 模态 vs 非模态
```cpp
// 模态对话框（阻塞）
CNetScanDemoDlg dlg;
if (dlg.DoModal() == IDOK)
{
    // 用户点击了确定
}

// 非模态对话框（不阻塞）
CNetScanDemoDlg* pDlg = new CNetScanDemoDlg;
pDlg->Create(IDD_NETSCANDEMO_DIALOG, this);
pDlg->ShowWindow(SW_SHOW);
```

#### 3.6.3 对比 Qt
- **Qt**: `QDialog::exec()` 模态，`show()` 非模态
- **MFC**: `DoModal()` 模态，`Create()` + `ShowWindow()` 非模态

---

### 3.7 控件访问

#### 3.7.1 获取控件指针
```cpp
// 方式1: GetDlgItem()
CWnd* pWnd = GetDlgItem(IDC_BUTTON_STARTJOB);
pWnd->EnableWindow(FALSE);

// 方式2: DDX_Control (推荐)
CButton m_btnStart;
void DoDataExchange(CDataExchange* pDX)
{
    DDX_Control(pDX, IDC_BUTTON_STARTJOB, m_btnStart);
}
// 使用
m_btnStart.EnableWindow(FALSE);
```

#### 3.7.2 常用控件类
- `CButton`: 按钮
- `CEdit`: 编辑框
- `CStatic`: 静态文本
- `CListCtrl`: 列表控件
- `CTreeCtrl`: 树形控件
- `CComboBox`: 组合框

#### 3.7.3 对比 Qt
- **Qt**: 通过 `ui->控件名` 访问，类型安全
- **MFC**: 通过资源ID访问，需要类型转换

---

### 3.8 绘制和图形

#### 3.8.1 设备上下文 (CDC)
```cpp
void CPicScreen::OnPaint()
{
    CPaintDC dc(this);  // 获取设备上下文
    
    // 绘制矩形
    dc.Rectangle(10, 10, 100, 100);
    
    // 绘制文本
    dc.TextOut(10, 10, _T("Hello"));
    
    // 绘制位图
    CBitmap bitmap;
    bitmap.LoadBitmap(IDB_BITMAP1);
    CDC memDC;
    memDC.CreateCompatibleDC(&dc);
    memDC.SelectObject(&bitmap);
    dc.BitBlt(0, 0, 100, 100, &memDC, 0, 0, SRCCOPY);
}
```

#### 3.8.2 对比 Qt
```cpp
// Qt 方式
void CustomWidget::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.drawRect(10, 10, 100, 100);
    painter.drawText(10, 10, "Hello");
}
```

---

### 3.9 文件操作

#### 3.9.1 CFile 类
```cpp
// 写入
CFile file;
if (file.Open(_T("data.dat"), CFile::modeCreate | CFile::modeWrite))
{
    file.Write(&data, sizeof(data));
    file.Close();
}

// 读取
CFile file;
if (file.Open(_T("data.dat"), CFile::modeRead))
{
    file.Read(&data, sizeof(data));
    file.Close();
}
```

#### 3.9.2 CString 路径操作
```cpp
CString path = _T("C:\\Users\\Test");
CString file = path + _T("\\data.dat");
```

#### 3.9.3 对比 Qt
- **Qt**: `QFile`, `QDir`, `QFileInfo` 更现代
- **MFC**: `CFile`, `CString` 更底层

---

## 四、项目中的 MFC 知识点应用

### 4.1 消息映射应用

#### 4.1.1 按钮点击
```cpp
// NetScanDemoDlg.h
afx_msg void OnBnClickedButtonStartjob();

// NetScanDemoDlg.cpp
BEGIN_MESSAGE_MAP(CNetScanDemoDlg, CDialogEx)
    ON_BN_CLICKED(IDC_BUTTON_STARTJOB, &CNetScanDemoDlg::OnBnClickedButtonStartjob)
END_MESSAGE_MAP()

void CNetScanDemoDlg::OnBnClickedButtonStartjob()
{
    if (!m_bJobStarted)
    {
        // 启动打印作业
        m_bJobStarted = true;
        GetDlgItem(IDC_BUTTON_STARTJOB)->SetWindowText(_T("停止打印"));
        AfxBeginThread(PrintThread, this);
    }
    else
    {
        // 停止打印作业
        IDP_StopPrintJob();
        m_bJobStarted = false;
        GetDlgItem(IDC_BUTTON_STARTJOB)->SetWindowText(_T("开始打印"));
    }
}
```

### 4.2 DDX/DDV 应用

#### 4.2.1 数据交换
```cpp
// NetScanDemoDlg.cpp
void CNetScanDemoDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_STATIC_PRT_INFO, m_ListPrtInfo);
    DDX_Control(pDX, IDC_STATIC_VT_INFO, m_ListDrvInfo);
    DDX_Check(pDX, IDC_CHECK_CLOSE_VT_MONITOR, m_bCloseAutoCheck);
    DDX_Control(pDX, IDC_STATIC_RIP_VIEW, m_ImgPreView);
    DDX_Text(pDX, IDC_MFC_PRT_FILE_PATH, m_szPrtFile);
    DDX_Check(pDX, IDC_CHECK_INK_WATCH, m_bEnableInkWacth);
}
```

### 4.3 自定义控件

#### 4.3.1 CPicScreen 控件
```cpp
// PicScreen.h
class CPicScreen : public CStatic
{
    DECLARE_MESSAGE_MAP()
protected:
    afx_msg void OnPaint();
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
};

// PicScreen.cpp
BEGIN_MESSAGE_MAP(CPicScreen, CStatic)
    ON_WM_PAINT()
    ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()

void CPicScreen::OnPaint()
{
    CPaintDC dc(this);
    if (m_hBmp)
    {
        DrawImg(m_hBmp, &dc);
    }
}
```

### 4.4 线程管理

#### 4.4.1 打印线程
```cpp
// NetScanDemoDlg.cpp
static UINT PrintThread(LPVOID pvoid)
{
    CNetScanDemoDlg* pDlg = (CNetScanDemoDlg*)pvoid;
    
    // 等待互斥锁
    WaitForSingleObject(g_PrtMutex, INFINITE);
    
    // 打印处理
    // ...
    
    // 释放互斥锁
    ReleaseMutex(g_PrtMutex);
    
    return 0;
}

// 启动线程
void CNetScanDemoDlg::OnBnClickedButtonStartjob()
{
    g_PrtMutex = CreateMutex(NULL, FALSE, "Mutex");
    AfxBeginThread(PrintThread, this);
}
```

### 4.5 文件操作

#### 4.5.1 参数保存
```cpp
// NetScanDemo.cpp
int CNetScanDemoApp::SaveParam(LPRYUSR_PARAM lparam)
{
    CFile file;
    CString szParamName = m_szAppPath + _T("\\userparam.dat");
    CFileException pError;
    
    if (!file.Open(szParamName, CFile::modeCreate | CFile::modeReadWrite))
    {
        return -1;
    }
    
    file.Write(&g_sysParam, sizeof(RYUSR_PARAM));
    file.Close();
    return 0;
}
```

---

## 五、学习路径建议

### 5.1 第一阶段：基础概念理解（1-2周）

1. **理解 MFC 应用程序结构**
   - 应用程序类 (`CWinApp`)
   - 对话框类 (`CDialog`, `CDialogEx`)
   - 消息映射机制

2. **掌握基本控件使用**
   - 按钮、编辑框、静态文本
   - 列表控件、树形控件
   - 数据交换 (DDX/DDV)

3. **学习资源管理**
   - 资源文件 (.rc)
   - 对话框资源编辑
   - 控件ID定义

**实践项目**: 创建一个简单的对话框应用程序，包含几个控件和基本交互。

---

### 5.2 第二阶段：深入理解（2-3周）

1. **消息处理机制**
   - 消息映射表
   - 自定义消息
   - 消息路由

2. **自定义控件开发**
   - 继承 MFC 控件类
   - 重写绘制函数
   - 自定义消息处理

3. **文件操作和数据结构**
   - `CFile` 类使用
   - `CString` 字符串操作
   - 二进制文件读写

**实践项目**: 开发一个自定义控件，实现图像预览功能。

---

### 5.3 第三阶段：高级特性（2-3周）

1. **多线程编程**
   - `AfxBeginThread()` 使用
   - 线程同步（互斥锁、事件）
   - 线程间通信

2. **图形绘制**
   - `CDC` 设备上下文
   - GDI 绘图函数
   - 位图操作

3. **高级对话框**
   - 属性页 (`CPropertySheet`, `CPropertyPage`)
   - 非模态对话框
   - 动态创建控件

**实践项目**: 实现多线程打印控制，包含设备监控和状态更新。

---

### 5.4 第四阶段：项目实战（3-4周）

1. **理解现有项目**
   - 阅读项目代码
   - 理解模块分工
   - 跟踪数据流

2. **功能扩展**
   - 添加新功能对话框
   - 扩展现有功能
   - 优化代码结构

3. **调试和优化**
   - MFC 调试技巧
   - 内存泄漏检测
   - 性能优化

**实践项目**: 在现有项目中添加新功能，如日志记录、错误处理等。

---

## 六、常见问题和解决方案

### 6.1 消息映射不工作

**问题**: 按钮点击没有响应

**解决方案**:
1. 检查 `DECLARE_MESSAGE_MAP()` 是否在头文件中
2. 检查 `BEGIN_MESSAGE_MAP()` 和 `END_MESSAGE_MAP()` 是否配对
3. 检查消息映射宏的参数是否正确
4. 检查函数声明是否有 `afx_msg` 前缀

### 6.2 DDX 数据不同步

**问题**: 控件显示的值和成员变量不一致

**解决方案**:
1. 确保在需要时调用 `UpdateData(TRUE)` 读取数据
2. 确保在需要时调用 `UpdateData(FALSE)` 写入数据
3. 检查 `DoDataExchange()` 中的 DDX 宏是否正确

### 6.3 线程访问 UI 控件

**问题**: 工作线程中直接访问 UI 控件导致崩溃

**解决方案**:
```cpp
// 错误方式
static UINT ThreadFunc(LPVOID p)
{
    pDlg->m_btnStart.EnableWindow(FALSE);  // 可能崩溃
}

// 正确方式：使用消息
static UINT ThreadFunc(LPVOID p)
{
    pDlg->PostMessage(WM_USER + 100, 0, 0);  // 发送消息
}

// 在主线程中处理消息
ON_MESSAGE(WM_USER + 100, OnThreadMessage)
```

### 6.4 内存泄漏

**问题**: 使用 `new` 分配内存后忘记释放

**解决方案**:
1. 使用智能指针（如果支持 C++11）
2. 在析构函数中释放内存
3. 使用 MFC 的内存管理类（如 `CString` 自动管理）

### 6.5 Unicode/MultiByte 问题

**问题**: 字符串处理出现乱码

**解决方案**:
1. 使用 `_T()` 宏包装字符串字面量
2. 使用 `CString` 而不是 `char*` 或 `wchar_t*`
3. 确保项目字符集设置一致

---

## 七、学习资源推荐

### 7.1 官方文档
- **MSDN MFC 文档**: https://docs.microsoft.com/en-us/cpp/mfc/
- **MFC 类库参考**: Visual Studio 帮助文档

### 7.2 书籍推荐
1. **《深入浅出 MFC》** (侯捷) - 深入理解 MFC 内部机制
2. **《Visual C++ MFC 编程实例》** - 实践性强的教程
3. **《MFC Windows 程序设计》** - 系统性的 MFC 教程

### 7.3 在线资源
- CodeProject MFC 教程
- Stack Overflow MFC 标签
- GitHub MFC 示例项目

---

## 八、总结

### 8.1 关键差异总结

| 特性 | Qt | MFC |
|------|----|----|
| 应用程序类 | QApplication | CWinApp |
| 主窗口 | QMainWindow/QWidget | CDialog/CDialogEx |
| 事件处理 | 信号槽 | 消息映射 |
| 数据绑定 | Model/View | DDX/DDV |
| 字符串 | QString (UTF-16) | CString (Unicode/MultiByte) |
| 文件操作 | QFile/QDataStream | CFile |
| 线程 | QThread | AfxBeginThread |
| 绘制 | QPainter | CDC |
| 资源 | .ui 文件 | .rc 文件 |

### 8.2 学习建议

1. **保持 Qt 思维**: 理解 MFC 的底层实现，但保持 Qt 的面向对象思维
2. **对比学习**: 将 MFC 概念与 Qt 对比，加深理解
3. **实践为主**: 多写代码，多调试，理解 MFC 的工作机制
4. **阅读源码**: 阅读现有项目代码，学习最佳实践
5. **逐步深入**: 从简单功能开始，逐步掌握复杂特性

### 8.3 迁移建议

如果要将 MFC 项目迁移到 Qt:

1. **UI 迁移**: 使用 Qt Designer 重新设计界面
2. **消息映射 → 信号槽**: 将消息映射转换为信号槽连接
3. **DDX/DDV → Model/View**: 使用 Qt 的 Model/View 框架
4. **CString → QString**: 替换字符串类
5. **CFile → QFile**: 替换文件操作类
6. **线程重构**: 使用 QThread 重构线程代码

---

**祝学习顺利！**

