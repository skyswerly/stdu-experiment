#include "fifo_replacer.h"

int FifoReplacer::selectVictim(const std::vector<int>& frames,
                                const std::vector<int>& /*history*/) {
    //内部队列取队首
    if (!m_queue.empty()) {
        int victim = m_queue.front();
        m_queue.pop();  // 淘汰的页面从队列中移除
        return victim;
    }
    // 兜底策略：队列为空
    return frames.empty() ? -1 : frames[0];
}

void FifoReplacer::onPageLoaded(int pageId) {
    m_queue.push(pageId);
}

void FifoReplacer::reset() {
    // 清空队列
    m_queue = std::queue<int>();
}
