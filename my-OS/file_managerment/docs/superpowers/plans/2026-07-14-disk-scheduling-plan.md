# 磁盘调度与文件管理模拟系统 — Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build a Qt6 Widgets application with two tabbed modules: disk scheduling simulation (FCFS/SSTF/SCAN/CSCAN/N-Step-SCAN) and file management simulation (cd/dir/md/rd/edit/del/exit).

**Architecture:** 三层分离 — UI 层 (Qt Widgets) 收集参数/展示结果，业务逻辑层 (DiskScheduler/FileSystem) 纯计算无 Qt 依赖，数据层 (DiskResult/FileNode) 传值对象。

**Tech Stack:** C++20, Qt 6.11.1 Widgets, CMake 3.16+, MinGW (mingw_64)

## Global Constraints

- Qt6 路径: `E:/IDE/Qt/6.11.1/mingw_64`
- C++20 标准, `CMAKE_AUTOMOC ON`
- 业务逻辑层零 Qt 依赖，只能用 STL
- UI 文件用 `#pragma once` 而非 include guards
- 命名风格与 `memory_managerment` 项目保持一致
- CMake 所有源文件必须在 Task 1 中一次性列出

---

### Task 1: CMakeLists.txt — 构建配置

**Files:**
- Modify: `file_managerment/CMakeLists.txt`

**Interfaces:**
- Produces: 构建目标 `file_managerment`，链接 `Qt6::Widgets`，包含全部 13 个源文件

- [ ] **Step 1: 写入完整的 CMakeLists.txt**

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

- [ ] **Step 2: 验证构建系统可用**

Run: `cd "f:/stdu-experiment/my-OS/file_managerment" && cmake -B cmake-build-debug -G "MinGW Makefiles"`
Expected: Configuring done, Generaring done (即使少文件也会成功，CMake 只在 link 时报错)

- [ ] **Step 3: Commit**

```bash
git add file_managerment/CMakeLists.txt
git commit -m "build: configure CMake with Qt6 for file_managerment project"
```

---

### Task 2: DiskScheduler — 五种磁盘调度算法（纯逻辑，无 Qt）

**Files:**
- Create: `file_managerment/disk_scheduler.h`
- Create: `file_managerment/disk_scheduler.cpp`

**Interfaces:**
- Produces:
  - `struct DiskResult { std::vector<int> seek_sequence; int total_distance; double avg_distance; int direction; };`
  - `class DiskScheduler { public: void generateRequests(int count, int max_track); DiskResult FCFS(int start_track); DiskResult SSTF(int start_track); DiskResult SCAN(int start_track, int max_track); DiskResult CSCAN(int start_track, int max_track); DiskResult NStepSCAN(int start_track, int step_size); std::vector<int> requests; };`

- [ ] **Step 1: 写入 disk_scheduler.h**

```cpp
#pragma once

#include <vector>

struct DiskResult {
    std::vector<int> seek_sequence;  // 寻道访问顺序
    int total_distance = 0;           // 总寻道距离
    double avg_distance = 0.0;        // 平均寻道长度
    int direction = 0;                // 0=向内(减少), 1=向外(增加)
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

- [ ] **Step 2: 写入 disk_scheduler.cpp — generateRequests + FCFS**

```cpp
#include "disk_scheduler.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>

void DiskScheduler::generateRequests(int count, int max_track) {
    requests.clear();
    requests.reserve(count);
    for (int i = 0; i < count; ++i) {
        requests.push_back(std::rand() % (max_track + 1));
    }
}

