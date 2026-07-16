# OS 模拟操作系统 — 四模块整合设计

## 概述

将 `device_managerment`、`file_managerment`、`memory_managerment`、`thread_managerment` 四个独立工程整合为一个完整的模拟操作系统，统一在 Qt6 GUI 下运行。**原四个目录不做任何修改**，所有文件复制到 `OS/` 子目录中。

## 目录结构

```
OS/
├── CMakeLists.txt                    # 顶层构建（Qt6 + C++20）
├── main.cpp                          # 唯一入口：创建 MainWindow
├── main_window.h/.cpp                # 统一主窗口（QTabWidget, 4 标签页）
│
├── device/                           # 设备管理 — 银行家算法（死锁避免）
│   ├── DeviceWidget.h/.cpp           # ★新建：Qt 包装控件
│   ├── i_banker.h                    # 复制
│   ├── banker_algorithm.h            # 复制
│   └── banker_algorithm.cpp          # 复制
│
├── file/                             # 文件管理 — 磁盘调度 + 文件系统
│   ├── disk_widget.h/.cpp            # 复制
│   ├── disk_scheduler.h/.cpp         # 复制
│   ├── file_widget.h/.cpp            # 复制
│   ├── file_system.h/.cpp            # 复制
│   ├── algorithm_compare.h/.cpp      # 复制
│   └── main_window.h/.cpp            # 复制（本模块内部窗口，不被顶层使用）
│
├── memory/                           # 内存管理 — 请求分页（FIFO / LRU）
│   ├── MemoryWidget.h/.cpp           # ★改名：原 MainWindow → MemoryWidget
│   ├── address_generator.h/.cpp      # 复制
│   ├── i_replacer.h                  # 复制
│   ├── fifo_replacer.h/.cpp          # 复制
│   ├── lru_replacer.h/.cpp           # 复制
│   └── page_table_manager.h/.cpp     # 复制
│
└── thread/                           # 进程管理 — 调度算法 + 生产者消费者
    ├── ThreadWidget.h/.cpp           # ★新建：Qt 包装控件
    ├── MyThread.h/.cpp               # 复制
    ├── Scheduler.h/.cpp              # 复制
    ├── SJFScheduler.h/.cpp           # 复制
    ├── HRRNScheduler.h/.cpp          # 复制
    ├── DynamicPriorityScheduler.h/.cpp # 复制
    └── ProducerConsumer.h/.cpp       # 复制
```

## 架构设计

### 主窗口 `MainWindow`

```
┌──────────────────────────────────────────────────┐
│  模拟操作系统 (Simulated OS)                       │
├──────────────────────────────────────────────────┤
│ [设备管理] [文件管理] [内存管理] [进程管理]          │
├──────────────────────────────────────────────────┤
│                                                    │
│  当前标签页的 Widget 内容                           │
│                                                    │
└──────────────────────────────────────────────────┘
```

- 继承 `QMainWindow`
- 中央控件为 `QTabWidget`，4 个标签页
- 每个标签页嵌入对应模块的主 Widget
- 窗口标题："模拟操作系统 (Simulated OS)"
- 初始大小：1000×700

### 各模块嵌入方式

| 模块 | 原 UI 形态 | 嵌入方式 |
|------|-----------|---------|
| device | 控制台 cin/cout | **新建 DeviceWidget**：QLineEdit 输入 → 调用 BankerAlgorithm → QTableWidget + QPlainTextEdit 输出 |
| file | Qt Widgets（DiskWidget + FileWidget） | 已有 `MainWindow` 含 QTabWidget（磁盘调度 + 文件系统两个子标签），直接嵌入或用其内部 Widget |
| memory | Qt Widgets（MainWindow） | 原 MainWindow 改名为 **MemoryWidget**，作为标签页嵌入 |
| thread | 控制台 cin/cout | **新建 ThreadWidget**：QComboBox 选算法 + QSpinBox/QLineEdit 输入参数 → 调用 Scheduler 子类 → QTableWidget + QPlainTextEdit 输出 |

