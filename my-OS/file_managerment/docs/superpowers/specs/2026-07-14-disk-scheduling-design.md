# 磁盘调度与文件管理模拟系统 — 设计文档

**日期**: 2026-07-14
**项目**: 操作系统小学期 — 任务五：文件管理
**框架**: Qt 6.11.1 (Widgets) + C++20

---

## 1. 概述

本系统模拟**磁盘调度过程**和**文件管理操作**，在同一个 Qt6 图形窗口中通过 QTabWidget 分页展示两个独立模块。

### 1.1 功能范围

| 模块 | 内容 |
|------|------|
| 磁盘调度 | FCFS、SSTF、SCAN、CSCAN、N-Step-SCAN 五种算法，随机生成磁道请求，展示寻道顺序/总距离/平均寻道长度，算法对比排序 |
| 文件管理 | 内存模拟目录树，支持 cd、dir、md、rd、edit、del、exit 七条命令 |

### 1.2 约束

- Qt6 路径: `E:\IDE\Qt\6.11.1\`
- CMake 构建，C++20 标准
- 与 `memory_managerment` 项目代码风格一致

---

## 2. 架构

### 2.1 分层架构

```
┌────────────────────────────────────────────────┐
│  UI 层（Qt6 Widgets）                          │
│  MainWindow  │  DiskWidget  │  FileWidget      │
├────────────────────────────────────────────────┤
│  业务逻辑层                                    │
│  DiskScheduler (五种算法)  │  FileSystem (目录树) │
├────────────────────────────────────────────────┤
│  数据层                                        │
│  DiskResult  │  FileNode                      │
└────────────────────────────────────────────────┘
```

- **UI 层不包含业务逻辑**，只负责参数收集和结果展示
- **业务逻辑层可独立测试**，不依赖 Qt 组件

### 2.2 文件结构

```
file_managerment/
├── CMakeLists.txt              # Qt6 构建配置
├── main.cpp                    # 入口 (QApplication + MainWindow)
├── main_window.h / .cpp        # 主窗口 + QTabWidget 分页
├── disk_widget.h / .cpp        # 磁盘调度页签 UI
├── disk_scheduler.h / .cpp     # 五种调度算法纯逻辑
├── file_widget.h / .cpp        # 文件管理页签 UI
├── file_system.h / .cpp        # 文件系统模拟（目录树 + 命令解析）
└── algorithm_compare.h / .cpp  # 算法对比辅助
```

**共 8 个模块、13 个文件**，每个模块单一职责。

---

## 3. 磁盘调度模块

### 3.1 数据结构

```cpp
struct DiskResult {
    std::vector<int> seek_sequence;  // 寻道访问顺序
    int total_distance;              // 总寻道距离
    double avg_distance;             // 平均寻道长度
    int direction;                   // 磁头移动方向: 1=向外(增加), 0=向内(减少)
};

class DiskScheduler {
public:
    void generateRequests(int count, int max_track);
    DiskResult FCFS(int start_track);
    DiskResult SSTF(int start_track);
    DiskResult SCAN(int start_track, int max_track);
    DiskResult CSCAN(int start_track, int max_track);
    DiskResult NStepSCAN(int start_track, int step_size);
    std::vector<int> requests;
};
```

### 3.2 五种算法

| 算法 | 策略 | 关键逻辑 |
|------|------|----------|
| **FCFS** | 先来先服务 | 按请求到达顺序依次服务，无优化 |
| **SSTF** | 最短寻道优先 | 每次从剩余请求中选距离当前磁头最近的，贪心策略，可能饥饿 |
| **SCAN** | 电梯扫描 | 从当前磁道沿一个方向移动到底再反向，初始方向由最近请求决定 |
| **CSCAN** | 循环扫描 | 单向移动到最大磁道后跳回 0 继续同向扫描，更均匀 |
| **N-Step-SCAN** | 分段扫描 | 请求分为 N 段（默认 2 段），每段内独立 SCAN |

### 3.3 流程图（FCFS / SSTF / SCAN / CSCAN / N-Step-SCAN）

**FCFS**:
```
请求队列: [R0, R1, R2, ..., R9]
当前磁头 → 依次访问 R0, R1, R2, ...
每次移动 = |当前位置 - 下一请求|
总距离 = sum(|Ri - Ri-1|)  (R-1 = 起始磁头)
```

**SSTF**:
```
初始磁头位置 = start_track
while 未访问请求非空:
    从未访问请求中选距离 current 最近的 Rk
    记录访问 Rk，移动距离 += |Rk - current|
    current = Rk，移除已访问
```

**SCAN**:
```
初始方向: 比较最近请求在 current 的哪一侧 → 决定向外(增加)或向内(减少)
while 该方向还有请求:
    选最近的一个，访问它
    移动到头后反向
反向扫描剩余请求
```

**CSCAN**:
```
while 请求非空:
    从 current 向外扫描到最大磁道号，访问途径请求
    若还有未访问请求，跳回最小磁道号，继续向外扫描
