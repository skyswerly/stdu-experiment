/**
 * @file    widget.cpp
 * @brief   智能电子钟主窗口 — 实现文件 (~350 行)
 *
 * 本文件是应用程序的核心实现，包含：
 *   - 窗口构造与定时器管理
 *   - 时钟走时逻辑 (含进位链)
 *   - 整点报时检测
 *   - 双通道按键处理 (键盘 + 物理按键)
 *   - 7 步完整绘制流程 (背景 → 文字 → 图标)
 *   - 6 种时段太阳/月亮图标的 QPainter 实现
 *
 * 架构设计要点：
 *   - 所有绘制坐标基于 kBaseWidth=800 × kBaseHeight=600 设计
 *   - 通过 QPainter::translate + scale 实现响应式缩放
 *   - 图标位置由基准尺寸计算，不写死像素值
 *   - 六个时段绘图函数独立封装，各约 15~30 行
 *
 * 修改记录：
 *   - 原始版：固定 600×450，中文界面，写死坐标
 *   - 当前版：800×600 基准 + 缩放，英文界面，计算坐标，新增蜂鸣器
 */

#include "widget.h"
#include "key.h"
#include "icon.h"
#include "rtc.h"
#include <QPainter>
#include <QPainterPath>
#include <QKeyEvent>
#include <QTime>
#include <cmath>
#define _USE_MATH_DEFINES
#include <cmath>

/*══════════════════════════════════════════════════════════════════
 * 匿名命名空间 — 布局常量
 *
 * 所有 UI 元素的位置和尺寸都基于这些常量推导，
 * 修改基准尺寸即可全局调整布局，无需逐个修改绘制代码。
 *══════════════════════════════════════════════════════════════════*/
namespace {
    const int kBaseWidth       = 800;   /**< 设计基准宽度 */
    const int kBaseHeight      = 600;   /**< 设计基准高度 */
    const int kIconSizeRatio   = 10;    /**< 图标半径 = 基准宽度 / 10，即 80px */
    const int kIconBoardMargin = 10;    /**< 图标画板距窗口右侧/下侧的距离 */
    const int kIconBoardPadding= 20;    /**< 图标画板边框内边距 */
}

/*══════════════════════════════════════════════════════════════════
 * Widget::Widget — 构造函数
 *══════════════════════════════════════════════════════════════════*/
Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , m_mode(0)               // 初始为正常运行模式 (时钟自动走时)
{
    // ① 从系统时钟初始化时间数据
    RTC_GetTime(&m_datetime);

    // ② 设置窗口属性
    //    - setMinimumSize 保证缩放算法不出现除零或负缩放
    //    - resize 初始为基准的 1.5 倍 (1200×900)，可自由拖拽调整
    setWindowTitle("Visual Electronic Clock");
    setMinimumSize(kBaseWidth, kBaseHeight);
    resize(int(kBaseWidth * 1.5), int(kBaseHeight * 1.5));

    // ③ 创建时钟走时定时器 (1 秒周期)
    //    Qt4 传统连接语法 SIGNAL/SLOT，运行时字符串匹配
    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(onTimerUpdate()));
    m_timer->start(1000);

    // ④ 创建按键轮询定时器 (50 毫秒周期)
    //    频率高于时钟定时器，确保短按不被遗漏
    m_buttonTimer = new QTimer(this);
    connect(m_buttonTimer, SIGNAL(timeout()), this, SLOT(onButtonPoll()));
    m_buttonTimer->start(50);

    // ⑤ 初始化蜂鸣器 (打开 /dev/pwm，失败则降级为日志输出)
    m_buzzer = new Buzzer(this);
}

Widget::~Widget()
{
    // Qt 父子关系自动销毁 m_timer、m_buttonTimer、m_buzzer
    // 无需手动 delete
}

/*══════════════════════════════════════════════════════════════════
 * Widget::checkAndChime — 整点报时检测
 *
 * 在 onTimerUpdate() 中每秒调用一次。
 * 检测整点瞬间 (分=0 且 秒=0)，触发蜂鸣器 200ms 短鸣。
 *
 * 设计细节：
 *   - 只在 m_mode==0 时检测 — 设置模式下调节时间可能跨越整点，
 *     此时不应触发报时
 *   - second==0 作为精确触发点 — 避免在整点分钟内每秒都蜂鸣
 *══════════════════════════════════════════════════════════════════*/
