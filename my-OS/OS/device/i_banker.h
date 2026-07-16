#ifndef I_BANKER_H
#define I_BANKER_H

#include <vector>
#include <string>

// 银行家算法抽象接口
// 所有具体的死锁避免算法均需实现此接口
// 业务层不依赖任何 UI 框架，可独立进行单元测试或对接 GUI
class IBanker {
public:
    virtual ~IBanker() = default;

    // 进程申请资源
    // 按照银行家算法的两步检查 + 安全性预演流程处理请求：
    //   1. 申请量是否超过该进程声明的最大需求量 (NEED)
    //   2. 申请量是否超过系统当前可用量 (AVAILABLE)
    //   3. 试探性分配后执行安全性算法，存在安全序列则确认分配
    // processId  — 申请资源的进程号 (0 ~ M-1)
    // request    — 各类资源的申请数量，长度必须等于资源类型数 N
    // safeSequence — 输出参数：若分配成功，写入安全执行序列；若失败，内容未定义
    // 返回 true 表示分配成功（系统保持安全状态），false 表示分配失败
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

    // 返回算法名称（用于 UI 显示和调试）
    virtual std::string name() const = 0;
};

#endif // I_BANKER_H
