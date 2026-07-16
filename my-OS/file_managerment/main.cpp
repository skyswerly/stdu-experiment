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
