/**
 * @file    icon.h
 * @brief   时段图标判断模块 — 头文件
 *
 * 提供两个纯函数，实现"小时 → 时段 → 图标"的映射：
 *   - icon_get_period(hour)  — 根据小时数 (0-23) 判断所属时段
 *   - icon_period_str(period) — 将时段枚举转换为可读的文字描述
 *
 * 一天 24 小时被划分为 6 个有视觉区分度的时段，
 * 每个时段对应一种太阳/月亮图标 (由 Widget 的 6 个 draw 函数实现)。
 */

#ifndef ICON_H
#define ICON_H

#include "hal.h"

/**
 * @brief 根据当前小时判断所属时段
 * @param hour  小时数 (0-23)
 * @return icon_period_t  时段枚举值
 *
 * 映射规则 (threshold-based，无循环，O(1) 时间复杂度)：
 *
 *   hour ∈ [ 6,  7]  → ICON_MORNING_SUN   清晨半轮朝阳
 *   hour ∈ [ 8, 11]  → ICON_YELLOW_SUN    早晨圆形黄太阳
 *   hour ∈ [12, 13]  → ICON_GLEAMING_SUN  中午带光芒金阳
 *   hour ∈ [14, 17]  → ICON_WESTERN_SUN   下午偏西太阳
 *   hour =  18       → ICON_RED_SUN       傍晚深红夕阳
 *   hour ∈ [19,  5]  → ICON_NIGHT_MOON    夜晚弯月 (包括凌晨)
 *
 * 链式 if-else 结构，短路求值，平均比较次数 ~3 次。
 */
icon_period_t icon_get_period(uint8_t hour);

/**
 * @brief 获取时段的英文文字描述
 * @param period  时段枚举值
 * @return const char*  静态字符串，生命周期为程序全程
 *
 * 返回值是编译期确定的字符串字面量，无需动态分配内存。
 * 格式示例："Morning (08:00 - 11:59)"
 */
const char* icon_period_str(icon_period_t period);

#endif // ICON_H
