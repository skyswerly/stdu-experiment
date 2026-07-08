/**
 * @file    key.h
 * @brief   按键处理模块 — 头文件
 *
 * 提供两套接口处理用户按键输入：
 *   - KEY_Process()     — 软件按键处理 (供键盘事件 keyPressEvent 调用)
 *   - KEY_PollButtons() — 物理按键轮询 (供定时器 onButtonPoll 调用)
 *
 * 工作模式 (m_mode)：
 *   - 0 = 正常运行 (时钟自动走时)
 *   - 1 = 设置小时  (↑↓ 调节 0-23 循环)
 *   - 2 = 设置分钟  (↑↓ 调节 0-59 循环)
 *   - 3 = 设置秒数  (↑↓ 调节 0-59 循环)
 *
 * key_code 约定：
 *   - 's' = Switch  (模式切换，按 S 键或物理按键 1)
 *   - 'u' = Up      (数值增加，按 ↑ 键或物理按键 2)
 *   - 'd' = Down    (数值减少，按 ↓ 键或物理按键 3)
 */

#ifndef KEY_H
#define KEY_H

#include "hal.h"

/**
 * @brief 处理单个按键动作 — 模式切换 / 数值加减
 * @param mode     [in/out] 当前工作模式指针
 * @param dt       [in/out] 当前时间数据指针
 * @param key_code         按键字符: 's'=切换模式, 'u'=增加, 'd'=减少
 *
 * 内部调用 apply_button_action()，根据当前 mode 和 key_code
 * 修改 mode (切换) 或 dt 中的对应字段 (加减)。
 *
 * 由 keyPressEvent 直接调用 (PC 键盘) 或
 * KEY_PollButtons 间接调用 (ARM 物理按键)。
 */
void KEY_Process(uint8_t *mode, hal_datetime_t *dt, char key_code);

/**
 * @brief 轮询 /dev/buttons 物理按键设备
 * @param mode  [in/out] 当前工作模式指针
 * @param dt    [in/out] 当前时间数据指针
 * @return bool  true = 本次轮询检测到按键动作，false = 无动作
 *
 * 工作流程：
 *   1. 以 O_NONBLOCK 模式打开 /dev/buttons 设备 (仅首次)
 *   2. 无阻塞 read() 读取 6 字节按键状态
 *   3. 与上次状态对比，边沿检测 (0→1 翻转 = 按键按下)
 *   4. 检测到按下的按键后调用 apply_button_action()
 *   5. 更新 last_buttons 缓冲区
 *
 * 返回值用于 onButtonPoll() 判断是否需要 update() 刷新界面。
 *
 * 容错设计：
 *   - 设备不存在 (PC 端)：fd < 0 → 立即返回 false，无副作用
 *   - read() 无数据 (无按键)：返回 -1 → 返回 false，不阻塞事件循环
 *   - 持续按住不触发：边沿检测只响应 0→1 翻转，1→1 忽略
 */
bool KEY_PollButtons(uint8_t *mode, hal_datetime_t *dt);

#endif // KEY_H