DiskResult DiskScheduler::FCFS(int start_track) {
    DiskResult result;
    int current = start_track;
    int total = 0;

    for (int r : requests) {
        int dist = std::abs(r - current);
        total += dist;
        result.seek_sequence.push_back(r);
        current = r;
    }

    result.total_distance = total;
    result.avg_distance = requests.empty() ? 0.0 : static_cast<double>(total) / requests.size();
    result.direction = 0;
    return result;
}
```

- [ ] **Step 3: 写入 SSTF 算法**

Append to `disk_scheduler.cpp`:

```cpp
DiskResult DiskScheduler::SSTF(int start_track) {
    DiskResult result;
    std::vector<bool> visited(requests.size(), false);
    int current = start_track;
    int total = 0;
    int n = static_cast<int>(requests.size());

    for (int i = 0; i < n; ++i) {
        int min_dist = 2147483647;  // INT_MAX
        int min_idx = -1;

        for (int j = 0; j < n; ++j) {
            if (visited[j]) continue;
            int dist = std::abs(requests[j] - current);
            if (dist < min_dist) {
                min_dist = dist;
                min_idx = j;
            }
        }

        if (min_idx == -1) break;

        total += min_dist;
        current = requests[min_idx];
        result.seek_sequence.push_back(current);
        visited[min_idx] = true;
    }

    result.total_distance = total;
    result.avg_distance = requests.empty() ? 0.0 : static_cast<double>(total) / requests.size();
    result.direction = 0;
    return result;
}
```

- [ ] **Step 4: 写入 SCAN 算法**

Append to `disk_scheduler.cpp`:

```cpp
DiskResult DiskScheduler::SCAN(int start_track, int max_track) {
    DiskResult result;
    std::vector<int> req = requests;
    std::sort(req.begin(), req.end());

    int current = start_track;
    int total = 0;

    // 确定初始方向：找最近的请求，若 >= start_track 则向外，否则向内
    int dir = 1; // 默认向外
    int nearest_dist = 2147483647;
    int nearest_idx = -1;
    for (int i = 0; i < static_cast<int>(req.size()); ++i) {
        int dist = std::abs(req[i] - current);
        if (dist < nearest_dist) {
            nearest_dist = dist;
            nearest_idx = i;
        }
    }
    if (nearest_idx >= 0 && req[nearest_idx] < current) {
        dir = 0; // 向内
    }

    // 按方向分组
    std::vector<int> outward, inward;
    for (int r : req) {
        if (r >= current)
            outward.push_back(r);
        else
            inward.push_back(r);
    }
    // 向外组升序，向内组降序
    std::sort(outward.begin(), outward.end());
    std::sort(inward.begin(), inward.end(), std::greater<int>());

    std::vector<int> order;
    if (dir == 1) {
        // 先向外
        for (int r : outward) order.push_back(r);
        for (int r : inward) order.push_back(r);
    } else {
        // 先向内
        for (int r : inward) order.push_back(r);
        for (int r : outward) order.push_back(r);
    }

    for (int r : order) {
        total += std::abs(r - current);
        current = r;
        result.seek_sequence.push_back(r);
    }

    result.total_distance = total;
    result.avg_distance = requests.empty() ? 0.0 : static_cast<double>(total) / requests.size();
    result.direction = dir;
    return result;
}
```

- [ ] **Step 5: 写入 CSCAN 算法**

Append to `disk_scheduler.cpp`:

```cpp
DiskResult DiskScheduler::CSCAN(int start_track, int max_track) {
    DiskResult result;
    std::vector<int> req = requests;
    std::sort(req.begin(), req.end());

    int current = start_track;
    int total = 0;

    // 分组：>= current 和 < current
    std::vector<int> right, left;
    for (int r : req) {
        if (r >= current)
            right.push_back(r);
        else
            left.push_back(r);
    }

    // 先处理右侧，然后跳回最小再处理左侧
    for (int r : right) {
        total += std::abs(r - current);
        current = r;
        result.seek_sequence.push_back(r);
    }

    if (!left.empty()) {
        // 跳到最小磁道号（0 或 left 的第一个）
        int low = left.front();
        total += std::abs(current - low);
        current = low;

        for (int r : left) {
            total += std::abs(r - current);
            current = r;
            result.seek_sequence.push_back(r);
        }
    }

    result.total_distance = total;
    result.avg_distance = requests.empty() ? 0.0 : static_cast<double>(total) / requests.size();
    result.direction = 1; // CSCAN 始终向外
    return result;
}
```

- [ ] **Step 6: 写入 NStepSCAN 算法**

Append to `disk_scheduler.cpp`:

```cpp
DiskResult DiskScheduler::NStepSCAN(int start_track, int step_size) {
    DiskResult result;
    int total = 0;
    int current = start_track;
    int n = static_cast<int>(requests.size());

    if (step_size <= 0) step_size = 1;

    for (int base = 0; base < n; base += step_size) {
        int end = std::min(base + step_size, n);

        // 取出当前段的请求
        std::vector<int> segment(requests.begin() + base, requests.begin() + end);
        std::sort(segment.begin(), segment.end());

        // 确定方向（与 SCAN 一致）
        int dir = 1;
        if (!segment.empty() && segment[0] < current) {
            // 有比 current 小的 → 检查最近的
            int nearest_dist = 2147483647;
            int nearest_idx = -1;
            for (int i = 0; i < static_cast<int>(segment.size()); ++i) {
                int dist = std::abs(segment[i] - current);
                if (dist < nearest_dist) {
                    nearest_dist = dist;
                    nearest_idx = i;
                }
            }
            if (nearest_idx >= 0 && segment[nearest_idx] < current) {
                dir = 0;
            }
        }

        std::vector<int> outward, inward;
        for (int r : segment) {
            if (r >= current)
                outward.push_back(r);
            else
                inward.push_back(r);
        }
        std::sort(outward.begin(), outward.end());
        std::sort(inward.begin(), inward.end(), std::greater<int>());

        std::vector<int> order;
        if (dir == 1) {
            for (int r : outward) order.push_back(r);
            for (int r : inward) order.push_back(r);
        } else {
            for (int r : inward) order.push_back(r);
            for (int r : outward) order.push_back(r);
        }

        for (int r : order) {
            total += std::abs(r - current);
            current = r;
            result.seek_sequence.push_back(r);
        }
    }

    result.total_distance = total;
    result.avg_distance = requests.empty() ? 0.0 : static_cast<double>(total) / requests.size();
    result.direction = 0;
    return result;
}
```

- [ ] **Step 7: 编译验证 disk_scheduler（无链接，仅检查语法）**

Run:
```bash
cd "f:/stdu-experiment/my-OS/file_managerment" && g++ -std=c++20 -c disk_scheduler.cpp -o disk_scheduler.o
```
Expected: Compilation successful, produces `disk_scheduler.o`

- [ ] **Step 8: Cleanup and commit**

```bash
rm -f disk_scheduler.o
git add file_managerment/disk_scheduler.h file_managerment/disk_scheduler.cpp
git commit -m "feat: add DiskScheduler with FCFS/SSTF/SCAN/CSCAN/NStepSCAN algorithms"
```

---

### Task 3: AlgorithmCompare — 算法对比辅助

**Files:**
- Create: `file_managerment/algorithm_compare.h`
- Create: `file_managerment/algorithm_compare.cpp`

**Interfaces:**
- Produces:
  - `struct CompareEntry { int rank; QString name; int total_distance; double avg_distance; };`
  - `std::vector<CompareEntry> compareAll(DiskScheduler& scheduler, int start_track, int max_track);`

- [ ] **Step 1: 写入 algorithm_compare.h**

```cpp
#pragma once

