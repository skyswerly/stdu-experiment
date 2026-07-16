#include "lru_replacer.h"
#include <climits>

int LruReplacer::selectVictim(const std::vector<int>& frames,
                               const std::vector<int>& history) {
    int victim = -1;
    int earliestPos = INT_MAX;  // 最靠前 = 最久未使用

    // 遍历每个页框中的页面，确定谁最久未被访问
    for (int page : frames) {
        // 在历史序列中从后向前搜索 page 的最后一次出现
        int lastPos = -1;  // -1 表示从未被访问
        for (int i = static_cast<int>(history.size()) - 1; i >= 0; i--) {
            if (history[i] == page) {
                lastPos = i;
                break;
            }
        }

        // 选取最久未使用的页面
        if (lastPos < earliestPos) {
            earliestPos = lastPos;
            victim = page;
        }
    }

    return victim;
}
