#include "page_table_manager.h"
#include <algorithm>

//无参构造函数
PageTableManager::PageTableManager() {
}

//初始化页表，frameCount为主存页框数量
void PageTableManager::init(int frameCount) {
    m_frames.assign(frameCount, -1);  // -1 表示该页框空闲
    m_history.clear();//清空历史数组
}

//访问指定虚拟页号，返回访问结果结构体
PageTableManager::AccessResult PageTableManager::access(int pageId,
                                                         IReplacer* replacer) {
    //1：查找页面是否已在页框中
    for (int i = 0; i < static_cast<int>(m_frames.size()); i++) {
        if (m_frames[i] == pageId) {
            // 命中：记录访问历史，不改变页框内容
            m_history.push_back(pageId);
            return {true, -1};
        }
    }

    //步骤 2：查找空闲页框(未命中)
    for (int i = 0; i < static_cast<int>(m_frames.size()); i++) {
        if (m_frames[i] == -1) {
            // 有空闲页框：直接调入，无需淘汰
            m_frames[i] = pageId;
            m_history.push_back(pageId);
            replacer->onPageLoaded(pageId);
            return {false, -1};
        }
    }

    //步骤 3：页表已满，需要淘汰
    int victim = replacer->selectVictim(m_frames, m_history);

    // 在页框中找到被淘汰的页面，替换为新页面
    for (int i = 0; i < static_cast<int>(m_frames.size()); i++) {
        if (m_frames[i] == victim) {
            m_frames[i] = pageId;
            break;
        }
    }

    m_history.push_back(pageId);
    replacer->onPageLoaded(pageId);

    return {false, victim};
}

//获取页框内容用于刷新UI表格数据
std::vector<int> PageTableManager::getFrameSnapshot() const {
    return m_frames;
}

//获取页面访问历史序列
const std::vector<int>& PageTableManager::getHistory() const {
    return m_history;
}

//重置页表（清空页表和历史数组）
void PageTableManager::reset() {
    std::fill(m_frames.begin(), m_frames.end(), -1);
    m_history.clear();
}

//获取页框数
int PageTableManager::frameCount() const {
    return static_cast<int>(m_frames.size());
}