```

**N-Step-SCAN**:
```
将请求队列分为 floor(N/step_size) 段
对每一段独立执行 SCAN，段与段之间重置磁头位置
```

### 3.4 与参考代码的改进

| 问题 | 参考代码 | 本设计 |
|------|----------|--------|
| 全局变量 | `NAll`, `Best`, `Jage` 等 | 无全局状态，算法返回 `DiskResult` |
| 硬编码数组 | `int[10]` 固定 10 个请求 | `std::vector<int>` 动态大小 |
| I/O 耦合 | 算法内直接 `printf` | 算法纯逻辑，UI 层负责展示 |
| 代码重复 | 每个算法手动复制队列 | `requests` 成员统一存储，算法只读 |
| "向外/内"判断 | 只在 SCAN 打印 | 所有算法返回 `DiskResult.direction` |

### 3.5 UI 布局 (DiskWidget)

```
┌──────────────────────────────────────────────────┐
│  起始磁道号: [____]   最大磁道号: [____]          │
│  请求数量:   [____]   算法选择:   [⏷ FCFS  ]     │
│                [ 执行 ]  [ 对比全部 ]              │
├──────────────────────────────────────────────────┤
│  寻道顺序:  [表格/列表展示 seq + 每步距离]        │
│  方向:      向外 / 向内                          │
│  总距离:    1234                                 │
│  平均寻道长度: 123.40                            │
├──────────────────────────────────────────────────┤
│  算法对比 (仅"对比全部"时显示):                   │
│  排名  算法      总距离  平均长度                 │
│  1     SSTF      980     98.00                   │
│  2     SCAN      1100    110.00                  │
│  ...                                            │
└──────────────────────────────────────────────────┘
```

---

## 4. 文件管理模块

### 4.1 数据结构

```cpp
struct FileNode {
    QString name;
    bool is_directory;
    QString content;        // 仅文件使用
    FileNode* parent = nullptr;
    std::vector<std::unique_ptr<FileNode>> children;
};

class FileSystem {
public:
    FileSystem();  // 初始化根目录 / 和几个示例目录/文件
    QString execute(const QString& command);
    QString currentPath() const;
private:
    std::unique_ptr<FileNode> root_;
    FileNode* current_dir_;
    QString current_path_;
    // 七条命令的内部实现
    QString cmd_cd(const QString& arg);
    QString cmd_dir(const QString& arg);
    QString cmd_md(const QString& arg);
    QString cmd_rd(const QString& arg);
    QString cmd_edit(const QString& arg);
    QString cmd_del(const QString& arg);
};
```

### 4.2 命令语义

| 命令 | 格式 | 行为 |
|------|------|------|
| `cd <dir>` | 切换目录 | 在 children 中查找匹配的目录节点，更新 current_dir_ |
| `cd ..` | 返回上级 | current_dir_ = current_dir_->parent |
| `dir` | 列出当前目录 | 遍历 children，`<DIR>` 前缀标记目录，`<FILE>` 前缀标记文件 |
| `dir <subdir>` | 列出子目录 | 先定位 subdir 再列出 |
| `md <name>` | 创建目录 | 在同级 children 中添加 FileNode(name, true) |
| `rd <name>` | 删除空目录 | 仅当目标目录 children 为空时删除 |
| `edit <name>` | 新建空文件 | 在同级 children 中添加 FileNode(name, false) |
| `del <name>` | 删除文件 | 从 children 中移除，目录不可 del |
| `exit` | 退出 | 在 UI 层直接 `qApp->quit()` |

### 4.3 错误处理

- 目录不存在 → `"错误: 目录 'xxx' 不存在"`
- 非空目录删除 → `"错误: 目录 'xxx' 非空，无法删除"`
- 文件名冲突 → `"错误: 'xxx' 已存在"`

### 4.4 UI 布局 (FileWidget)

```
┌──────────────────────────────────────────────┐
│  当前路径: [/]  (QLineEdit 只读)              │
├─────────────────┬────────────────────────────┤
│  目录树          │  终端输出                   │
│  (QTreeWidget)   │  (QPlainTextEdit 只读)     │
│  ├─ home/       │                           │
│  ├─ etc/        │  > dir                    │
│  ├─ var/        │  <DIR> home/              │
│  └─ boot/       │  <DIR> etc/               │
│                 │  <DIR> var/               │
│                 │  <DIR> boot/              │
│                 │                           │
│                 │  >                        │
├─────────────────┴────────────────────────────┤
│  > [命令输入 QLineEdit                    ]   │
└──────────────────────────────────────────────┘
```

- 左侧目录树随命令执行自动刷新
- 右侧终端追加模式，历史命令可滚动
- 底部输入行按 Enter 执行命令

### 4.5 初始化文件系统

启动时预设以下结构：

```
/
├── home/
│   └── user/
├── etc/
├── var/
├── boot/
└── README (文件)
```

---

## 5. 主窗口布局

```
┌─────────────────────────────────────────────┐
│  磁盘调度与文件管理模拟系统                    │
├─────────────────────────────────────────────┤
│  [磁盘调度]  [文件管理]                       │  ← QTabWidget
├─────────────────────────────────────────────┤
│                                             │
│   当前活动页签的内容                          │
│                                             │
└─────────────────────────────────────────────┘
```

- `MainWindow` 窗口标题 "磁盘调度与文件管理模拟系统"
- 默认尺寸 900×600
- 两个页签各自独立，互不干扰

---

## 6. 构建配置

```cmake
cmake_minimum_required(VERSION 3.16)
project(file_managerment)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

# Qt6 路径
set(CMAKE_PREFIX_PATH "E:/IDE/Qt/6.11.1/mingw_64")
find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(file_managerment
    main.cpp
    main_window.h main_window.cpp
    disk_widget.h disk_widget.cpp
    disk_scheduler.h disk_scheduler.cpp
    file_widget.h file_widget.cpp
    file_system.h file_system.cpp
    algorithm_compare.h algorithm_compare.cpp
)

target_link_libraries(file_managerment PRIVATE Qt6::Widgets)
```

---

## 7. 测试策略

| 测试维度 | 方法 |
|----------|------|
| `DiskScheduler` 算法正确性 | 固定输入跑一遍，对比手工计算期望值 |
| `FileSystem` 命令正确性 | 执行命令序列，断言 current_path 和 children 状态 |
| UI 集成 | 手动运行，验证控件交互和数据展示 |
