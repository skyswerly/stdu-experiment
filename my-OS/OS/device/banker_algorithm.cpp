#include "banker_algorithm.h"

// ====== 构造 / 析构 ======

BankerAlgorithm::BankerAlgorithm(const std::vector<std::vector<int>>& max,
                                 const std::vector<int>& available)
    : m_max(max)
    , m_available(available)
{
    m_processCount = static_cast<int>(max.size());
    m_resourceCount = m_processCount > 0 ? static_cast<int>(max[0].size()) : 0;

    // 初始时各进程尚未分配任何资源
    m_allocation.assign(m_processCount, std::vector<int>(m_resourceCount, 0));

    // NEED = MAX（因为 ALLOCATION 全为 0）
    m_need = m_max;
}

// ====== 公开接口 ======

bool BankerAlgorithm::requestResources(int processId,
                                        const std::vector<int>& request,
                                        std::vector<int>& safeSequence) {
    // 第 0 步：参数校验
    if (processId < 0 || processId >= m_processCount) {
        return false;
    }
    if (static_cast<int>(request.size()) != m_resourceCount) {
        return false;
    }

    // 第 1 步：申请量不能超过进程声明的最大需求
    for (int j = 0; j < m_resourceCount; ++j) {
        if (request[j] > m_need[processId][j]) {
            return false;  // 申请量超过 NEED，不合理
        }
    }

    // 第 2 步：申请量不能超过系统当前可用资源
    for (int j = 0; j < m_resourceCount; ++j) {
        if (request[j] > m_available[j]) {
            return false;  // 资源暂时不足，需等待
        }
    }

    // 第 3 步：试探性分配
    applyAllocation(processId, request);

    // 第 4 步：安全性检查
    if (isSafe(safeSequence)) {
        return true;  // 系统安全，分配确认
    } else {
        // 系统不安全，回滚并拒绝
        rollbackAllocation(processId, request);
        safeSequence.clear();
        return false;
    }
}

std::vector<int> BankerAlgorithm::getAvailable() const {
    return m_available;
}

std::vector<std::vector<int>> BankerAlgorithm::getAllocation() const {
    return m_allocation;
}

std::vector<std::vector<int>> BankerAlgorithm::getNeed() const {
    return m_need;
}

const std::vector<std::vector<int>>& BankerAlgorithm::getMax() const {
    return m_max;
}

int BankerAlgorithm::getProcessCount() const {
    return m_processCount;
}

int BankerAlgorithm::getResourceCount() const {
    return m_resourceCount;
}

std::string BankerAlgorithm::name() const {
    return "Banker's Algorithm";
}

// ====== 私有方法 ======

bool BankerAlgorithm::isSafe(std::vector<int>& safeSequence) {
    std::vector<int> work = m_available;
    std::vector<bool> finish(m_processCount, false);
    safeSequence.clear();

    // 最多找 m_processCount 轮，每轮尝试找一个可完成的进程
    for (int count = 0; count < m_processCount; ++count) {
        bool found = false;
        for (int i = 0; i < m_processCount; ++i) {
            if (!finish[i] && canSatisfy(m_need[i], work)) {
                // 进程 i 可以完成，释放其已占用资源
                for (int j = 0; j < m_resourceCount; ++j) {
                    work[j] += m_allocation[i][j];
                }
                finish[i] = true;
                safeSequence.push_back(i);
                found = true;
                break;  // 从头重新扫描（保证找到的就是可能的执行顺序）
            }
        }
        if (!found) {
            return false;  // 本轮无进程可完成 → 系统不安全
        }
    }
    return true;  // 所有进程均能完成 → 系统安全
}

bool BankerAlgorithm::canSatisfy(const std::vector<int>& need,
                                  const std::vector<int>& work) {
    for (int j = 0; j < m_resourceCount; ++j) {
        if (need[j] > work[j]) {
            return false;
        }
    }
    return true;
}

void BankerAlgorithm::applyAllocation(int pid, const std::vector<int>& request) {
    for (int j = 0; j < m_resourceCount; ++j) {
        m_available[j] -= request[j];
        m_allocation[pid][j] += request[j];
        m_need[pid][j] -= request[j];
    }
}

void BankerAlgorithm::rollbackAllocation(int pid, const std::vector<int>& request) {
    for (int j = 0; j < m_resourceCount; ++j) {
        m_available[j] += request[j];
        m_allocation[pid][j] -= request[j];
        m_need[pid][j] += request[j];
    }
}