#include <QString>
#include <vector>

struct CompareEntry {
    int rank;
    QString name;
    int total_distance;
    double avg_distance;
};

// 前向声明，避免引入 Qt 依赖的业务层头文件
class DiskScheduler;

/// 对全部 5 种算法执行并返回按 total_distance 升序排列的结果
std::vector<CompareEntry> compareAll(DiskScheduler& scheduler,
                                     int start_track,
                                     int max_track);
```

- [ ] **Step 2: 写入 algorithm_compare.cpp**

```cpp
#include "algorithm_compare.h"
#include "disk_scheduler.h"
#include <algorithm>

std::vector<CompareEntry> compareAll(DiskScheduler& scheduler,
                                     int start_track,
                                     int max_track) {
    std::vector<CompareEntry> entries;

    auto add = [&](const QString& name, const DiskResult& r) {
        entries.push_back({0, name, r.total_distance, r.avg_distance});
    };

    add("FCFS",         scheduler.FCFS(start_track));
    add("SSTF",         scheduler.SSTF(start_track));
    add("SCAN",         scheduler.SCAN(start_track, max_track));
    add("CSCAN",        scheduler.CSCAN(start_track, max_track));
    add("N-Step-SCAN",  scheduler.NStepSCAN(start_track, 5));

    // 按 total_distance 升序排列
    std::sort(entries.begin(), entries.end(),
              [](const CompareEntry& a, const CompareEntry& b) {
                  return a.total_distance < b.total_distance;
              });

    for (int i = 0; i < static_cast<int>(entries.size()); ++i) {
        entries[i].rank = i + 1;
    }

    return entries;
}
```

- [ ] **Step 3: Commit**

```bash
git add file_managerment/algorithm_compare.h file_managerment/algorithm_compare.cpp
git commit -m "feat: add AlgorithmCompare — run all 5 algorithms and rank by total distance"
```

---

### Task 4: FileSystem — 文件系统模拟（纯逻辑，无 Qt 依赖）

**Files:**
- Create: `file_managerment/file_system.h`
- Create: `file_managerment/file_system.cpp`

**Interfaces:**
- Produces:
  - `struct FileNode { QString name; bool is_directory; QString content; FileNode* parent; std::vector<std::unique_ptr<FileNode>> children; };`
  - `class FileSystem { public: FileSystem(); QString execute(const QString& command); QString currentPath() const; FileNode* rootNode() const; FileNode* currentNode() const; private: ... };`

- [ ] **Step 1: 写入 file_system.h**

```cpp
#pragma once

#include <QString>
#include <memory>
#include <vector>

struct FileNode {
    QString name;
    bool is_directory = false;
    QString content;
    FileNode* parent = nullptr;
    std::vector<std::unique_ptr<FileNode>> children;
};

class FileSystem {
public:
    FileSystem();

    /// 执行一条命令，返回输出文本
    QString execute(const QString& command);

    /// 当前工作目录路径
    QString currentPath() const;

    /// 获取根节点（供 UI 层构建目录树）
    FileNode* rootNode() const;

    /// 获取当前工作目录节点
    FileNode* currentNode() const;

private:
    std::unique_ptr<FileNode> root_;
    FileNode* current_dir_;

    void initFileTree();

    // 命令实现
    QString cmd_cd(const QString& arg);
    QString cmd_dir(const QString& arg);
    QString cmd_md(const QString& arg);
    QString cmd_rd(const QString& arg);
    QString cmd_edit(const QString& arg);
    QString cmd_del(const QString& arg);

