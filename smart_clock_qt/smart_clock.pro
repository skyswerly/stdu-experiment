# ============================================================================
#  Visual Electronic Clock — Qt Project File
# ============================================================================
#  目标平台：
#    - PC 桌面 (开发调试)：使用系统 Qt 库编译，键盘 S/↑/↓ 操作
#    - Tiny4412 ARM (实际部署)：交叉编译 arm-linux-g++，物理按键操作
#
#  Qt 版本兼容：
#    - Qt 4.x (Embedded)：项目原始目标版本 (QtEmbedded-4.8.5-arm)
#    - Qt 5.x+：第 3 行 greaterThan 条件自动引入 widgets 模块
#
#  构建：
#    PC:  qmake && make
#    ARM: /path/to/arm-qmake && make
# ============================================================================

# 核心模块：core (事件循环、字符串) + gui (绘制、窗口)
QT += core gui

# Qt5+ 将 QWidget 从 QtGui 拆分到独立 QtWidgets 模块，
# 需要显式引入以保证跨版本兼容
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

# C++11 选项 (当前注释掉，因为 Qt Embedded 4.8.5 的 ARM 交叉编译器
# 可能不完全支持 C++11；若使用现代编译器可取消注释)
# CONFIG += c++11

# ============================================================================
# 源文件列表
# ============================================================================
SOURCES += \
    main.cpp \
    widget.cpp \
    app/icon.cpp \
    app/key.cpp \
    hal/buzzer.cpp

HEADERS += \
    widget.h \
    hal/hal.h \
    app/icon.h \
    app/key.h \
    app/rtc.h \
    hal/buzzer.h

# ============================================================================
# 包含路径
# hal/  — 硬件抽象层 (hal_datetime_t, icon_period_t, Buzzer)
# app/  — 应用逻辑层 (RTC, Key, Icon)
# ============================================================================
INCLUDEPATH += hal app

# ============================================================================
# 部署规则 (目标设备安装路径)
# ============================================================================
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
