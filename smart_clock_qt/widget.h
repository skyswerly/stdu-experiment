/**
 * @file    widget.h
 * @brief   智能电子钟主窗口 — 头文件
 *
 * Widget 类是整个应用的核心，承担以下职责：
 *   1. 时间管理    — 模拟 RTC 时钟每秒走时，支持进位链与整点报时
 *   2. 模式控制    — 管理 4 种工作模式 (正常 / 设时 / 设分 / 设秒)
 *   3. 双通道输入   — PC 键盘 (开发调试) + ARM 物理按键 (实际部署)
 *   4. 界面绘制    — 渐变背景 + 数字时钟 + 日期 + 彩色时段图标 + 响应式缩放
 *   5. 蜂鸣器控制   — 整点触发 PWM 蜂鸣器发出提示音
 *
 * 依赖关系：
 *   Widget ──依赖──▶ hal.h      (hal_datetime_t, icon_period_t)
 *   Widget ──依赖──▶ buzzer.h   (Buzzer 蜂鸣器)
 *   Widget ──调用──▶ rtc.h      (RTC_GetTime)
 *   Widget ──调用──▶ key.h      (KEY_Process, KEY_PollButtons)
 *   Widget ──调用──▶ icon.h     (icon_get_period, icon_period_str)
 */

#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QTimer>
#include "hal.h"
#include "buzzer.h"

class Widget : public QWidget
{
    Q_OBJECT  /**< Qt 元对象系统宏 — 启用信号/槽机制，MOC 自动生成 moc_widget.cpp */

public:
    /**
     * @brief 构造函数 — 初始化时钟数据、定时器、窗口属性、蜂鸣器
     * @param parent 父窗口指针 (nullptr 表示顶层窗口)
     *
     * 初始化顺序：
     *   1. m_mode 置 0 (正常运行模式)
     *   2. RTC_GetTime() 从系统读取当前时间
     *   3. 设置窗口标题与最小尺寸 (800×600 基准)
     *   4. 创建并启动两个 QTimer (走时 + 按键轮询)
     *   5. 实例化 Buzzer 蜂鸣器对象
     */
    Widget(QWidget *parent = 0);

    /** @brief 析构函数 — Qt 父子关系自动回收 QTimer 和 Buzzer 资源 */
    ~Widget();

protected:
    /**
     * @brief Qt 绘制事件回调 — 每帧重新绘制整个窗口
     *
     * 触发时机：
     *   - 窗口首次 show()
     *   - 任何代码路径调用 update() (onTimerUpdate / 按键处理)
     *   - 窗口被遮挡后重新暴露 / 缩放
     *
     * 响应式缩放策略：
     *   所有绘制坐标基于 kBaseWidth=800 × kBaseHeight=600 设计，
     *   通过 painter.translate() + painter.scale() 适配任意窗口尺寸，
     *   保持等比缩放 + 居中显示。
     *
     * 绘制流程 (共 7 步，详见 widget.cpp)：
     *   ① 计算缩放比例与偏移
     *   ② 判断时段 → ③ 渐变背景 → ④ 标题栏
     *   → ⑤ 数字时钟 + 日期 + 模式 → ⑥ 图标画板边框 → ⑦ 时段图标
     */
    void paintEvent(QPaintEvent *event);

    /**
     * @brief Qt 键盘按下事件 — PC 端按键输入通道
     *
     * 按键映射：
     *   - S 键 → 's' (切换模式: 0→1→2→3→0)
     *   - ↑ 键 → 'u' (增加当前设置项的值)
     *   - ↓ 键 → 'd' (减少当前设置项的值)
     *
     * 与物理按键 (KEY_PollButtons) 的区别：
     *   - keyPressEvent 是 Qt 原生事件，即时响应，延迟 < 1ms
     *   - KEY_PollButtons 是定时器轮询 /dev/buttons，50ms 周期，边沿触发
     *   - 两者最终都调用 KEY_Process() -> apply_button_action()，行为一致
     */
    void keyPressEvent(QKeyEvent *event);

private slots:
    /**
     * @brief 1 秒定时器回调 — 驱动时钟走时
     *
     * 在正常运行模式 (m_mode==0) 下：
     *   - 递增 m_datetime.second
     *   - 处理秒→分→时→日的进位链
     * 在设置模式 (m_mode≠0) 下：
     *   - 跳过自动累加，数值由用户通过按键手动调节
     *
     * 末尾执行两件事：
     *   1. checkAndChime()  — 检测整点并触发蜂鸣
     *   2. update()         — 触发 paintEvent 刷新显示
     */
    void onTimerUpdate();