void Widget::checkAndChime()
{
    if (m_mode != 0) return;

    if (m_datetime.minute == 0 && m_datetime.second == 0) {
        m_buzzer->beep(1000, 200);  // 1000Hz 纯音，持续 200ms
    }
}

/*══════════════════════════════════════════════════════════════════
 * Widget::onTimerUpdate — 每秒走时
 *
 * 进位链：second(0-59) → minute(0-59) → hour(0-23)
 * 注意 hour 进位后没有 date 进位 — 日期显示由 paintEvent 读取
 * m_datetime.day 直接使用初始值，不自动更新跨日逻辑。
 *
 * 时间复杂度：O(1)，无循环，4 层 if 嵌套最坏 4 次比较
 *══════════════════════════════════════════════════════════════════*/
void Widget::onTimerUpdate()
{
    if (m_mode == 0) {
        // 秒进位
        m_datetime.second++;
        if (m_datetime.second >= 60) {
            m_datetime.second = 0;

            // 分进位
            m_datetime.minute++;
            if (m_datetime.minute >= 60) {
                m_datetime.minute = 0;

                // 时进位
                m_datetime.hour++;
                if (m_datetime.hour >= 24) {
                    m_datetime.hour = 0;
                }
            }
        }
    }

    checkAndChime();  // 每秒检查整点 → 可能需要触发蜂鸣器
    update();         // 触发 paintEvent 刷新显示
}

/*══════════════════════════════════════════════════════════════════
 * Widget::onButtonPoll — 50ms 物理按键轮询
 *
 * PC 端：/dev/buttons 不存在 → fd < 0 → 立即返回 false → 无操作
 * ARM 端：O_NONBLOCK 读取 → read() 无数据立即返回 -1 → 继续事件循环
 *
 * 边沿检测：对比当前值与上次值，仅在 0→1 翻转时触发一次动作，
 * 避免持续按住按键导致重复触发。
 *══════════════════════════════════════════════════════════════════*/
void Widget::onButtonPoll()
{
    if (KEY_PollButtons(&m_mode, &m_datetime)) {
        update();  // 按键动作确认后立即刷新界面
    }
}

/*══════════════════════════════════════════════════════════════════
 * Widget::keyPressEvent — PC 键盘事件
 *
 * 与物理按键使用同一套 apply_button_action() 逻辑，
 * 保证两种输入方式的行为完全一致。
 *══════════════════════════════════════════════════════════════════*/
void Widget::keyPressEvent(QKeyEvent *event)
{
    char key_code = 0;

    // Qt 键盘码 → 内部 key_code 字符
    if (event->key() == Qt::Key_S) {
        key_code = 's';       // Switch mode (切换模式)
    } else if (event->key() == Qt::Key_Up) {
        key_code = 'u';       // Up (数值增加)
    } else if (event->key() == Qt::Key_Down) {
        key_code = 'd';       // Down (数值减少)
    }

    if (key_code != 0) {
        KEY_Process(&m_mode, &m_datetime, key_code);
        update();             // 立即刷新，延迟极低 (< 1ms)
    }
}

/*══════════════════════════════════════════════════════════════════
 * Widget::paintEvent — 主绘制函数 (7 步流程)
 *
 * 绘图顺序 = 绘制层次 (先画的在底层，后画的在上层)。
 * 使用 painter.save()/restore() 保护状态栈，
 * 确保每个步骤的画笔/画刷/字体修改不影响后续步骤。
 *
 * 缩放算法：
 *   scaleX = 实际宽度 / 800, scaleY = 实际高度 / 600
 *   取 min(scaleX, scaleY) 保持等比缩放
 *   translate 居中偏移 = (实际尺寸 - 缩放后尺寸) / 2
 *══════════════════════════════════════════════════════════════════*/
