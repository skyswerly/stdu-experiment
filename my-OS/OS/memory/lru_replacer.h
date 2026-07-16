#ifndef LRU_REPLACER_H
#define LRU_REPLACER_H

#include "i_replacer.h"

// LRU 最近最少使用页面置换算法
class LruReplacer : public IReplacer {
public:
    int selectVictim(const std::vector<int>& frames,
                     const std::vector<int>& history) override;

    void onPageLoaded(int /*pageId*/) override {
        // LRU 所有信息来自 history，无需额外状态维护
    }

    void reset() override {
        // 无内部状态需要重置
    }

    std::string name() const override { return "LRU"; }
};

#endif // LRU_REPLACER_H
