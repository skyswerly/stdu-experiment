/**
 * @file    main.cpp
 * @brief   智能电子钟程序入口
 *
 * Qt GUI 应用的标准启动流程：
 *   1. 创建 QApplication 实例 — 初始化 Qt 框架运行时
 *   2. 实例化主窗口 Widget  — 构造 UI、启动定时器、初始化时钟数据
 *   3. 显示主窗口           — 首次触发 paintEvent 绘制界面
 *   4. 进入事件循环         — 程序在此阻塞运行，直到窗口关闭
 *
 * 编译：
 *   - PC 端 (测试):  qmake && make (使用桌面 Qt)
 *   - ARM 端 (部署): qmake → Makefile 使用 arm-linux-g++ 交叉编译
 *
 * 运行：
 *   - PC 端: ./smart_clock (通过键盘 S/↑/↓ 操作)
 *   - ARM 端: ./smart_clock -qws (通过物理按键操作)
 */

#include <QApplication>
#include "widget.h"

/**
 * @brief 应用程序主函数
 * @param argc 命令行参数个数 (由 OS 传入)
 * @param argv 命令行参数数组 (由 OS 传入)
 * @return int  程序退出码 (0 = 正常退出)
 *
 * 生命周期：
 *   QApplication 构造 → Widget 构造 → show() → exec() → return
 *
 * ═══════════════════════════════════════════════════════
 *  关于 QApplication 与 Widget 的区别：
 *
 *  QApplication a(argc, argv)
 *    - 进程早在 main() 之前就由 OS 创建好了
 *    - 此行只是把 Qt 框架"安装"到当前进程：
 *      初始化事件循环引擎、字体系统、配色方案等基础设施
 *    - 有且仅有一个 QApplication 实例 (全局单例)
 *    - 它不负责任何 UI 显示
 *
 *  Widget w
 *    - 只是进程拥有的一个窗口对象 (可以有多个)
 *    - 负责屏幕上看到的一切：外观、交互、业务逻辑
 *    - 窗口关闭 → exec() 返回 → 进程退出
 * ═══════════════════════════════════════════════════════
 */
int main(int argc, char *argv[])
{
    // ① 初始化 Qt 框架：字体、样式、事件循环引擎等底层基础设施
    QApplication a(argc, argv);

    // ② 构造主窗口对象：
    //    - RTC_GetTime() 读取系统时间作为初始值
    //    - 创建两个 QTimer (1秒走时 + 50ms按键轮询)
    //    - 设置窗口标题与固定尺寸 600×450
    Widget w;

    // ③ 将窗口显示到屏幕上，触发首次 paintEvent
    w.show();

    // ④ 进入 Qt 事件循环，程序在此阻塞运行
    //    此后所有代码都由事件驱动 (定时器 / 键盘 / 重绘)
    return a.exec();
}
