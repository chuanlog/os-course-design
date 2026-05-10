#include "FCFSScheduler.h"

#include "SchedulingUtils.h"

namespace scheduling {

SimulationResult simulateFCFS(const std::vector<ProcessInput>& input) {
    SimulationResult result;
    result.algorithmName = "FCFS";
    result.processes = resetProcesses(input);

    const auto order = sortByArrivalThenName(result.processes);
    int currentTime = 0;

    for (int index : order) {
        auto& process = result.processes[index];
        if (currentTime < process.arrival) {
            appendSegment(result.timeline, "IDLE", currentTime, process.arrival);
            currentTime = process.arrival;
        }
        process.response = currentTime - process.arrival;
        appendSegment(result.timeline, process.name, currentTime, currentTime + process.burst);
        currentTime += process.burst;
        process.completion = currentTime;
        process.finished = true;
    }

    calculateMetrics(result);
    return result;
}

}  // namespace scheduling