### 控制台模块适配策略（device、thread）

两个模块原本用 `cin`/`cout` 交互，Scheduler 基类直接向 `std::cout` 打印。新建的 Qt 包装控件策略：

1. **DeviceWidget**：不依赖原控制台 I/O。直接调用 `BankerAlgorithm` 类的核心方法（如 `requestResources()`），通过信号/槽获取状态，在 QTableWidget 中展示资源分配矩阵。
2. **ThreadWidget**：由于 Scheduler 子类的 `schedule()` 和 `displayResults()` 直接写 `cout`，需要**重定向输出**。方案：
   - 用 `stringstream` 临时替换 `cout` 的 streambuf
   - 调度完成后将字符串内容显示在 QPlainTextEdit 中
   - 输入通过 QLineEdit + QSpinBox 收集，不走 `cin`

## 各模块细节

### 1. device — 设备管理

**源文件**：`i_banker.h`, `banker_algorithm.h`, `banker_algorithm.cpp`

**新建 DeviceWidget**：
- 左侧：参数配置区（进程数、资源类型数、各类资源总量、请求输入）
- 右侧：状态展示区
  - QTableWidget：当前分配矩阵、最大需求矩阵、可用资源向量
  - QPlainTextEdit：操作日志（请求是否安全）
- 按钮："初始化"、"发送请求"、"查看状态"

**接口适配**：BankerAlgorithm 类需要暴露以下方法（检查原作）：
- `setMaxMatrix()`, `setAllocationMatrix()`, `setAvailableResources()`
- `requestResources(int pid, vector<int> request)` → 返回安全/不安全
- `getSafeSequence()` → 返回安全序列

### 2. file — 文件管理

**源文件**：`disk_scheduler.h/.cpp`, `file_system.h/.cpp`, `disk_widget.h/.cpp`, `file_widget.h/.cpp`, `algorithm_compare.h/.cpp`

**嵌入方式**：原模块已有完整的 Qt6 GUI（MainWindow 内含 QTabWidget，分别放 DiskWidget 和 FileWidget）。方案：
- 原 `main_window.h/.cpp` 也复制过来，但不使用其 MainWindow 类
- 直接在顶层 MainWindow 的 "文件管理" 标签页中嵌入一个内嵌 QTabWidget，包含磁盘调度和文件系统两个子标签
- 或者直接把 DiskWidget 和 FileWidget 作为两个独立的顶层标签页

**选择**：保持 file 模块原有 2 个子标签结构，在顶层 "文件管理" 标签页内再放一个 QTabWidget。

### 3. memory — 内存管理

**源文件**：`i_replacer.h`, `fifo_replacer.h/.cpp`, `lru_replacer.h/.cpp`, `page_table_manager.h/.cpp`, `address_generator.h/.cpp`, `main_window.h/.cpp`

**处理**：
- 将原 `main_window.h/.cpp` 中的 `MainWindow` 类改名为 `MemoryWidget`（继承 QWidget）
- 去掉 QMainWindow 的菜单栏/状态栏，只保留中央控件内容
- 检查信号/槽连接是否完整

### 4. thread — 进程管理

**源文件**：`MyThread.h/.cpp`, `Scheduler.h/.cpp`, `SJFScheduler.h/.cpp`, `HRRNScheduler.h/.cpp`, `DynamicPriorityScheduler.h/.cpp`, `ProducerConsumer.h/.cpp`

**新建 ThreadWidget**：
- 左侧：配置区
  - QComboBox：选择算法（SJF / HRRN / 动态优先级）
  - QSpinBox：进程数量
  - 每个进程的参数输入行（名称、CPU 时间、优先级、到达时间）
  - QPushButton："开始调度"
- 右侧：结果区
  - QTableWidget：调度结果表（进程名、开始时间、完成时间、周转时间、带权周转时间）
  - QPlainTextEdit：调度过程日志（捕获 cout 输出）

