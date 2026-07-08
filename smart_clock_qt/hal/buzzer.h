/**
 * @file    buzzer.h
 * @brief   PWM 蜂鸣器驱动 — 头文件
 *
 * 封装 ARM 开发板 /dev/pwm 设备的操作：
 *   - 通过 ioctl() 设置 PWM 频率发出指定音调
 *   - 通过 QTimer::singleShot() 实现定时自动停止
 *   - PC 端 /dev/pwm 不存在时自动降级为 qDebug 日志
 *
 * 使用方式：
 *   Buzzer *buzzer = new Buzzer(this);
 *   buzzer->beep(1000, 200);  // 1000Hz 响 200ms 后自动停止
 *
 * 依赖：
 *   - /dev/pwm 字符设备 (ARM 板)
 *   - ioctl 命令 PWM_IOCTL_SET_FREQ / PWM_IOCTL_STOP
 */

#ifndef BUZZER_H
#define BUZZER_H

#include <QObject>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <QDebug>

/**
 * @brief ioctl 命令字
 *
 * PWM_IOCTL_SET_FREQ = 1 — 设置 PWM 频率并启动输出
 * PWM_IOCTL_STOP     = 0 — 停止 PWM 输出 (关闭蜂鸣器)
 *
 * 这些值来源于 Tiny4412 板载 PWM 驱动程序的接口定义。
 */
#define PWM_IOCTL_SET_FREQ 1
#define PWM_IOCTL_STOP     0

class Buzzer : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数 — 打开 /dev/pwm 设备
     * @param parent 父对象指针 (用于 Qt 内存管理)
     *
     * 若打开失败 (PC 端无此设备)，保存 fd=-1，
     * 后续 beep() 调用会降级为 qDebug 日志输出。
     */
    explicit Buzzer(QObject *parent = 0);

    /** @brief 析构函数 — 停止蜂鸣并关闭设备文件 */
    ~Buzzer();

    /**
     * @brief 以指定频率发声，持续一段时间后自动停止
     * @param freq        频率 (Hz)，如 1000 = 1kHz 中音
     * @param duration_ms 持续时间 (毫秒)，到时自动调用 stop()
     *
     * 实现细节：
     *   - ARM 端：ioctl(fd, SET_FREQ, freq) 启动 PWM
     *   - PC 端：qDebug() 输出 "[模拟蜂鸣器] 频率: xxx Hz, 持续: xxx ms"
     *   - 使用 QTimer::singleShot 在 duration_ms 后调用 stop()
     */
    void beep(int freq, int duration_ms);

public slots:
    /**
     * @brief 立即停止发声
     *
     * 调用 ioctl(fd, PWM_IOCTL_STOP) 关闭 PWM 输出。
     * 由 QTimer::singleShot 或析构函数自动触发。
     */
    void stop();

private:
    int  fd;     /**< /dev/pwm 文件描述符，-1 表示设备不可用 */
    bool is_on;  /**< 当前蜂鸣器是否正在发声 */
};

#endif // BUZZER_H
