#include "banker_algorithm.h"

#include <iostream>
#include <iomanip>
#include <string>

using namespace std;

/** 进程数 */
const int DEFAULT_M = 5;
/** 资源类型数 */
const int DEFAULT_N = 3;

/** 5 个进程对 3 类资源的最大需求量 */
const vector<vector<int>> DEFAULT_MAX = {
    {7, 5, 3},
    {3, 2, 2},
    {9, 0, 2},
    {2, 2, 2},
    {4, 3, 3}
};

/** 系统初始可用资源数 */
const vector<int> DEFAULT_AVAILABLE = {10, 5, 7};

// ============================================================================
// 辅助显示函数
// ============================================================================

/**
 * 打印资源向量
 */
void printVector(const vector<int>& vec, const string& label) {
    cout << "  " << label << ": [ ";
    for (size_t j = 0; j < vec.size(); ++j) {
        cout << vec[j];
        if (j < vec.size() - 1) cout << ", ";
    }
    cout << " ]" << endl;
}

/**
 * 显示完整系统状态
 */
void showStatus(const IBanker& banker) {
    int M = banker.getProcessCount();
    int N = banker.getResourceCount();

    cout << "\n========================================" << endl;
    cout << "         当前系统状态" << endl;
    cout << "========================================" << endl;

    // 可用资源
    cout << "\n系统可用资源向量 (AVAILABLE):" << endl;
    printVector(banker.getAvailable(), "AVA");

    // 最大需求矩阵 MAX
    cout << "\n最大需求矩阵 (MAX):" << endl;
    cout << "      ";
    for (int j = 0; j < N; ++j) cout << " 资源" << j;
    cout << endl;
    for (int i = 0; i < M; ++i) {
        cout << " 进程" << i << ": [ ";
        for (int j = 0; j < N; ++j) {
            cout << setw(4) << left << banker.getMax()[i][j];
        }
        cout << "]" << endl;
    }

    // 已分配矩阵 ALLOCATION
    cout << "\n已分配矩阵 (ALLOCATION):" << endl;
    cout << "      ";
    for (int j = 0; j < N; ++j) cout << " 资源" << j;
    cout << endl;
    const auto& alloc = banker.getAllocation();
    for (int i = 0; i < M; ++i) {
        cout << " 进程" << i << ": [ ";
        for (int j = 0; j < N; ++j) {
            cout << setw(4) << left << alloc[i][j];
        }
        cout << "]" << endl;
    }

    // 需求矩阵 NEED
    cout << "\n需求矩阵 (NEED):" << endl;
    cout << "      ";
    for (int j = 0; j < N; ++j) cout << " 资源" << j;
    cout << endl;
    const auto& need = banker.getNeed();
    for (int i = 0; i < M; ++i) {
        cout << " 进程" << i << ": [ ";
        for (int j = 0; j < N; ++j) {
            cout << setw(4) << left << need[i][j];
        }
        cout << "]" << endl;
    }

    cout << "\n========================================" << endl;
}

/**
 * 交互式资源申请流程
 */
void requestFlow(IBanker& banker) {
    int M = banker.getProcessCount();
    int N = banker.getResourceCount();

    // 输入进程号
    int pid = -1;
    cout << "\n请输入需申请资源的进程号 (0 ~ " << M - 1 << "): ";
    cin >> pid;
    if (cin.fail() || pid < 0 || pid >= M) {
        cout << "错误：进程号无效！" << endl;
        cin.clear();
        cin.ignore(10000, '\n');
        return;
    }

    // 输入各类资源申请量
    vector<int> request(N, 0);
    cout << "请输入进程 " << pid << " 申请的资源数 (共 " << N << " 类):" << endl;
    for (int j = 0; j < N; ++j) {
        cout << "  资源 " << j << ": ";
        cin >> request[j];
        if (cin.fail() || request[j] < 0) {
            cout << "错误：资源数必须为非负整数！" << endl;
            cin.clear();
            cin.ignore(10000, '\n');
            return;
        }
    }

    // 执行申请
    cout << "\n正在处理申请..." << endl;
    vector<int> safeSequence;
    bool granted = banker.requestResources(pid, request, safeSequence);

    if (granted) {
        cout << "\n========================================" << endl;
        cout << "  经安全性检查，系统安全，本次分配成功！" << endl;
        cout << "========================================" << endl;

        // 输出安全序列
        cout << "  安全序列: ";
        for (size_t k = 0; k < safeSequence.size(); ++k) {
            cout << "P" << safeSequence[k];
            if (k < safeSequence.size() - 1) cout << " -> ";
        }
        cout << endl;
    } else {
        cout << "\n========================================" << endl;
        cout << "  系统不安全，本次资源申请被拒绝！" << endl;
        cout << "  系统状态已回滚到申请前。" << endl;
        cout << "========================================" << endl;
    }
}

/**
 * 显示菜单并处理用户选择
 *
 * @return true  继续运行
 * @return false 用户选择退出
 */
bool showMenu(IBanker& banker) {
    cout << "\n┌──────────────────────────────────────┐" << endl;
    cout << "│         银行家算法演示系统            │" << endl;
    cout << "├──────────────────────────────────────┤" << endl;
    cout << "│  1. 申请资源                          │" << endl;
    cout << "│  2. 显示当前系统状态                  │" << endl;
    cout << "│  3. 退出                              │" << endl;
    cout << "└──────────────────────────────────────┘" << endl;
    cout << "请选择操作 (1-3): ";

    int choice = 0;
    cin >> choice;
    if (cin.fail()) {
        cin.clear();
        cin.ignore(10000, '\n');
        cout << "输入无效，请重新选择。" << endl;
        return true;
    }

    switch (choice) {
    case 1:
        requestFlow(banker);
        return true;
    case 2:
        showStatus(banker);
        return true;
    case 3:
        cout << "再见！" << endl;
        return false;
    default:
        cout << "无效选项，请选择 1-3。" << endl;
        return true;
    }
}

// ============================================================================
// 主函数
// ============================================================================

int main() {
    // 设置控制台编码为 UTF-8
    system("chcp 65001 > nul");

    cout << "========================================" << endl;
    cout << "    操作系统小学期实验 — 设备管理" << endl;
    cout << "    银行家算法" << endl;
    cout << "    死锁避免模拟" << endl;
    cout << "========================================" << endl;

    // 使用默认示例数据初始化
    BankerAlgorithm banker(DEFAULT_MAX, DEFAULT_AVAILABLE);

    cout << "\n使用默认示例数据初始化：" << endl;
    cout << "  进程数 M = " << DEFAULT_M << endl;
    cout << "  资源类型数 N = " << DEFAULT_N << endl;
    printVector(DEFAULT_AVAILABLE, "初始可用资源");

    showStatus(banker);

    // 主循环
    while (showMenu(banker)) {
        // 继续循环
    }

    return 0;
}
