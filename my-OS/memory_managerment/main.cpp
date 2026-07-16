/**
 * 请求分页存储管理模拟
 * FIFO / LRU 两种页面置换算法
 */

#include <QApplication>
#include "main_window.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    app.setApplicationName("memory-management");

    MainWindow window;
    window.show();

    return app.exec();
}
