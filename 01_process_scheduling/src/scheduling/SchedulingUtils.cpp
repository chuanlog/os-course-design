#include "SchedulingUtils.h"

#include <algorithm>

namespace scheduling {

std::vector<ProcessState> resetProcesses(const std::vector<ProcessInput>& input) {
    std::vector<ProcessState> result;
    result.reserve(input.size());
    for (const auto& process : input) {
        result.push_back(ProcessState{
            process.name,
            process.arrival,
            process.burst,
            process.priority,
            process.burst,
            0,
            0,
            0,
            -1,
            false,
        });
    }
    return result;
}

void appendSegment(std::vector<TimelineSegment>& timeline, const std::string& name, int start, int end) {
    if (start == end) {
        return;
    }
    if (!timeline.empty() && timeline.back().name == name && timeline.back().end == start) {
        timeline.back().end = end;
        return;
    }
    timeline.push_back({name, start, end});
}

void calculateMetrics(SimulationResult& result) {
    double totalTurnaround = 0.0;
    double totalWaiting = 0.0;
    double totalResponse = 0.0;

    for (auto& process : result.processes) {
        process.turnaround = process.completion - process.arrival;
        process.waiting = process.turnaround - process.burst;
        totalTurnaround += process.turnaround;
        totalWaiting += process.waiting;
        totalResponse += process.response;
    }

    if (result.processes.empty()) {
        return;
    }

    const double count = static_cast<double>(result.processes.size());
    result.avgTurnaround = totalTurnaround / count;
    result.avgWaiting = totalWaiting / count;
    result.avgResponse = totalResponse / count;
}

std::vector<int> sortByArrivalThenName(const std::vector<ProcessState>& processes) {
    std::vector<int> order(processes.size());
    for (std::size_t i = 0; i < order.size(); ++i) {
        order[i] = static_cast<int>(i);
    }

    std::stable_sort(order.begin(), order.end(), [&](int lhs, int rhs) {
        if (processes[lhs].arrival != processes[rhs].arrival) {
            return processes[lhs].arrival < processes[rhs].arrival;
        }
        return processes[lhs].name < processes[rhs].name;
    });
    return order;
}

std::string validateProcesses(const std::vector<ProcessInput>& processes) {
    if (processes.empty()) {
        return "至少需要添加一个进程。";
    }

    for (const auto& process : processes) {
        if (process.name.empty()) {
            return "进程名称不能为空。";
        }
        if (process.arrival < 0) {
            return "到达时间不能小于 0。";
        }
        if (process.burst <= 0) {
            return "运行时间必须大于 0。";
        }
    }
    return {};
}

}  // namespace scheduling
