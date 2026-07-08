/**
 * @file    buzzer.cpp
 * @brief   PWM 蜂鸣器驱动 — 实现文件
 *
 * 操作 Tiny4412 板载 PWM 蜂鸣器的核心逻辑：
 *   - 构造函数打开 /dev/pwm 设备
 *   - beep() 通过 ioctl SET_FREQ 启动鸣叫，QTimer::singleShot 定时关闭
 *   - stop() 通过 ioctl STOP 关闭 PWM
 *   - 设备不可用时自动降级，不影响主程序运行
 */

#include "buzzer.h"
#include <QTimer>

/**
 * @brief 构造函数 — 打开 PWM 蜂鸣器设备
 *
 * 以读写模式打开 /dev/pwm。若设备不存在 (PC 环境)，
 * fd 保持 -1，后续所有操作走日志降级路径。
 */
Buzzer::Buzzer(QObject *parent)
    : QObject(parent)
    , fd(-1)         // 默认设备不可用
    , is_on(false)   // 初始状态为静音
{
    fd = open("/dev/pwm", O_RDWR);
    if (fd < 0) {
        qWarning() << "无法打开 /dev/pwm，蜂鸣器不可用";
        // 不 return — 对象仍然可用，beep() 会走日志降级路径
    }
}

/**
 * @brief 析构函数 — 静音并释放设备
 *
 * 先停止 PWM 输出避免程序退出后蜂鸣器继续响，
 * 再关闭文件描述符释放内核资源。
 */
Buzzer::~Buzzer()
{
    stop();                           // 先静音
    if (fd >= 0) {
        close(fd);                    // 再释放设备
    }
}

/**
 * @brief 以指定频率发声，持续一定时间
 * @param freq        频率 (Hz)，有效范围通常 1 ~ 10000
 * @param duration_ms 持续时间 (ms)，到期后自动调用 stop()
 *
 * 执行路径：
 *   ARM 端 (fd >= 0)：
 *     1. ioctl(fd, SET_FREQ, freq) — 启动 PWM 输出
 *     2. QTimer::singleShot(duration_ms, this, SLOT(stop())) — 定时关闭
 *
 *   PC 端 (fd < 0)：
 *     1. qDebug() 打印模拟信息
 *     2. 直接 return
 *
 * 使用 Qt4 兼容的字符串槽名语法 "stop()"，
 * 这是为了兼容 Qt Embedded 4.8.5 的 MOC 实现。
 */
void Buzzer::beep(int freq, int duration_ms)
{
    if (fd < 0) {
        // PC 降级路径：仅输出日志
        qDebug() << "[模拟蜂鸣器] 频率:" << freq << "Hz, 持续:" << duration_ms << "ms";
        return;
    }

    // ARM 路径：通过 ioctl 设置 PWM 频率
    if (ioctl(fd, PWM_IOCTL_SET_FREQ, freq) == 0) {
        is_on = true;
    } else {
        perror("ioctl SET_FREQ 失败");
        return;
    }

    // 使用单次定时器，duration_ms 毫秒后自动调用 stop()
    // Qt4 兼容写法：使用字符串 SLOT(stop()) 而非函数指针
    QTimer::singleShot(duration_ms, this, SLOT(stop()));
}

/**
 * @brief 立即停止蜂鸣器发声
 *
 * 调用 ioctl(fd, PWM_IOCTL_STOP) 将 PWM 占空比置零。
 * 若设备已不可用或蜂鸣器未在响，则无操作。
 */
void Buzzer::stop()
{
    if (fd < 0 || !is_on) return;

    if (ioctl(fd, PWM_IOCTL_STOP) == 0) {
        is_on = false;
    } else {
        perror("ioctl STOP 失败");
    }
}