void Widget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);

    // 全局：开启抗锯齿，所有绘制平滑无锯齿
    painter.setRenderHint(QPainter::Antialiasing);

    /*────────────────────────────────────────────────────────────
     * 第 0 步：计算响应式缩放
     *────────────────────────────────────────────────────────────*/
    const qreal scaleX  = qreal(width())  / qreal(kBaseWidth);
    const qreal scaleY  = qreal(height()) / qreal(kBaseHeight);
    const qreal scale   = qMin(scaleX, scaleY);          // 等比缩放
    const qreal offsetX = (width()  - kBaseWidth  * scale) / 2.0;
    const qreal offsetY = (height() - kBaseHeight * scale) / 2.0;

    // 将坐标系原点移到居中位置，之后所有绘制使用基准 800×600 坐标
    painter.translate(offsetX, offsetY);
    painter.scale(scale, scale);

    /*────────────────────────────────────────────────────────────
     * 第 1 步：根据当前小时判断所属时段
     *────────────────────────────────────────────────────────────*/
    icon_period_t period = icon_get_period(m_datetime.hour);

    /*────────────────────────────────────────────────────────────
     * 第 2 步：绘制渐变背景 (3 种配色方案)
     *
     *   - 夜晚 (19:00-05:59)：深蓝→暗蓝，模拟夜空
     *   - 清晨/傍晚 (06-08, 18)：暗紫红→暗棕，模拟晨昏
     *   - 白天 (08-18)：浅蓝→淡蓝，模拟晴朗天空
     *────────────────────────────────────────────────────────────*/
    QLinearGradient bgGradient(0, 0, 0, kBaseHeight);
    if (period == ICON_NIGHT_MOON) {
        // 夜晚：深蓝黑 → 暗蓝
        bgGradient.setColorAt(0, QColor(10, 15, 30));
        bgGradient.setColorAt(1, QColor(30, 40, 65));
    } else if (period == ICON_RED_SUN || period == ICON_MORNING_SUN) {
        // 晨昏过渡：暗紫红 → 暗棕
        bgGradient.setColorAt(0, QColor(45, 20, 40));
        bgGradient.setColorAt(1, QColor(80, 45, 50));
    } else {
        // 白天：浅蓝 → 淡蓝
        bgGradient.setColorAt(0, QColor(220, 240, 255));
        bgGradient.setColorAt(1, QColor(170, 210, 245));
    }
    painter.fillRect(QRectF(0, 0, kBaseWidth, kBaseHeight), bgGradient);

    /*────────────────────────────────────────────────────────────
     * 第 3 步：标题栏 + 按键提示
     *
     *   文字颜色根据背景明暗自适应：
     *   - 暗色背景 (夜晚/清晨/傍晚) → 白色文字
     *   - 亮色背景 (白天) → 黑色文字
     *────────────────────────────────────────────────────────────*/
    QPen textPen((period == ICON_NIGHT_MOON || period == ICON_MORNING_SUN || period == ICON_RED_SUN)
                 ? Qt::white : Qt::black);
    painter.setPen(textPen);

    // 标题
    QFont titleFont("Microsoft YaHei", 12, QFont::Bold);
    painter.setFont(titleFont);
    painter.drawText(20, 35, "Visual Electronic Clock");

    // 按键使用提示
    QFont tipFont("Microsoft YaHei", 9);
    painter.setFont(tipFont);
    painter.drawText(20, 65, "Tip: [Key 1] Switch mode | [Key 2/3] Increase/Decrease values");

    /*────────────────────────────────────────────────────────────
     * 第 4 步：数字时钟 HH:MM:SS
     *
     *   使用 QChar('0') 前导零填充，保证始终显示 2 位。
     *   等宽字体 "Consolas" 确保数字宽度一致，不会抖动。
     *────────────────────────────────────────────────────────────*/
    QFont clockFont("Consolas", 40, QFont::Bold);
    painter.setFont(clockFont);
    QString timeStr = QString("%1:%2:%3")
                      .arg(m_datetime.hour,   2, 10, QChar('0'))
                      .arg(m_datetime.minute, 2, 10, QChar('0'))
                      .arg(m_datetime.second, 2, 10, QChar('0'));
    painter.drawText(20, 150, timeStr);

    /*────────────────────────────────────────────────────────────
     * 第 5 步：日期 + 时段描述
     *
     *   格式：YYYY-MM-DD <时段英文描述>
     *   如："2026-07-08 Noon (12:00 - 13:59)"
     *────────────────────────────────────────────────────────────*/
    QFont dateFont("Microsoft YaHei", 11);
    painter.setFont(dateFont);
    QString dateStr = QString("%1-%2-%3 %4")
                      .arg(m_datetime.year)
                      .arg(m_datetime.month)
                      .arg(m_datetime.day)
                      .arg(icon_period_str(period));
    painter.drawText(20, 200, dateStr);

    /*────────────────────────────────────────────────────────────
     * 第 6 步：当前模式状态文字
     *
     *   - 正常模式 (m_mode==0)：绿色 "(Normal Operation Mode)"
     *   - 设置模式 (m_mode>0)：红色 "[Setting: Hour/Minute/Second]"
     *────────────────────────────────────────────────────────────*/
    painter.save();
    if (m_mode > 0) {
        QPen modePen(Qt::red, 2);
        painter.setPen(modePen);
        QFont modeFont("Microsoft YaHei", 12, QFont::Bold);
        painter.setFont(modeFont);
        QString modeStr;
        if (m_mode == 1)      modeStr = "[Setting: Hour]";
        else if (m_mode == 2) modeStr = "[Setting: Minute]";
        else if (m_mode == 3) modeStr = "[Setting: Second]";
        painter.drawText(20, 250, modeStr);
    } else {
        painter.setPen(QPen(QColor(50, 150, 50), 2));
        QFont modeFont("Microsoft YaHei", 11, QFont::Bold);
        painter.setFont(modeFont);
        painter.drawText(20, 250, "[Normal Operation Mode]");
    }
    painter.restore();

    /*────────────────────────────────────────────────────────────
     * 第 7 步：时段图标画板
     *
     *   图标位置由布局常量计算：
     *     icon_r  = 基准宽度 / 10 = 80px
     *     icon_cx = 800 - 10 - 80 - 20 = 690  (右对齐)
     *     icon_cy = 600 / 2 = 300             (垂直居中)
     *────────────────────────────────────────────────────────────*/
    const int icon_r  = qRound(kBaseWidth / qreal(kIconSizeRatio));
    const int icon_cx = kBaseWidth - kIconBoardMargin - icon_r - kIconBoardPadding;
    const int icon_cy = kBaseHeight / 2;

    // 虚线边框 + 半透明填充
    painter.save();
    painter.setPen(QPen(period == ICON_NIGHT_MOON ? Qt::white : Qt::black, 3, Qt::DashLine));
    painter.setBrush(QColor(255, 255, 255, 20));   // 白色半透明
    painter.drawRoundedRect(
        icon_cx - icon_r - 20, icon_cy - icon_r - 20,
        (icon_r + 20) * 2, (icon_r + 20) * 2,
        15, 15);  // 圆角半径
    painter.restore();

    // 根据时段分发到对应的绘图函数
    switch (period) {
        case ICON_MORNING_SUN:
            drawMorningSun(painter, icon_cx, icon_cy, icon_r);
            break;
        case ICON_YELLOW_SUN:
            drawYellowSun(painter, icon_cx, icon_cy, icon_r);
            break;
        case ICON_GLEAMING_SUN:
            drawGleamingSun(painter, icon_cx, icon_cy, icon_r);
            break;
        case ICON_WESTERN_SUN:
            drawWesternSun(painter, icon_cx, icon_cy, icon_r);
            break;
        case ICON_RED_SUN:
            drawRedSun(painter, icon_cx, icon_cy, icon_r);
            break;
        case ICON_NIGHT_MOON:
            drawNightMoon(painter, icon_cx, icon_cy, icon_r);
            break;
    }
}

