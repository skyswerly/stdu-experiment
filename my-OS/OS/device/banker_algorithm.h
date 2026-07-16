#ifndef BANKER_ALGORITHM_H
#define BANKER_ALGORITHM_H

#include "i_banker.h"
#include <vector>
#include <string>

// 银行家算法实现（死锁避免）
// 每当进程申请资源时，先进行"试探性分配"，然后执行安全性检查：
//   - 若存在一个安全序列（所有进程都能按序完成并释放资源），则确认分配
//   - 若不存在安全序列，则回滚试探性分配并拒绝请求
class BankerAlgorithm : public IBanker {
public:
    // 构造银行家算法实例
    // max       — 最大需求矩阵 MAX[M][N]，M 个进程对 N 类资源的最大需求量
    // available — 系统初始可用资源向量 AVAILABLE[N]
    BankerAlgorithm(const std::vector<std::vector<int>>& max,
                    const std::vector<int>& available);

    //======IBanker 接口实现======
    bool requestResources(int processId,
                          const std::vector<int>& request,
                          std::vector<int>& safeSequence) override;

    std::vector<int> getAvailable() const override;
    std::vector<std::vector<int>> getAllocation() const override;
    std::vector<std::vector<int>> getNeed() const override;
    const std::vector<std::vector<int>>& getMax() const override;
    int getProcessCount() const override;
    int getResourceCount() const override;
    std::string name() const override;

private:
    // 安全性检查算法
    // 尝试找出一个安全执行序列，使所有进程都能按序获得所需资源并完成
    // safeSequence — 输出参数：安全序列（进程号列表）
    // 返回 true 表示存在安全序列，false 表示不存在
    bool isSafe(std::vector<int>& safeSequence);

    // 向量比较：检查 need 的每一维是否都不超过 work
    // 返回 true 表示 need[j] <= work[j] 对所有 j 均成立
    bool canSatisfy(const std::vector<int>& need, const std::vector<int>& work);

    // 试探性分配：AVAILABLE -= request, ALLOCATION[i] += request, NEED[i] -= request
    void applyAllocation(int pid, const std::vector<int>& request);

    // 回滚试探性分配：恢复 applyAllocation 前的状态
    void rollbackAllocation(int pid, const std::vector<int>& request);

    int m_processCount;                           // 进程数量 M
    int m_resourceCount;                          // 资源类型数量 N
    std::vector<std::vector<int>> m_max;          // 最大需求矩阵 MAX
    std::vector<int> m_available;                 // 可用资源向量 AVAILABLE
    std::vector<std::vector<int>> m_allocation;   // 已分配矩阵 ALLOCATION
    std::vector<std::vector<int>> m_need;         // 需求矩阵 NEED
};

#endif // BANKER_ALGORITHM_H
