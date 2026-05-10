#include "SJFScheduler.h"

#include "SchedulingUtils.h"

#include <algorithm>
#include <limits>

namespace scheduling {

SimulationResult simulateSJF(const std::vector<ProcessInput>& input) {
    SimulationResult result;
    result.algorithmName = "SJF";
    result.processes = resetProcesses(input);

    const int total = static_cast<int>(result.processes.size());
    int completed = 0;
    int currentTime = 0;

    while (completed < total) {
        int selected = -1;
        for (int i = 0; i < total; ++i) {
            const auto& process = result.processes[i];
            if (process.finished || process.arrival > currentTime) {
                continue;
            }
            if (selected == -1 || process.burst < result.processes[selected].burst ||
                (process.burst == result.processes[selected].burst && process.arrival < result.processes[selected].arrival) ||
                (process.burst == result.processes[selected].burst && process.arrival == result.processes[selected].arrival && process.name < result.processes[selected].name)) {
                selected = i;
            }
        }

        if (selected == -1) {
            int nextArrival = std::numeric_limits<int>::max();
            for (const auto& process : result.processes) {
                if (!process.finished) {
                    nextArrival = std::min(nextArrival, process.arrival);
                }
            }
            appendSegment(result.timeline, "IDLE", currentTime, nextArrival);
            currentTime = nextArrival;
            continue;
        }

        auto& process = result.processes[selected];
        process.response = currentTime - process.arrival;
        appendSegment(result.timeline, process.name, currentTime, currentTime + process.burst);
        currentTime += process.burst;
        process.completion = currentTime;
        process.finished = true;
        ++completed;
    }

    calculateMetrics(result);
    return result;
}

}  // namespace scheduling