    // 辅助：在节点的 children 中查找目录
    FileNode* findDir(FileNode* parent, const QString& name) const;
    // 辅助：在节点的 children 中查找文件
    FileNode* findFile(FileNode* parent, const QString& name) const;
    // 辅助：构建路径字符串
    QString buildPath(FileNode* node) const;
};
```

- [ ] **Step 2: 写入 file_system.cpp — 构造 + initFileTree + 命令解析**

```cpp
#include "file_system.h"
#include <QStringList>

FileSystem::FileSystem()
    : root_(std::make_unique<FileNode>()), current_dir_(nullptr) {
    root_->name = "/";
    root_->is_directory = true;
    root_->parent = nullptr;
    current_dir_ = root_.get();
    initFileTree();
}

void FileSystem::initFileTree() {
    // /
    // ├── home/
    // │   └── user/
    // ├── etc/
    // ├── var/
    // ├── boot/
    // └── README (文件)

    auto addDir = [this](FileNode* parent, const QString& name) -> FileNode* {
        auto node = std::make_unique<FileNode>();
        node->name = name;
        node->is_directory = true;
        node->parent = parent;
        FileNode* ptr = node.get();
        parent->children.push_back(std::move(node));
        return ptr;
    };

    auto addFile = [this](FileNode* parent, const QString& name, const QString& content = QString()) {
        auto node = std::make_unique<FileNode>();
        node->name = name;
        node->is_directory = false;
        node->content = content;
        node->parent = parent;
        parent->children.push_back(std::move(node));
    };

    FileNode* home = addDir(root_.get(), "home");
    addDir(home, "user");
    addDir(root_.get(), "etc");
    addDir(root_.get(), "var");
    addDir(root_.get(), "boot");
    addFile(root_.get(), "README", "Welcome to the simulated file system.");
}

QString FileSystem::execute(const QString& command) {
    QString trimmed = command.trimmed();
    if (trimmed.isEmpty()) return QString();

    // 按空格分割命令和参数
    int space = trimmed.indexOf(' ');
    QString cmd = (space < 0) ? trimmed.toLower() : trimmed.left(space).toLower();
    QString arg = (space < 0) ? QString() : trimmed.mid(space + 1).trimmed();

    if (cmd == "cd")       return cmd_cd(arg);
    if (cmd == "dir")      return cmd_dir(arg);
    if (cmd == "md")       return cmd_md(arg);
    if (cmd == "rd")       return cmd_rd(arg);
    if (cmd == "edit")     return cmd_edit(arg);
    if (cmd == "del")      return cmd_del(arg);

    return QString("错误: 未知命令 '%1'").arg(cmd);
}

QString FileSystem::currentPath() const {
    return buildPath(current_dir_);
}

FileNode* FileSystem::rootNode() const {
    return root_.get();
}

FileNode* FileSystem::currentNode() const {
    return current_dir_;
}
```

- [ ] **Step 3: 写入辅助方法**

Append to `file_system.cpp`:

```cpp
FileNode* FileSystem::findDir(FileNode* parent, const QString& name) const {
    for (auto& child : parent->children) {
        if (child->is_directory && child->name == name)
            return child.get();
    }
    return nullptr;
}

FileNode* FileSystem::findFile(FileNode* parent, const QString& name) const {
    for (auto& child : parent->children) {
        if (!child->is_directory && child->name == name)
            return child.get();
    }
    return nullptr;
}

QString FileSystem::buildPath(FileNode* node) const {
    if (node == root_.get()) return "/";
    QString path;
    FileNode* cur = node;
    while (cur && cur != root_.get()) {
        path = "/" + cur->name + path;
        cur = cur->parent;
    }
    return path.isEmpty() ? "/" : path;
}
```

- [ ] **Step 4: 写入 cd 命令**

Append to `file_system.cpp`:

```cpp
QString FileSystem::cmd_cd(const QString& arg) {
    if (arg.isEmpty() || arg == "/") {
        current_dir_ = root_.get();
        return QString("当前路径: %1").arg(currentPath());
    }
    if (arg == "..") {
        if (current_dir_->parent) {
            current_dir_ = current_dir_->parent;
        }
        return QString("当前路径: %1").arg(currentPath());
    }

    FileNode* target = findDir(current_dir_, arg);
    if (target) {
        current_dir_ = target;
        return QString("当前路径: %1").arg(currentPath());
    }
    return QString("错误: 目录 '%1' 不存在").arg(arg);
}
```

- [ ] **Step 5: 写入 dir 命令**

Append to `file_system.cpp`:

```cpp
QString FileSystem::cmd_dir(const QString& arg) {
    FileNode* target = current_dir_;
    if (!arg.isEmpty()) {
        FileNode* sub = findDir(current_dir_, arg);
        if (sub) target = sub;
        else return QString("错误: 目录 '%1' 不存在").arg(arg);
    }

    QString out;
    out += QString("目录: %1\n").arg(buildPath(target));
    for (auto& child : target->children) {
        if (child->is_directory)
            out += QString("  <DIR>   %1/\n").arg(child->name);
        else
            out += QString("  <FILE>  %1\n").arg(child->name);
    }
    if (target->children.empty()) {
        out += "  (空)\n";
    }
    // 去掉末尾换行
    if (out.endsWith('\n')) out.chop(1);
    return out;
}
```

- [ ] **Step 6: 写入 md / rd 命令**

Append to `file_system.cpp`:

```cpp
QString FileSystem::cmd_md(const QString& arg) {
    if (arg.isEmpty()) return "错误: 请指定目录名";

    if (findDir(current_dir_, arg)) {
        return QString("错误: '%1' 已存在").arg(arg);
    }

    auto node = std::make_unique<FileNode>();
    node->name = arg;
    node->is_directory = true;
    node->parent = current_dir_;
    current_dir_->children.push_back(std::move(node));
    return QString("目录 '%1' 创建成功").arg(arg);
}

