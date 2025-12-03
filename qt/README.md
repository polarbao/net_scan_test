# NetScanDemo Qt 转换项目

## 项目说明

这是将 MFC 版本的 NetScanDemo 项目转换为 Qt 框架的示范代码。

## 文件结构

```
qt/
├── README.md                    # 本文件
├── Qt转换方案.md                # 详细的转换方案文档
├── MainWindow.h/cpp             # 主窗口类（对应CNetScanDemoDlg）
├── PrintThread.h/cpp            # 打印线程类（对应PrintThread静态函数）
├── ParameterManager.h/cpp       # 参数管理类（对应参数保存/加载函数）
├── MonitorThread.h/cpp          # 监控线程类（需要实现）
├── ImagePreviewWidget.h/cpp      # 图像预览控件（对应CPicScreen，需要实现）
├── PrintInfoTreeWidget.h/cpp     # 打印信息树形控件（对应CSWListTreeCtrl，需要实现）
├── PrintJobDialog.h/cpp          # 打印作业对话框（对应CPrtJobDlg，需要实现）
├── SysParamDialog.h/cpp          # 系统参数对话框（对应CSysParamDlg，需要实现）
└── main.cpp                      # 应用程序入口（需要实现）
```

## 使用说明

### 1. 环境要求

- Qt 5.15+ 或 Qt 6.x
- CMake 3.16+ 或 qmake
- Visual Studio 2019+ 或 MinGW
- Windows 平台（因为RYPrtCtler库是Windows专用）

### 2. 创建项目

#### 使用 CMake

```cmake
cmake_minimum_required(VERSION 3.16)
project(NetScanDemoQt)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 REQUIRED COMPONENTS Core Widgets)

# 添加源文件
set(SOURCES
    main.cpp
    MainWindow.cpp
    PrintThread.cpp
    ParameterManager.cpp
    # ... 其他源文件
)

set(HEADERS
    MainWindow.h
    PrintThread.h
    ParameterManager.h
    # ... 其他头文件
)

# 添加UI文件
set(UI_FORMS
    MainWindow.ui
    # ... 其他UI文件
)

# 创建可执行文件
qt5_wrap_cpp(MOC_SOURCES ${HEADERS})
qt5_wrap_ui(UI_SOURCES ${UI_FORMS})
qt5_add_resources(RESOURCES resources.qrc)

add_executable(${PROJECT_NAME}
    ${SOURCES}
    ${MOC_SOURCES}
    ${UI_SOURCES}
    ${RESOURCES}
)

target_link_libraries(${PROJECT_NAME}
    Qt5::Core
    Qt5::Widgets
    # 链接RYPrtCtler库
    ${CMAKE_SOURCE_DIR}/../lib/x64/Release/RYPrtCtler.lib
)

# 包含目录
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/../Inc
)
```

#### 使用 qmake

创建 `NetScanDemoQt.pro` 文件：

```qmake
QT += core widgets

TARGET = NetScanDemoQt
TEMPLATE = app

SOURCES += \
    main.cpp \
    MainWindow.cpp \
    PrintThread.cpp \
    ParameterManager.cpp

HEADERS += \
    MainWindow.h \
    PrintThread.h \
    ParameterManager.h

FORMS += \
    MainWindow.ui

INCLUDEPATH += $$PWD/../Inc

LIBS += -L$$PWD/../lib/x64/Release -lRYPrtCtler

CONFIG += c++17
```

### 3. 实现缺失的类

当前提供的示范代码中，以下类需要完整实现：

1. **MonitorThread**: 监控线程类
2. **ImagePreviewWidget**: 图像预览控件
3. **PrintInfoTreeWidget**: 打印信息树形控件
4. **PrintJobDialog**: 打印作业对话框
5. **SysParamDialog**: 系统参数对话框
6. **main.cpp**: 应用程序入口

### 4. 创建UI文件

使用 Qt Designer 创建 `.ui` 文件，设计界面布局。

### 5. 编译和运行

```bash
# 使用 CMake
mkdir build
cd build
cmake ..
cmake --build .

# 或使用 qmake
qmake NetScanDemoQt.pro
make
```

## 转换要点总结

### 1. 应用程序类
- `CWinApp` → `QApplication`
- `InitInstance()` → `main()` 函数

### 2. 对话框类
- `CDialogEx` → `QMainWindow` 或 `QDialog`
- 消息映射 → 信号槽
- `DoDataExchange()` → 手动数据同步

### 3. 线程管理
- 静态线程函数 → `QThread` 子类
- `AfxBeginThread()` → `QThread::start()`
- Windows消息 → Qt信号槽

### 4. 文件操作
- `CFile` → `QFile` + `QDataStream`
- `CString` → `QString`

### 5. 自定义控件
- `CStatic` → `QWidget`
- `OnPaint()` → `paintEvent()`
- `CPaintDC` → `QPainter`

## 注意事项

1. **兼容性**: RYPrtCtler库是Windows专用，项目只能在Windows平台运行
2. **线程安全**: 使用Qt的信号槽进行线程间通信
3. **内存管理**: Qt使用父子对象自动管理内存
4. **编码**: Qt使用UTF-8编码，与Windows API交互时注意转换

## 参考文档

- [Qt转换方案.md](Qt转换方案.md) - 详细的转换方案和代码示例
- [项目详细分析文档.md](../项目详细分析文档.md) - MFC版本的详细分析

## 许可证

本项目仅用于学习和参考目的。

