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
