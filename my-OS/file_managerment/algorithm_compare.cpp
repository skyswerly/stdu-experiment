#include "algorithm_compare.h"
#include "disk_scheduler.h"
#include <algorithm>

std::vector<CompareEntry> compareAll(DiskScheduler& scheduler,
                                     int start_track,
                                     int max_track) {
    std::vector<CompareEntry> entries;

    auto add = [&](const QString& name, const DiskResult& r) {
        entries.push_back({0, name, r.total_distance, r.avg_distance});
    };

    add("FCFS",         scheduler.FCFS(start_track));
    add("SSTF",         scheduler.SSTF(start_track));
    add("SCAN",         scheduler.SCAN(start_track, max_track));
    add("CSCAN",        scheduler.CSCAN(start_track, max_track));
    add("N-Step-SCAN",  scheduler.NStepSCAN(start_track, 5));

    // 按 total_distance 升序排列
    std::sort(entries.begin(), entries.end(),
              [](const CompareEntry& a, const CompareEntry& b) {
                  return a.total_distance < b.total_distance;
              });

    for (int i = 0; i < static_cast<int>(entries.size()); ++i) {
        entries[i].rank = i + 1;
    }

    return entries;
}