/*══════════════════════════════════════════════════════════════════
 * 六个时段图标绘制函数
 *
 * 每个函数独立封装，约 15~30 行，只关注一种视觉方案。
 * 全部遵循：painter.save() → 绘制 → painter.restore() 模式，
 * 保证各自的画笔/画刷/变换修改不影响后续绘制。
 *══════════════════════════════════════════════════════════════════*/

/**
 * @brief 清晨太阳 — 海平线上方升起半轮橙色朝阳
 *
 * 绘制技法：
 *   - 水平线：天蓝色实线，宽 2×r
 *   - 太阳：QPainter::drawChord() 绘制上半圆弧 (180° 扇形)
 *   - 渐变：径向渐变中心在水平线上(cx, cy+r/2)，模拟太阳紧贴海面
 *   - 对应时段：06:00 - 07:59
 */
void Widget::drawMorningSun(QPainter &painter, int cx, int cy, int r)
{
    painter.save();

    // 天蓝色水平线 (海平面/地平线)
    painter.setPen(QPen(QColor(0, 191, 255), 3));
    painter.drawLine(cx - r, cy + r/2, cx + r, cy + r/2);

    // 半轮橙色朝阳 (drawChord: start=0°, span=180°)
    // 参数中的 *16 是因为 Qt 使用 1/16 度作为角度单位
    QRadialGradient sunGrad(cx, cy + r/2, r);
    sunGrad.setColorAt(0, QColor(255, 140, 0));        // 橙黄中心
    sunGrad.setColorAt(1, QColor(255, 69, 0, 150));    // 橙红边缘 (半透明)
    painter.setBrush(sunGrad);
    painter.setPen(Qt::NoPen);                          // 无描边，柔合融合
    // 矩形: (cx-r/2, cy) 宽高 (r, r)，从 0° 画 180°
    painter.drawChord(cx - r/2, cy, r, r, 0 * 16, 180 * 16);

    painter.restore();
}