**输出重定向**：
```cpp
// 调度前
ostringstream oss;
streambuf* old = cout.rdbuf(oss.rdbuf());

scheduler->schedule();
scheduler->displayResults();

// 恢复
cout.rdbuf(old);
logTextEdit->setPlainText(QString::fromUtf8(oss.str()));
```

## CMake 构建

```cmake
cmake_minimum_required(VERSION 3.16)
project(SimulatedOS LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(simulated_os
    main.cpp
    main_window.cpp

    # device
    device/DeviceWidget.cpp
    device/banker_algorithm.cpp

    # file
    file/disk_widget.cpp
    file/disk_scheduler.cpp
    file/file_widget.cpp
    file/file_system.cpp
    file/algorithm_compare.cpp
    file/main_window.cpp          # 内嵌窗口用

    # memory
    memory/MemoryWidget.cpp
    memory/address_generator.cpp
    memory/fifo_replacer.cpp
    memory/lru_replacer.cpp
    memory/page_table_manager.cpp

    # thread
    thread/ThreadWidget.cpp
    thread/MyThread.cpp
    thread/Scheduler.cpp
    thread/SJFScheduler.cpp
    thread/HRRNScheduler.cpp
    thread/DynamicPriorityScheduler.cpp
    thread/ProducerConsumer.cpp
)

target_include_directories(simulated_os PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(simulated_os PRIVATE Qt6::Widgets)
```

- **C++ 标准**：C++20（file 和 memory 模块用 C++20，向下兼容 C++17 的 device 和 thread）
- 统一用 `target_include_directories` 设置根目录为 include path，各模块头文件用 `device/xxx.h` 等形式引用
- 不加 `-fexec-charset=UTF-8`（Qt 原生支持 UTF-8，不再有控制台乱码问题）

## 文件复制清单

**从 device_managerment 复制→ OS/device/**：
- `i_banker.h`, `banker_algorithm.h`, `banker_algorithm.cpp`

**从 file_managerment 复制→ OS/file/**：
- `disk_widget.h/.cpp`, `disk_scheduler.h/.cpp`, `file_widget.h/.cpp`, `file_system.h/.cpp`, `algorithm_compare.h/.cpp`, `main_window.h/.cpp`

**从 memory_managerment 复制→ OS/memory/**：
- `main_window.h/.cpp`（复制后重命名类为 MemoryWidget）
- `address_generator.h/.cpp`, `i_replacer.h`, `fifo_replacer.h/.cpp`, `lru_replacer.h/.cpp`, `page_table_manager.h/.cpp`

**从 thread_managerment 复制→ OS/thread/**：
- `MyThread.h/.cpp`, `Scheduler.h/.cpp`, `SJFScheduler.h/.cpp`, `HRRNScheduler.h/.cpp`, `DynamicPriorityScheduler.h/.cpp`, `ProducerConsumer.h/.cpp`

## 新建文件清单

| 文件 | 说明 |
|------|------|
| `OS/CMakeLists.txt` | 顶层构建文件 |
| `OS/main.cpp` | 入口：创建 QApplication + MainWindow |
| `OS/main_window.h/.cpp` | 统一主窗口，QTabWidget 含 4 标签页 |
| `OS/device/DeviceWidget.h/.cpp` | 银行家算法 Qt 包装 |
| `OS/thread/ThreadWidget.h/.cpp` | 调度算法 Qt 包装 |

## 验证方式

1. `mkdir build && cd build && cmake .. -G "Ninja" && ninja` 编译成功
2. 运行 `simulated_os.exe`，4 个标签页正常显示
3. 设备管理：输入资源/进程配置，测试安全检查
4. 文件管理：磁盘调度 + 文件系统操作正常
5. 内存管理：生成地址序列 → 执行 FIFO/LRU 置换 → 显示缺页率
6. 进程管理：选择算法 → 输入进程 → 调度 → 显示结果表格
