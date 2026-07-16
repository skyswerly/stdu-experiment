// main.cpp — 模拟操作系统统一入口
// 整合设备管理、文件管理、内存管理、进程管理四大模块
// 基于 Qt6 Widgets 提供统一图形界面

#include <QApplication>
#include "main_window.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("SimulatedOS");

    MainWindow window;
    window.show();

    return app.exec();
}