QString FileSystem::cmd_rd(const QString& arg) {
    if (arg.isEmpty()) return "错误: 请指定目录名";

    FileNode* target = findDir(current_dir_, arg);
    if (!target) {
        return QString("错误: 目录 '%1' 不存在").arg(arg);
    }
    if (!target->children.empty()) {
        return QString("错误: 目录 '%1' 非空，无法删除").arg(arg);
    }

    // 从 parent 的 children 中移除
    auto& siblings = current_dir_->children;
    for (auto it = siblings.begin(); it != siblings.end(); ++it) {
        if (it->get() == target) {
            siblings.erase(it);
            break;
        }
    }
    return QString("目录 '%1' 已删除").arg(arg);
}
```

- [ ] **Step 7: 写入 edit / del 命令**

Append to `file_system.cpp`:

```cpp
QString FileSystem::cmd_edit(const QString& arg) {
    if (arg.isEmpty()) return "错误: 请指定文件名";

    if (findFile(current_dir_, arg)) {
        return QString("错误: '%1' 已存在").arg(arg);
    }

    auto node = std::make_unique<FileNode>();
    node->name = arg;
    node->is_directory = false;
    node->parent = current_dir_;
    current_dir_->children.push_back(std::move(node));
    return QString("文件 '%1' 创建成功").arg(arg);
}

QString FileSystem::cmd_del(const QString& arg) {
    if (arg.isEmpty()) return "错误: 请指定文件名";

    FileNode* target = findFile(current_dir_, arg);
    if (!target) {
        return QString("错误: 文件 '%1' 不存在").arg(arg);
    }

    auto& siblings = current_dir_->children;
    for (auto it = siblings.begin(); it != siblings.end(); ++it) {
        if (it->get() == target) {
            siblings.erase(it);
            break;
        }
    }
    return QString("文件 '%1' 已删除").arg(arg);
}
```

- [ ] **Step 8: 编译验证**

Run:
```bash
cd "f:/stdu-experiment/my-OS/file_managerment" && g++ -std=c++20 -c file_system.cpp -o file_system.o -I "E:/IDE/Qt/6.11.1/mingw_64/include" -I "E:/IDE/Qt/6.11.1/mingw_64/include/QtCore"
```
Expected: Compilation successful

- [ ] **Step 9: Cleanup and commit**

```bash
rm -f file_system.o
git add file_managerment/file_system.h file_managerment/file_system.cpp
git commit -m "feat: add FileSystem with cd/dir/md/rd/edit/del command simulation"
```

---

### Task 5: DiskWidget — 磁盘调度 UI

**Files:**
- Create: `file_managerment/disk_widget.h`
- Create: `file_managerment/disk_widget.cpp`

**Interfaces:**
- Consumes: `DiskScheduler` (disk_scheduler.h), `AlgorithmCompare::compareAll` (algorithm_compare.h)
- Produces: `class DiskWidget : public QWidget` — 完整磁盘调度页签

- [ ] **Step 1: 写入 disk_widget.h**

```cpp
#pragma once

#include <QWidget>

class QSpinBox;
class QComboBox;
class QPushButton;
class QTableWidget;
class QLabel;

class DiskScheduler;

class DiskWidget : public QWidget {
    Q_OBJECT
public:
    explicit DiskWidget(QWidget* parent = nullptr);

private slots:
    void onExecute();
    void onCompareAll();

private:
    void setupUI();

    QSpinBox* start_track_spin_;
    QSpinBox* max_track_spin_;
    QSpinBox* request_count_spin_;
    QComboBox* algorithm_combo_;
    QPushButton* execute_btn_;
    QPushButton* compare_btn_;
    QTableWidget* result_table_;
    QLabel* direction_label_;
    QLabel* total_label_;
    QLabel* avg_label_;
    QTableWidget* compare_table_;

