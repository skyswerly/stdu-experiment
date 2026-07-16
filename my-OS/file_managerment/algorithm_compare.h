#pragma once

#include <QString>
#include <vector>

struct CompareEntry {
    int rank;
    QString name;
    int total_distance;
    double avg_distance;
};

// 前向声明，避免引入 Qt 依赖的业务层头文件
class DiskScheduler;

/// 对全部 5 种算法执行并返回按 total_distance 升序排列的结果
std::vector<CompareEntry> compareAll(DiskScheduler& scheduler,
                                     int start_track,
                                     int max_track);
