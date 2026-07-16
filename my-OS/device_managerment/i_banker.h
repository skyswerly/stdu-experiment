#ifndef I_BANKER_H
#define I_BANKER_H

#include <vector>
#include <string>

/**
 * 银行家算法抽象接口
 */
class IBanker {
public:
    virtual ~IBanker() = default;

    //
    virtual bool requestResources(int processId,
                                  const std::vector<int>& request,
                                  std::vector<int>& safeSequence) = 0;

    // 获取当前系统可用资源向量
    virtual std::vector<int> getAvailable() const = 0;

    // 获取当前已分配矩阵
    virtual std::vector<std::vector<int>> getAllocation() const = 0;

    // 获取当前需求矩阵 (NEED = MAX - ALLOCATION)
    virtual std::vector<std::vector<int>> getNeed() const = 0;

    // 获取最大需求矩阵 (MAX)，只读引用
    virtual const std::vector<std::vector<int>>& getMax() const = 0;

    // 获取进程数量 M
    virtual int getProcessCount() const = 0;

    // 获取资源类型数量 N
    virtual int getResourceCount() const = 0;

    // 返回算法名称
    virtual std::string name() const = 0;
};

#endif // I_BANKER_H
