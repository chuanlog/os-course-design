#include "RRScheduler.h"

#include "SchedulingUtils.h"

#include <algorithm>
#include <deque>

namespace scheduling {

SimulationResult simulateRR(const std::vector<ProcessInput>& input, int quantum) {
    SimulationResult result;
    result.algorithmName = "RR";
    result.processes = resetProcesses(input);

    const auto order = sortByArrivalThenName(result.processes);
    std::deque<int> readyQueue;
    int currentTime = 0;
    int completed = 0;
    std::size_t nextArrivalIndex = 0;

    auto pushArrivedProcesses = [&](int time) {
        while (nextArrivalIndex < order.size() && result.processes[order[nextArrivalIndex]].arrival <= time) {
            readyQueue.push_back(order[nextArrivalIndex]);
            ++nextArrivalIndex;
        }
    };

    pushArrivedProcesses(currentTime);

    while (completed < static_cast<int>(result.processes.size())) {
        if (readyQueue.empty()) {
            const int nextArrival = result.processes[order[nextArrivalIndex]].arrival;
            appendSegment(result.timeline, "IDLE", currentTime, nextArrival);
            currentTime = nextArrival;
            pushArrivedProcesses(currentTime);
            continue;
        }

        const int index = readyQueue.front();
        readyQueue.pop_front();
        auto& process = result.processes[index];

        if (process.response == -1) {
            process.response = currentTime - process.arrival;
        }

        const int duration = std::min(quantum, process.remaining);
        appendSegment(result.timeline, process.name, currentTime, currentTime + duration);
        currentTime += duration;
        process.remaining -= duration;

        pushArrivedProcesses(currentTime);

        if (process.remaining > 0) {
            readyQueue.push_back(index);
        } else {
            process.completion = currentTime;
            process.finished = true;
            ++completed;
        }
    }

    calculateMetrics(result);
    return result;
}

}  // namespace scheduling
