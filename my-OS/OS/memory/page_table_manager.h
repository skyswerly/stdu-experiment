#ifndef PAGE_TABLE_MANAGER_H
#define PAGE_TABLE_MANAGER_H

#include "i_replacer.h"
#include <vector>

// 页表管理器
class PageTableManager {
public:
    // 单次访问页表结果结构体
    struct AccessResult {
        bool hit;           // 页面是否命中
        int  replacedPage;  // 淘汰页号，-1 表示无页号被淘汰
    };

    // 无参构造函数
    PageTableManager();

    // 初始化页表，frameCount 为主存页框数量
    void init(int frameCount);

    // 访问指定虚拟页号，返回访问结果结构体
    AccessResult access(int pageId, IReplacer* replacer);  // 虚拟页号，指定页面替换算法

    // 获取页框内容用于刷新 UI 表格数据
    std::vector<int> getFrameSnapshot() const;

    // 获取页面访问历史序列
    const std::vector<int>& getHistory() const;

    // 重置页表（清空页表和历史数组）
    void reset();

    // 获取页框数
    int frameCount() const;

private:
    std::vector<int> m_frames;   // 页框数组，m_frames[i] == -1 表示空闲
    std::vector<int> m_history;  // 页面访问历史（按时间顺序）
};

#endif // PAGE_TABLE_MANAGER_H