    /**
     * @brief 50ms 按键轮询回调 — 读取 ARM 物理按键
     *
     * 调用 KEY_PollButtons() 以 O_NONBLOCK 模式读取 /dev/buttons，
     * 对比前后两次读取结果进行边沿检测，捕获按键按下瞬间。
     * 若检测到有效按键则调用 update() 立即刷新界面。
     *
     * 轮询频率 50ms 的设计依据：
     *   - 20Hz 远高于人类按键频率 (最快 ~100ms/次)
     *   - 两次轮询间不超过 100ms，不会遗漏短按
     *   - 不对 CPU 造成显著负载
     */
    void onButtonPoll();

private:
    hal_datetime_t m_datetime; /**< 当前时间 — 全模块唯一的时间数据源 */
    uint8_t m_mode;            /**< 工作模式: 0=正常, 1=设时, 2=设分, 3=设秒 */

    /**
     * @brief 时钟走时定时器 (1 秒间隔)
     *
     * QTimer 基于 Qt 事件循环而非硬件定时器，精度取决于事件循环响应速度。
     * 在非阻塞场景下误差 < 10ms；长时间运行会有累积误差（每月数秒），
     * 对本应用（无需精确计时）可接受。
     */
    QTimer *m_timer;

    /**
     * @brief 物理按键轮询定时器 (50 毫秒间隔)
     *
     * 见 onButtonPoll() 注释了解 50ms 间隔的设计依据。
     */
    QTimer *m_buttonTimer;

    /**
     * @brief 蜂鸣器 (PWM 驱动)
     *
     * 通过 /dev/pwm 设备控制 ARM 板载蜂鸣器。
     * 在 PC 端 /dev/pwm 不存在时自动降级为 qDebug 日志输出。
     */
    Buzzer *m_buzzer;

    /**
     * @brief 整点报时检测 — 在 onTimerUpdate() 中每秒调用
     *
     * 触发条件：m_mode==0 && minute==0 && second==0 (整点瞬间)
     * 行为：调用 m_buzzer->beep(1000Hz, 200ms) 发一声短鸣
     */
    void checkAndChime();

    /*══════════════════════════════════════════════════════════════
     * 六个时段图标绘制函数
     *
     * 每个函数对应 icon_period_t 的一个枚举值。
     * 全部使用 QPainter API 纯代码绘制 (零外部图片资源依赖)，
     * 适合嵌入式设备上无文件系统辅助的场景。
     *
     * 参数约定：
     *   @param painter  QPainter 引用，调用前已启用 Antialiasing
     *   @param cx       图标画板中心 X 坐标 (以基准 800×600 坐标系为准)
     *   @param cy       图标画板中心 Y 坐标
     *   @param r        图标画板半径 (实际绘制半径约为 r/2~r 之间)
     *══════════════════════════════════════════════════════════════*/

    /** @brief 清晨 (06:00-07:59)：海平线 + QPainterPath 弧形半轮朝阳 */
    void drawMorningSun(QPainter &painter, int cx, int cy, int r);

    /** @brief 早晨 (08:00-11:59)：纯黄圆形太阳 + 径向渐变光晕 */
    void drawYellowSun(QPainter &painter, int cx, int cy, int r);

    /** @brief 中午 (12:00-13:59)：耀眼金阳本体 + 8 条三角光芒芒刺 */
    void drawGleamingSun(QPainter &painter, int cx, int cy, int r);

    /** @brief 下午 (14:00-17:59)：暖橙太阳，右下方偏移模拟偏西效果 */
    void drawWesternSun(QPainter &painter, int cx, int cy, int r);

    /** @brief 傍晚 (18:00-18:59)：水平线 + 大半没入海面的深红夕阳 */
    void drawRedSun(QPainter &painter, int cx, int cy, int r);

    /** @brief 夜晚 (19:00-05:59)：4 颗星 + QPainterPath::subtracted() 差集月牙 */
    void drawNightMoon(QPainter &painter, int cx, int cy, int r);
};

#endif // WIDGET_H
