#include "main_window.h"

#include <QTabWidget>

#include "device/DeviceWidget.h"   // 设备管理
#include "file/disk_widget.h"      // 磁盘调度
#include "file/file_widget.h"      // 文件系统
#include "memory/MemoryWidget.h"   // 内存管理
#include "thread/ThreadWidget.h"   // 进程管理

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle("模拟操作系统 (Simulated OS)");
    resize(1000, 700);

    tab_widget_ = new QTabWidget(this);

    // 标签页 0：设备管理 — 银行家算法
    tab_widget_->addTab(new DeviceWidget(this), "设备管理");

    // 标签页 1：文件管理 — 磁盘调度 + 文件系统（内嵌子标签）
    auto* fileInnerTabs = new QTabWidget(this);
    fileInnerTabs->addTab(new DiskWidget(this), "磁盘调度");
    fileInnerTabs->addTab(new FileWidget(this), "文件系统");
    tab_widget_->addTab(fileInnerTabs, "文件管理");

    // 标签页 2：内存管理 — 页面置换
    tab_widget_->addTab(new MemoryWidget(this), "内存管理");

    // 标签页 3：进程管理 — 调度算法
    tab_widget_->addTab(new ThreadWidget(this), "进程管理");

    setCentralWidget(tab_widget_);
}