/**
 * @brief 早晨太阳 — 空中一轮纯黄圆形，带光晕
 *
 * 绘制技法：
 *   - QRadialGradient 三层渐变：亮黄中心 → 金黄 → 半透明橙色外缘
 *   - 描边用深橙 (255,165,0)，使太阳边缘柔和但有轮廓
 *   - 对应时段：08:00 - 11:59
 */
void Widget::drawYellowSun(QPainter &painter, int cx, int cy, int r)
{
    painter.save();

    QRadialGradient sunGrad(cx, cy, r);
    sunGrad.setColorAt(0.0, QColor(255, 255, 0));          // 中心：纯黄
    sunGrad.setColorAt(0.7, QColor(255, 215, 0));          // 70%：金色
    sunGrad.setColorAt(1.0, QColor(255, 165, 0, 50));      // 外缘：半透明橙
    painter.setBrush(sunGrad);
    painter.setPen(QPen(QColor(255, 165, 0), 2));          // 橙色轮廓线
    // 圆直径 = r，圆心偏移使其居中于 (cx, cy)
    painter.drawEllipse(cx - r/2, cy - r/2, r, r);

    painter.restore();
}

/**
 * @brief 中午太阳 — 耀眼金阳 + 8 条三角光芒
 *
 * 绘制技法：
 *   1. 太阳本体：白黄→金黄→橙红径向渐变
 *   2. 光芒刺：8 个等腰三角形，均分 360° (每 45° 一根)
 *      - 三角形底边在太阳边缘，顶角指向外侧
 *      - 用 cos/sin 计算每个三角形的三个顶点
 *   - 对应时段：12:00 - 13:59
 */
void Widget::drawGleamingSun(QPainter &painter, int cx, int cy, int r)
{
    painter.save();

    // ① 太阳本体
    QRadialGradient sunGrad(cx, cy, r);
    sunGrad.setColorAt(0.0, QColor(255, 255, 240));        // 白黄核心
    sunGrad.setColorAt(0.5, QColor(255, 215, 0));          // 50%：金黄
    sunGrad.setColorAt(1.0, QColor(255, 69, 0, 80));       // 外缘：半透明橙红
    painter.setBrush(sunGrad);
    painter.setPen(QPen(QColor(255, 69, 0), 2));
    painter.drawEllipse(cx - r/2, cy - r/2, r, r);

    // ② 8 条三角光芒刺
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(255, 215, 0, 200));            // 半透明金黄
    for (int i = 0; i < 8; ++i) {
        double angle = i * M_PI / 4.0;                     // 每 45° 一根

        // 三角形三个顶点：
        //   p1, p2: 太阳边缘的两个近点 (角度 ±0.15 rad ≈ ±8.6°)
        //   p3:     光芒尖端，距圆心 r-10 像素
        QPointF p1(cx + (r/2 + 2) * cos(angle - 0.15),
                   cy + (r/2 + 2) * sin(angle - 0.15));
        QPointF p2(cx + (r/2 + 2) * cos(angle + 0.15),
                   cy + (r/2 + 2) * sin(angle + 0.15));
        QPointF p3(cx + (r - 10) * cos(angle),
                   cy + (r - 10) * sin(angle));

        QPolygonF triangle;
        triangle << p1 << p2 << p3;
        painter.drawPolygon(triangle);
    }

    painter.restore();
}

/**
 * @brief 下午太阳 — 暖橙色，向右下偏移模拟"偏西"
 *
 * 绘制技法：
 *   - 圆心偏移 (shift_x, shift_y) = (r/4, r/6)，约 (20, 13) px
 *   - 珊瑚色→橙→暗红径向渐变，温暖但不刺眼
 *   - 对应时段：14:00 - 17:59
 */
