#pragma once

#include <vector>

struct DiskResult {
    std::vector<int> seek_sequence;  // 寻道访问顺序
    int total_distance = 0;           // 总寻道距离
    double avg_distance = 0.0;        // 平均寻道长度
    int direction = 0;                // 0=向内(减少), 1=向外(增加)
};

class DiskScheduler {
public:
    void generateRequests(int count, int max_track);

    DiskResult FCFS(int start_track);
    DiskResult SSTF(int start_track);
    DiskResult SCAN(int start_track, int max_track);
    DiskResult CSCAN(int start_track, int max_track);
    DiskResult NStepSCAN(int start_track, int step_size);

    std::vector<int> requests;
};
