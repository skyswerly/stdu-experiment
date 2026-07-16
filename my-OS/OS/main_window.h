#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>

class QTabWidget;

// 模拟操作系统主窗口
class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);

private:
    QTabWidget* tab_widget_;  // 标签页容器
};

#endif // MAIN_WINDOW_H
