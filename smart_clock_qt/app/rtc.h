/**
 * @file    rtc.h
 * @brief   实时时钟 (RTC) 模拟层
 *
 * 通过操作系统 API 获取当前时间，填充 hal_datetime_t 结构体。
 *
 * 为什么叫"模拟"：
 *   真正的 RTC 是硬件芯片 (如 DS1307)，通过 I2C 读取。
 *   本模块使用 QTime::currentTime() / QDate::currentDate() 从系统时钟
 *   获取时间，在 PC 端和 ARM Linux 上均可用，达到了"模拟硬件 RTC"的效果。
 *
 * 设计说明：
 *   - 全函数 inline，放在头文件中，避免额外的 .cpp 编译单元
 *   - 只在 Widget 构造时调用一次，后续时钟走时由 onTimerUpdate() 管理
 *   - 不是每秒调用 (那样会丢失"手动调整"的效果)
 */

#ifndef RTC_H
#define RTC_H

#include "hal.h"
#include <QTime>
#include <QDate>

/**
 * @brief 从系统时钟读取当前时间
 * @param dt  [out] 指向 hal_datetime_t 结构体的指针，结果写入此处
 *
 * 分别从 QDate 获取年月日、从 QTime 获取时分秒，
 * 填入跨平台统一的 hal_datetime_t 结构体。
 *
 * 调用时机：
 *   - Widget 构造函数 — 初始化时钟显示值
 *   - (未来可扩展为按键"复位到当前时间"功能)
 *
 * 注意：
 *   本函数只读取一次快照。读取后 dt 中的时间不会自动更新，
 *   需要 onTimerUpdate() 每秒累加 second 字段来驱动走时。
 */
static inline void RTC_GetTime(hal_datetime_t *dt) {
    QTime now   = QTime::currentTime();
    QDate today = QDate::currentDate();

    dt->year   = today.year();
    dt->month  = today.month();
    dt->day    = today.day();
    dt->hour   = now.hour();
    dt->minute = now.minute();
    dt->second = now.second();
}

#endif // RTC_H