    DiskScheduler scheduler_;
};
```

- [ ] **Step 2: 写入 disk_widget.cpp — setupUI**

```cpp
#include "disk_widget.h"
#include "disk_scheduler.h"
#include "algorithm_compare.h"

#include <QComboBox>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <ctime>

DiskWidget::DiskWidget(QWidget* parent)
    : QWidget(parent) {
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    setupUI();
}

void DiskWidget::setupUI() {
    auto* main_layout = new QVBoxLayout(this);

    // ── 参数设置 ──
    auto* param_group = new QGroupBox("参数设置", this);
    auto* param_layout = new QHBoxLayout(param_group);

    param_layout->addWidget(new QLabel("起始磁道号:", this));
    start_track_spin_ = new QSpinBox(this);
    start_track_spin_->setRange(0, 65535);
    start_track_spin_->setValue(100);
    param_layout->addWidget(start_track_spin_);

    param_layout->addWidget(new QLabel("最大磁道号:", this));
    max_track_spin_ = new QSpinBox(this);
    max_track_spin_->setRange(1, 65535);
    max_track_spin_->setValue(200);
    param_layout->addWidget(max_track_spin_);

    param_layout->addWidget(new QLabel("请求数量:", this));
    request_count_spin_ = new QSpinBox(this);
    request_count_spin_->setRange(1, 100);
    request_count_spin_->setValue(10);
    param_layout->addWidget(request_count_spin_);

    param_layout->addWidget(new QLabel("算法:", this));
    algorithm_combo_ = new QComboBox(this);
    algorithm_combo_->addItems({"FCFS", "SSTF", "SCAN", "CSCAN", "N-Step-SCAN"});
    param_layout->addWidget(algorithm_combo_);

    execute_btn_ = new QPushButton("执行", this);
    param_layout->addWidget(execute_btn_);

    compare_btn_ = new QPushButton("对比全部", this);
    param_layout->addWidget(compare_btn_);

    main_layout->addWidget(param_group);

    // ── 寻道结果表格 ──
    result_table_ = new QTableWidget(this);
    result_table_->setColumnCount(3);
    result_table_->setHorizontalHeaderLabels({"步数", "磁道号", "移动距离"});
    result_table_->horizontalHeader()->setStretchLastSection(true);
    result_table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    main_layout->addWidget(result_table_);

    // ── 统计信息 ──
    auto* stats_layout = new QHBoxLayout();
    direction_label_ = new QLabel("方向: --", this);
    total_label_     = new QLabel("总距离: --", this);
    avg_label_       = new QLabel("平均寻道长度: --", this);
    stats_layout->addWidget(direction_label_);
    stats_layout->addWidget(total_label_);
    stats_layout->addWidget(avg_label_);
    stats_layout->addStretch();
    main_layout->addLayout(stats_layout);

    // ── 算法对比表格 ──
    compare_table_ = new QTableWidget(this);
    compare_table_->setColumnCount(4);
    compare_table_->setHorizontalHeaderLabels({"排名", "算法", "总距离", "平均长度"});
    compare_table_->horizontalHeader()->setStretchLastSection(true);
    compare_table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    compare_table_->setMaximumHeight(200);
    main_layout->addWidget(compare_table_);

    // ── 连接信号 ──
    connect(execute_btn_, &QPushButton::clicked, this, &DiskWidget::onExecute);
    connect(compare_btn_, &QPushButton::clicked, this, &DiskWidget::onCompareAll);
}
```

- [ ] **Step 3: 写入 onExecute 槽函数**

Append to `disk_widget.cpp`:

```cpp
void DiskWidget::onExecute() {
    int start   = start_track_spin_->value();
    int max_trk = max_track_spin_->value();
    int count   = request_count_spin_->value();
    int algo    = algorithm_combo_->currentIndex();

    scheduler_.generateRequests(count, max_trk);

    DiskResult result;
    switch (algo) {
        case 0: result = scheduler_.FCFS(start);            break;
        case 1: result = scheduler_.SSTF(start);             break;
        case 2: result = scheduler_.SCAN(start, max_trk);    break;
        case 3: result = scheduler_.CSCAN(start, max_trk);   break;
        case 4: result = scheduler_.NStepSCAN(start, 5);     break;
    }

    // 填充寻道顺序表
    result_table_->setRowCount(static_cast<int>(result.seek_sequence.size()));
    int prev = start;
    for (int i = 0; i < static_cast<int>(result.seek_sequence.size()); ++i) {
        int track = result.seek_sequence[i];
        int dist = std::abs(track - prev);
        result_table_->setItem(i, 0, new QTableWidgetItem(QString::number(i + 1)));
        result_table_->setItem(i, 1, new QTableWidgetItem(QString::number(track)));
        result_table_->setItem(i, 2, new QTableWidgetItem(QString::number(dist)));
        prev = track;
    }

    // 更新统计
    direction_label_->setText(QString("方向: %1").arg(result.direction ? "向外" : "向内"));
    total_label_->setText(QString("总距离: %1").arg(result.total_distance));
    avg_label_->setText(QString("平均寻道长度: %1").arg(result.avg_distance, 0, 'f', 2));
}
```

- [ ] **Step 4: 写入 onCompareAll 槽函数**

Append to `disk_widget.cpp`:

```cpp
void DiskWidget::onCompareAll() {
    int start   = start_track_spin_->value();
    int max_trk = max_track_spin_->value();
    int count   = request_count_spin_->value();

    scheduler_.generateRequests(count, max_trk);

    auto entries = compareAll(scheduler_, start, max_trk);

    compare_table_->setRowCount(static_cast<int>(entries.size()));
    for (int i = 0; i < static_cast<int>(entries.size()); ++i) {
        compare_table_->setItem(i, 0, new QTableWidgetItem(QString::number(entries[i].rank)));
        compare_table_->setItem(i, 1, new QTableWidgetItem(entries[i].name));
        compare_table_->setItem(i, 2, new QTableWidgetItem(QString::number(entries[i].total_distance)));
        compare_table_->setItem(i, 3, new QTableWidgetItem(
            QString::number(entries[i].avg_distance, 'f', 2)));
    }

    // 同时执行当前选中的算法展示在上方
    onExecute();
}
```

- [ ] **Step 5: Commit**

```bash
git add file_managerment/disk_widget.h file_managerment/disk_widget.cpp
git commit -m "feat: add DiskWidget UI — parameter controls, seek sequence table, compare all view"
```

---

### Task 6: FileWidget — 文件管理 UI

**Files:**
- Create: `file_managerment/file_widget.h`
- Create: `file_managerment/file_widget.cpp`

**Interfaces:**
- Consumes: `FileSystem` (file_system.h)
- Produces: `class FileWidget : public QWidget` — 完整文件管理页签

- [ ] **Step 1: 写入 file_widget.h**

```cpp
#pragma once

