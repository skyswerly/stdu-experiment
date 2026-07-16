#include "disk_scheduler.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <functional>

void DiskScheduler::generateRequests(int count, int max_track) {
    requests.clear();
    requests.reserve(count);
    for (int i = 0; i < count; ++i) {
        requests.push_back(std::rand() % (max_track + 1));
    }
}

DiskResult DiskScheduler::FCFS(int start_track) {
    DiskResult result;
    int current = start_track;
    int total = 0;

    for (int r : requests) {
        int dist = std::abs(r - current);
        total += dist;
        result.seek_sequence.push_back(r);
        current = r;
    }

    result.total_distance = total;
    result.avg_distance = requests.empty() ? 0.0 : static_cast<double>(total) / requests.size();
    result.direction = 0;
    return result;
}

DiskResult DiskScheduler::SSTF(int start_track) {
    DiskResult result;
    std::vector<bool> visited(requests.size(), false);
    int current = start_track;
    int total = 0;
    int n = static_cast<int>(requests.size());

    for (int i = 0; i < n; ++i) {
        int min_dist = 2147483647;  // INT_MAX
        int min_idx = -1;

        for (int j = 0; j < n; ++j) {
            if (visited[j]) continue;
            int dist = std::abs(requests[j] - current);
            if (dist < min_dist) {
                min_dist = dist;
                min_idx = j;
            }
        }

        if (min_idx == -1) break;

        total += min_dist;
        current = requests[min_idx];
        result.seek_sequence.push_back(current);
        visited[min_idx] = true;
    }

    result.total_distance = total;
    result.avg_distance = requests.empty() ? 0.0 : static_cast<double>(total) / requests.size();
    result.direction = 0;
    return result;
}

DiskResult DiskScheduler::SCAN(int start_track, int max_track) {
    DiskResult result;
    std::vector<int> req = requests;
    std::sort(req.begin(), req.end());

    int current = start_track;
    int total = 0;

    // 确定初始方向：找最近的请求，若 >= start_track 则向外，否则向内
    int dir = 1; // 默认向外
    int nearest_dist = 2147483647;
    int nearest_idx = -1;
    for (int i = 0; i < static_cast<int>(req.size()); ++i) {
        int dist = std::abs(req[i] - current);
        if (dist < nearest_dist) {
            nearest_dist = dist;
            nearest_idx = i;
        }
    }
    if (nearest_idx >= 0 && req[nearest_idx] < current) {
        dir = 0; // 向内
    }

    // 按方向分组
    std::vector<int> outward, inward;
    for (int r : req) {
        if (r >= current)
            outward.push_back(r);
        else
            inward.push_back(r);
    }
    // 向外组升序，向内组降序
    std::sort(outward.begin(), outward.end());
    std::sort(inward.begin(), inward.end(), std::greater<int>());

    std::vector<int> order;
    if (dir == 1) {
        // 先向外
        for (int r : outward) order.push_back(r);
        for (int r : inward) order.push_back(r);
    } else {
        // 先向内
        for (int r : inward) order.push_back(r);
        for (int r : outward) order.push_back(r);
    }

    for (int r : order) {
        total += std::abs(r - current);
        current = r;
        result.seek_sequence.push_back(r);
    }

    result.total_distance = total;
    result.avg_distance = requests.empty() ? 0.0 : static_cast<double>(total) / requests.size();
    result.direction = dir;
    return result;
}

DiskResult DiskScheduler::CSCAN(int start_track, int max_track) {
    DiskResult result;
    std::vector<int> req = requests;
    std::sort(req.begin(), req.end());

    int current = start_track;
    int total = 0;

    // 分组：>= current 和 < current
    std::vector<int> right, left;
    for (int r : req) {
        if (r >= current)
            right.push_back(r);
        else
            left.push_back(r);
    }

    // 先处理右侧，然后跳回最小再处理左侧
    for (int r : right) {
        total += std::abs(r - current);
        current = r;
        result.seek_sequence.push_back(r);
    }

    if (!left.empty()) {
        // 跳到最小磁道号（0 或 left 的第一个）
        int low = left.front();
        total += std::abs(current - low);
        current = low;

        for (int r : left) {
            total += std::abs(r - current);
            current = r;
            result.seek_sequence.push_back(r);
        }
    }

    result.total_distance = total;
    result.avg_distance = requests.empty() ? 0.0 : static_cast<double>(total) / requests.size();
    result.direction = 1; // CSCAN 始终向外
    return result;
}

DiskResult DiskScheduler::NStepSCAN(int start_track, int step_size) {
    DiskResult result;
    int total = 0;
    int current = start_track;
    int n = static_cast<int>(requests.size());

    if (step_size <= 0) step_size = 1;

    for (int base = 0; base < n; base += step_size) {
        int end = std::min(base + step_size, n);

        // 取出当前段的请求
        std::vector<int> segment(requests.begin() + base, requests.begin() + end);
        std::sort(segment.begin(), segment.end());

        // 确定方向（与 SCAN 一致）
        int dir = 1;
        if (!segment.empty() && segment[0] < current) {
            // 有比 current 小的 → 检查最近的
            int nearest_dist = 2147483647;
            int nearest_idx = -1;
            for (int i = 0; i < static_cast<int>(segment.size()); ++i) {
                int dist = std::abs(segment[i] - current);
                if (dist < nearest_dist) {
                    nearest_dist = dist;
                    nearest_idx = i;
                }
            }
            if (nearest_idx >= 0 && segment[nearest_idx] < current) {
                dir = 0;
            }
        }

        std::vector<int> outward, inward;
        for (int r : segment) {
            if (r >= current)
                outward.push_back(r);
            else
                inward.push_back(r);
        }
        std::sort(outward.begin(), outward.end());
        std::sort(inward.begin(), inward.end(), std::greater<int>());

        std::vector<int> order;
        if (dir == 1) {
            for (int r : outward) order.push_back(r);
            for (int r : inward) order.push_back(r);
        } else {
            for (int r : inward) order.push_back(r);
            for (int r : outward) order.push_back(r);
        }

        for (int r : order) {
            total += std::abs(r - current);
            current = r;
            result.seek_sequence.push_back(r);
        }
    }

    result.total_distance = total;
    result.avg_distance = requests.empty() ? 0.0 : static_cast<double>(total) / requests.size();
    result.direction = 0;
    return result;
}
