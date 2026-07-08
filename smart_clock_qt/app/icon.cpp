/**
 * @file    icon.cpp
 * @brief   时段图标判断模块 — 实现文件 (~30 行)
 *
 * 实现两个功能：
 *   1. 小时值 → 时段枚举 (icon_get_period)
 *   2. 时段枚举 → 可读字符串 (icon_period_str)
 *
 * 两者都是纯函数 (无副作用、无状态)，函数结果仅取决于输入参数。
 */

#include "icon.h"

/**
 * @brief 根据当前时间 → 时段映射枚举值
 *
 * 使用链式 if-else 进行区间判断。
 * 注意判断顺序：先判晨→午→晚，最后 fallback 到夜晚，
 * 保证覆盖全部 24 小时 (包括凌晨 0-5 点)。
 *
 * 每个区间左闭右开 [start, end) 或精确匹配 (==)。
 *
 * 时间复杂度：平均 O(1)，最坏 6 次比较 (命中夜晚)
 * 空间复杂度：O(1)
 */
icon_period_t icon_get_period(uint8_t hour)
{
    if (hour >= 6 && hour < 8)
    {
        return ICON_MORNING_SUN; // 06:00 - 07:59 清晨
    }
    else if (hour >= 8 && hour < 12)
    {
        return ICON_YELLOW_SUN; // 08:00 - 11:59 早晨
    }
    else if (hour >= 12 && hour < 14)
    {
        return ICON_GLEAMING_SUN; // 12:00 - 13:59 中午
    }
    else if (hour >= 14 && hour < 18)
    {
        return ICON_WESTERN_SUN; // 14:00 - 17:59 下午
    }
    else if (hour == 18)
    {
        return ICON_RED_SUN; // 18:00 - 18:59 傍晚
    }
    else
    {
        return ICON_NIGHT_MOON; // 19:00 - 05:59 夜晚
    }
}

/**
 * @brief 时段枚举值 → 可读字符串
 *
 * 使用 switch-case 结构，每个 case 对应一个编译期常量字符串。
 * 返回的 const char* 指向只读数据段，无需调用者释放。
 *
 * default 分支处理非法枚举值 (逻辑上不应出现，防御性编程)。
 */
const char *icon_period_str(icon_period_t period)
{
    switch (period)
    {
    case ICON_MORNING_SUN:
        return "Early Morning (06:00 - 07:59)";
    case ICON_YELLOW_SUN:
        return "Morning (08:00 - 11:59)";
    case ICON_GLEAMING_SUN:
        return "Noon (12:00 - 13:59)";
    case ICON_WESTERN_SUN:
        return "Afternoon (14:00 - 17:59)";
    case ICON_RED_SUN:
        return "Evening (18:00 - 18:59)";
    case ICON_NIGHT_MOON:
        return "Night (19:00 - 05:59)";
    default:
        return "Unknown Period";
    }
}