#include <QWidget>

class QLineEdit;
class QTreeWidget;
class QPlainTextEdit;

class FileSystem;

class FileWidget : public QWidget {
    Q_OBJECT
public:
    explicit FileWidget(QWidget* parent = nullptr);

private slots:
    void onCommandEntered();

private:
    void setupUI();
    void refreshTree();
    void appendOutput(const QString& text);
    void buildTreeNode(QTreeWidgetItem* parent_item, class FileNode* node);

    QLineEdit* path_display_;
    QTreeWidget* tree_widget_;
    QPlainTextEdit* terminal_;
    QLineEdit* command_input_;

    FileSystem fs_;
};
```

- [ ] **Step 2: 写入 file_widget.cpp — setupUI**

```cpp
#include "file_widget.h"
#include "file_system.h"

#include <QApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QSplitter>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVBoxLayout>

FileWidget::FileWidget(QWidget* parent)
    : QWidget(parent) {
    setupUI();
    refreshTree();
    appendOutput("模拟文件系统已启动。输入 'dir' 查看根目录内容。");
}

void FileWidget::setupUI() {
    auto* main_layout = new QVBoxLayout(this);

    // ── 当前路径 ──
    auto* path_layout = new QHBoxLayout();
    path_layout->addWidget(new QLabel("当前路径:", this));
    path_display_ = new QLineEdit(this);
    path_display_->setReadOnly(true);
    path_display_->setText(fs_.currentPath());
    path_layout->addWidget(path_display_);
    main_layout->addLayout(path_layout);

    // ── 左右分栏: 目录树 | 终端 ──
    auto* splitter = new QSplitter(Qt::Horizontal, this);

    tree_widget_ = new QTreeWidget(this);
    tree_widget_->setHeaderLabel("目录结构");
    splitter->addWidget(tree_widget_);

    terminal_ = new QPlainTextEdit(this);
    terminal_->setReadOnly(true);
    splitter->addWidget(terminal_);

    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 2);
    main_layout->addWidget(splitter);

    // ── 命令输入 ──
    auto* cmd_layout = new QHBoxLayout();
    cmd_layout->addWidget(new QLabel(">", this));
    command_input_ = new QLineEdit(this);
    command_input_->setPlaceholderText("输入命令 (cd/dir/md/rd/edit/del/exit)");
    cmd_layout->addWidget(command_input_);
    main_layout->addLayout(cmd_layout);

    connect(command_input_, &QLineEdit::returnPressed,
            this, &FileWidget::onCommandEntered);
}
```

- [ ] **Step 3: 写入槽函数 + 辅助方法**

Append to `file_widget.cpp`:

```cpp
void FileWidget::onCommandEntered() {
    QString cmd = command_input_->text().trimmed();
    if (cmd.isEmpty()) return;

    // exit 特殊处理
    if (cmd.toLower() == "exit") {
        qApp->quit();
        return;
    }

    appendOutput("> " + cmd);

    QString output = fs_.execute(cmd);
    if (!output.isEmpty()) {
        appendOutput(output);
    }

    // 更新视图
    path_display_->setText(fs_.currentPath());
    refreshTree();
    command_input_->clear();
}

