/**
 * @file    key.cpp
 * @brief   按键处理模块 — 实现文件 (~85 行)
 *
 * 实现双通道按键输入的底层逻辑：
 *   通道 A：KEY_Process() → apply_button_action() — 纯软件接口
 *   通道 B：KEY_PollButtons() → read(/dev/buttons) → 边沿检测 → apply_button_action()
 *
 * 两条通道最终汇聚到同一个 apply_button_action() 函数，
 * 保证键盘 (PC) 和物理按键 (ARM) 的行为完全一致。
 */

#include "key.h"
#include <fcntl.h>
#include <unistd.h>

/*══════════════════════════════════════════════════════════════════
 * 匿名命名空间 — 按键设备常量与内部函数
 *══════════════════════════════════════════════════════════════════*/
namespace {

/** @brief Tiny4412 按键设备文件路径 */
const char *kButtonsDevice = "/dev/buttons";

/** @brief 按键数量 (Tiny4412 有 6 个板载按键，本应用只用前 3 个) */
const int kButtonCount = 6;

/** @brief 模式切换按键在 buttons 数组中的索引 */
const int kModeButtonIndex = 0;

/** @brief 数值增加按键在 buttons 数组中的索引 */
const int kUpButtonIndex   = 1;

/** @brief 数值减少按键在 buttons 数组中的索引 */
const int kDownButtonIndex = 2;

/**
 * @brief 惰性打开 /dev/buttons 设备 (仅首次调用时打开)
 * @return int  文件描述符，-1 表示设备不可用
 *
 * 使用 static 变量保存 fd，避免每次轮询都重复 open/close。
 * O_NONBLOCK 保证读操作不阻塞事件循环。
 * 设备不存在时返回 -1，后续 KEY_PollButtons 直接返回 false。
 */
int buttons_fd() {
    static int fd = -1;
    if (fd < 0) {
        fd = open(kButtonsDevice, O_RDONLY | O_NONBLOCK);
    }
    return fd;
}

/**
 * @brief 执行按键对应的动作 (两个输入通道共享的核心逻辑)
 * @param mode     [in/out] 工作模式
 * @param dt       [in/out] 时间数据
 * @param key_code         按键: 's'=切换, 'u'=加, 'd'=减
 *
 * 模式切换：
 *   m_mode = (m_mode + 1) % 4  →  0→1→2→3→0 循环
 *
 * 数值调节 (带循环回绕)：
 *   - mode==1 (设时)：hour   ± 1，回绕 0↔23
 *   - mode==2 (设分)：minute ± 1，回绕 0↔59
 *   - mode==3 (设秒)：second ± 1，回绕 0↔59
 *   - mode==0 (正常)：加减操作无效果
 *
 * 减法用 +23/+59/+59 而非 -1 实现，避免 uint8_t 下溢出。
 */
void apply_button_action(uint8_t *mode, hal_datetime_t *dt, char key_code) {
    if (!mode || !dt) return;  // 空指针保护

    if (key_code == 's') {
        // 模式循环切换：0→1→2→3→0→...
        *mode = (*mode + 1) % 4;

    } else if (key_code == 'u') {
        // 数值增加 — 根据当前模式操作不同字段
        if (*mode == 1) {
            dt->hour   = (dt->hour   + 1) % 24;
        } else if (*mode == 2) {
            dt->minute = (dt->minute + 1) % 60;
        } else if (*mode == 3) {
            dt->second = (dt->second + 1) % 60;
        }

    } else if (key_code == 'd') {
        // 数值减少 — 用 +N-1 模拟 -1，避免 uint8_t 下溢出
        if (*mode == 1) {
            dt->hour   = (dt->hour   + 23) % 24;   // hour-1，0→23
        } else if (*mode == 2) {
            dt->minute = (dt->minute + 59) % 60;   // minute-1，0→59
        } else if (*mode == 3) {
            dt->second = (dt->second + 59) % 60;   // second-1，0→59
        }
    }
}

} // namespace

/*══════════════════════════════════════════════════════════════════
 * 公开接口
 *══════════════════════════════════════════════════════════════════*/

/**
 * @brief 软件按键处理 — 直接调用内部动作函数
 *
 * 由 Widget::keyPressEvent 调用，将键盘事件转换为按键动作。
 * 无额外封装，直接转发到 apply_button_action()。
 */
void KEY_Process(uint8_t *mode, hal_datetime_t *dt, char key_code) {
    apply_button_action(mode, dt, key_code);
}

/**
 * @brief 物理按键轮询 — 读取 /dev/buttons + 边沿检测
 *
 * 每 50ms 由 Widget::onButtonPoll() 调用一次。
 *
 * 边沿检测原理：
 *   - last_buttons 保存上一次读取的 6 字节按键状态
 *   - 本次读取到 buttons，与 last 逐字节对比
 *   - '1' 表示按键按下，'0' 表示松开
 *   - 仅当 buttons[i]=='1' && last_buttons[i]!='1' 时触发动作
 *     (下降沿忽略，避免松开时误触发)
 *
 * 返回值：
 *   - true:  检测到按键动作，调用方应 update() 刷新界面
 *   - false: 无新按键、设备不存在、或读取失败
 */
bool KEY_PollButtons(uint8_t *mode, hal_datetime_t *dt) {
    int fd = buttons_fd();
    if (fd < 0) {
        return false;  // 设备不存在 (PC 端)，静默返回
    }

    // 非阻塞读取 6 字节按键状态
    char buttons[kButtonCount] = {0};
    if (read(fd, buttons, sizeof(buttons)) != sizeof(buttons)) {
        return false;  // 读取失败或无数据 (EAGAIN)
    }

    // 保存上一次状态以进行边沿检测
    static char last_buttons[kButtonCount] = {0};
    bool triggered = false;

    // 按键 0 (模式切换)：仅在 0→1 翻转时触发
    if (buttons[kModeButtonIndex] == '1' && last_buttons[kModeButtonIndex] != '1') {
        apply_button_action(mode, dt, 's');
        triggered = true;
    }

    // 按键 1 (数值增加)
    if (buttons[kUpButtonIndex] == '1' && last_buttons[kUpButtonIndex] != '1') {
        apply_button_action(mode, dt, 'u');
        triggered = true;
    }

    // 按键 2 (数值减少)
    if (buttons[kDownButtonIndex] == '1' && last_buttons[kDownButtonIndex] != '1') {
        apply_button_action(mode, dt, 'd');
        triggered = true;
    }

    // 更新历史状态 (用于下次轮询边沿检测)
    for (int i = 0; i < kButtonCount; ++i) {
        last_buttons[i] = buttons[i];
    }

    return triggered;
}
