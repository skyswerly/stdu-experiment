#ifndef I_REPLACER_H
#define I_REPLACER_H

#include <vector>
#include <string>

// 页面置换算法抽象接口
class IReplacer {
public:
    virtual ~IReplacer() = default;

    // 从当前页表中选择一个进行淘汰
    virtual int selectVictim(const std::vector<int>& frames,    // 当前页表
                             const std::vector<int>& history) = 0;  // 历史访问数组

    // 页面装入通知
    virtual void onPageLoaded(int pageId) = 0;

    // 重置算法状态（切换算法，清空输出表，重设参数）
    virtual void reset() = 0;

    // 返回算法名称
    virtual std::string name() const = 0;
};

#endif // I_REPLACER_H
