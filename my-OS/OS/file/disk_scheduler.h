#ifndef DISK_SCHEDULER_H
#define DISK_SCHEDULER_H

#include <vector>

// 寻道结果结构体
struct DiskResult {
    std::vector<int> seek_sequence;  // 寻道访问顺序
    int total_distance = 0;           // 总寻道距离
    double avg_distance = 0.0;        // 平均寻道长度
    int direction = 0;                // 0=向内(减少), 1=向外(增加)
};

// 磁盘调度器
class DiskScheduler {
public:
    // 随机生成 count 个磁道请求（范围 0 ~ max_track）
    void generateRequests(int count, int max_track);

    // 先来先服务算法
    DiskResult FCFS(int start_track);
    // 最短寻道时间优先算法
    DiskResult SSTF(int start_track);
    // 扫描算法（电梯算法）
    DiskResult SCAN(int start_track, int max_track);
    // 循环扫描算法
    DiskResult CSCAN(int start_track, int max_track);
    // N 步扫描算法
    DiskResult NStepSCAN(int start_track, int step_size);

    std::vector<int> requests;  // 请求序列
};

#endif // DISK_SCHEDULER_H