void Widget::drawWesternSun(QPainter &painter, int cx, int cy, int r)
{
    painter.save();

    int shift_x = r / 4;    // 向右偏移
    int shift_y = r / 6;    // 向下偏移
    QRadialGradient sunGrad(cx + shift_x, cy + shift_y, r);
    sunGrad.setColorAt(0.0, QColor(255, 127, 80));          // 珊瑚色
    sunGrad.setColorAt(0.8, QColor(255, 69, 0));            // 橙红
    sunGrad.setColorAt(1.0, QColor(139, 0, 0, 30));         // 暗红半透明
    painter.setBrush(sunGrad);
    painter.setPen(QPen(QColor(255, 69, 0), 2));
    painter.drawEllipse(cx + shift_x - r/2, cy + shift_y - r/2, r, r);

    painter.restore();
}

/**
 * @brief 傍晚太阳 — 大半沉入海面的深红夕阳
 *
 * 绘制技法：
 *   - 水平线：深紫灰 (72, 61, 139)，色调比清晨的蓝线更暗
 *   - 夕阳：drawChord 下半圆弧 (从水平线向下)
 *   - 渐变：鲜红→火砖红→暗红
 *   - 矩形区域放大 (r+20) 以让大半轮太阳出现在地平线下方
 *   - 对应时段：18:00 - 18:59
 */
void Widget::drawRedSun(QPainter &painter, int cx, int cy, int r)
{
    painter.save();

    // 深色水平线
    painter.setPen(QPen(QColor(72, 61, 139), 3));
    painter.drawLine(cx - r, cy + r/2, cx + r, cy + r/2);

    // 沉入海面的夕阳
    QRadialGradient sunGrad(cx, cy + r/2, r);
    sunGrad.setColorAt(0.0, QColor(255, 0, 0));             // 纯红
    sunGrad.setColorAt(0.6, QColor(178, 34, 34));           // 火砖红
    sunGrad.setColorAt(1.0, QColor(139, 0, 0, 50));         // 暗红半透明
    painter.setBrush(sunGrad);
    painter.setPen(Qt::NoPen);
    // 向下偏移 + 放大绘制区域，使太阳"沉入"水面
    painter.drawChord(cx - r/2 - 10, cy + r/3, r + 20, r + 20, 0 * 16, 180 * 16);

    painter.restore();
}

/**
 * @brief 夜晚月亮 — 星空 + QPainterPath 差集月牙
 *
 * 绘制技法：
 *   1. 星星：4 个白色小圆点散布在画板四周
 *   2. 月牙：QPainterPath::subtracted() 双圆差集法
 *      - 大圆 (moonPath) 为基底圆
 *      - 偏移大圆 (clipPath) 为裁剪圆，向右偏移 r/4
 *      - 差集 crescent = moonPath - clipPath → 弯月形状
 *   3. 线性渐变填充月牙（白→银灰）
 *   - 对应时段：19:00 - 05:59
 */
void Widget::drawNightMoon(QPainter &painter, int cx, int cy, int r)
{
    painter.save();

    /* ① 画星星 — 4 个无描边的白色小圆点 */
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(255, 255, 224));     // 暖白
    painter.drawEllipse(cx - r/2, cy - r/3, 4, 4);     // 左上
    painter.drawEllipse(cx + r/3, cy - r/2, 3, 3);     // 右上
    painter.drawEllipse(cx - r/3, cy + r/3, 3, 3);     // 左下
    painter.drawEllipse(cx + r/2, cy + r/4, 4, 4);     // 右下

    /* ② 画月牙 — Path 差集技法
     *
     *   ┌─────────┐     ┌────────────┐
     *   │  moon   │     │  moon      │
     *   │  Path   │  -  │    clip    │  =  🌙 月牙
     *   │  (大圆)  │     │ (偏移大圆)  │
     *   └─────────┘     └────────────┘
     */
    // 基底大圆
    QPainterPath moonPath;
    moonPath.addEllipse(cx - r/2, cy - r/2, r, r);

    // 裁剪偏移圆 (向右偏移 r/4，重合部分被切掉)
    QPainterPath clipPath;
    clipPath.addEllipse(cx - r/2 + r/4, cy - r/2, r, r);

    // 差集 = 月牙
    QPainterPath crescent = moonPath.subtracted(clipPath);

    // 月牙填充：线性渐变 银白 → 浅灰
    QLinearGradient moonGrad(cx - r/2, cy - r/2, cx + r/2, cy + r/2);
    moonGrad.setColorAt(0, QColor(255, 250, 240));
    moonGrad.setColorAt(1, QColor(224, 224, 224));
    painter.setBrush(moonGrad);
    painter.setPen(QPen(QColor(255, 255, 255, 180), 1));   // 半透明白边
    painter.drawPath(crescent);

    painter.restore();
}
