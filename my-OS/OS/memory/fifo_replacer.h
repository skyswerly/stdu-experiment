#ifndef FIFO_REPLACER_H
#define FIFO_REPLACER_H

#include "i_replacer.h"
#include <queue>

// FIFO 先进先出页面置换算法
class FifoReplacer : public IReplacer {
public:
    int selectVictim(const std::vector<int>& frames,
                     const std::vector<int>& history) override;

    void onPageLoaded(int pageId) override;

    void reset() override;

    std::string name() const override { return "FIFO"; }

private:
    std::queue<int> m_queue;  // 页面装入顺序队列
};

#endif // FIFO_REPLACER_H