void FileWidget::refreshTree() {
    tree_widget_->clear();
    FileNode* root = fs_.rootNode();
    if (!root) return;

    auto* root_item = new QTreeWidgetItem(tree_widget_);
    root_item->setText(0, "/");
    root_item->setExpanded(true);

    for (auto& child : root->children) {
        buildTreeNode(root_item, child.get());
    }

    // 展开到当前工作目录
    // 先构建完整路径再展开
    tree_widget_->expandAll();  // 简单起见全部展开
}

void FileWidget::buildTreeNode(QTreeWidgetItem* parent_item, FileNode* node) {
    auto* item = new QTreeWidgetItem(parent_item);
    QString label = node->is_directory ? node->name + "/" : node->name;
    item->setText(0, label);

    for (auto& child : node->children) {
        buildTreeNode(item, child.get());
    }
}

void FileWidget::appendOutput(const QString& text) {
    terminal_->appendPlainText(text);
}
```

- [ ] **Step 4: Commit**

```bash
git add file_managerment/file_widget.h file_managerment/file_widget.cpp
git commit -m "feat: add FileWidget UI — directory tree, terminal output, command input"
```

---

### Task 7: MainWindow — 主窗口 + QTabWidget

**Files:**
- Create: `file_managerment/main_window.h`
- Create: `file_managerment/main_window.cpp`

**Interfaces:**
- Consumes: `DiskWidget`, `FileWidget`
- Produces: `class MainWindow : public QMainWindow`

- [ ] **Step 1: 写入 main_window.h**

```cpp
#pragma once

#include <QMainWindow>

class QTabWidget;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);

private:
    QTabWidget* tab_widget_;
};
```

- [ ] **Step 2: 写入 main_window.cpp**

```cpp
#include "main_window.h"
#include "disk_widget.h"
#include "file_widget.h"

#include <QTabWidget>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent) {
    setWindowTitle("磁盘调度与文件管理模拟系统");
    resize(900, 600);

    tab_widget_ = new QTabWidget(this);
    tab_widget_->addTab(new DiskWidget(this), "磁盘调度");
    tab_widget_->addTab(new FileWidget(this), "文件管理");

    setCentralWidget(tab_widget_);
}
```

- [ ] **Step 3: Commit**

```bash
git add file_managerment/main_window.h file_managerment/main_window.cpp
git commit -m "feat: add MainWindow with QTabWidget hosting disk and file tabs"
```

---

### Task 8: main.cpp — 入口

**Files:**
- Modify: `file_managerment/main.cpp`

**Interfaces:**
- Consumes: `MainWindow`, `QApplication`

- [ ] **Step 1: 写入 main.cpp**

当前 `main.cpp` 为空文件，写入：

```cpp
/**
 * main.cpp — 磁盘调度与文件管理模拟程序入口
 *
 * 操作系统小学期实验：文件管理
 * 支持五种磁盘调度算法模拟 + 文件管理命令模拟
 * 基于 Qt6 Widgets 实现图形窗口界面
 */

#include <QApplication>
#include "main_window.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("file-management");

    MainWindow window;
    window.show();

    return app.exec();
}
```

- [ ] **Step 2: 完整构建**

Run:
```bash
cd "f:/stdu-experiment/my-OS/file_managerment" && cmake -B cmake-build-debug -G "MinGW Makefiles" -DCMAKE_PREFIX_PATH="E:/IDE/Qt/6.11.1/mingw_64" && cmake --build cmake-build-debug
```
Expected: Build succeeds with no errors

- [ ] **Step 3: 运行验证**

Run:
```bash
"f:/stdu-experiment/my-OS/file_managerment/cmake-build-debug/file_managerment.exe"
```
Expected: 窗口启动，两个页签可切换，磁盘调度可执行算法并展示结果，文件管理可输入命令

- [ ] **Step 4: Commit**

```bash
git add file_managerment/main.cpp
git commit -m "feat: add main entry point for file_managerment application"
```

---

## 依赖图

```
Task 1 (CMake)
  └─► Task 2 (DiskScheduler) ──┐
  └─► Task 3 (AlgorithmCompare) ─┤
  └─► Task 4 (FileSystem) ───────┤
          ├──► Task 5 (DiskWidget) ──┐
          ├──► Task 6 (FileWidget) ──┤
                  ├──► Task 7 (MainWindow) ──► Task 8 (main.cpp)
```

Tasks 2/3/4 可并行编写；Task 5 依赖 2+3；Task 6 依赖 4；Task 7 依赖 5+6；Task 8 依赖 7。
