#include "MLQScheduler.h"

#include "SchedulingUtils.h"

#include <algorithm>
#include <deque>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace scheduling {
namespace {

int queueLevelForPriority(int priority) {
    if (priority <= 2) {
        return 0;
    }
    if (priority <= 4) {
        return 1;
    }
    return 2;
}

std::vector<int> sortedQueueOrder(const MLQConfig& config) {
    std::vector<int> order(config.queues.size());
    for (std::size_t i = 0; i < config.queues.size(); ++i) {
        order[i] = static_cast<int>(i);
    }
    std::sort(order.begin(), order.end(), [&](int lhs, int rhs) {
        if (config.queues[lhs].dispatchPriority != config.queues[rhs].dispatchPriority) {
            return config.queues[lhs].dispatchPriority < config.queues[rhs].dispatchPriority;
        }
        return lhs < rhs;
    });
    return order;
}

std::string formatAlgorithmName(const MLQConfig& config) {
    std::ostringstream oss;
    oss << "MLQ[";
    for (std::size_t i = 0; i < config.queues.size(); ++i) {
        if (i != 0) {
            oss << ", ";
        }
        const auto& queue = config.queues[i];
        oss << queue.name << ":P" << queue.dispatchPriority << "/Q" << queue.timeSlice;
    }
    oss << ']';
    return oss.str();
}

}  // namespace

MLQConfig defaultMLQConfig() {
    return MLQConfig{{
        {"高队列", 1, 2},
        {"中队列", 2, 4},
        {"低队列", 3, 0},
    }};
}

std::string validateMLQConfig(const MLQConfig& config) {
    if (config.queues.size() != 3) {
        return "MLQ 当前要求配置 3 个队列。";
    }

    for (std::size_t i = 0; i < config.queues.size(); ++i) {
        const auto& queue = config.queues[i];
        if (queue.dispatchPriority <= 0) {
            return "MLQ 队列优先级必须大于 0。";
        }
        if (queue.timeSlice < 0) {
            return "MLQ 时间片不能小于 0，填 0 表示按 FCFS 运行。";
        }
        for (std::size_t j = i + 1; j < config.queues.size(); ++j) {
            if (queue.dispatchPriority == config.queues[j].dispatchPriority) {
                return "MLQ 队列优先级不能重复。";
            }
        }
    }

    return {};
}

SimulationResult simulateMLQ(const std::vector<ProcessInput>& input, const MLQConfig& config) {
    const std::string validationMessage = validateMLQConfig(config);
    if (!validationMessage.empty()) {
        throw std::invalid_argument(validationMessage);
    }

    SimulationResult result;
    result.algorithmName = formatAlgorithmName(config);
    result.processes = resetProcesses(input);

    const auto processOrder = sortByArrivalThenName(result.processes);
    const auto queueOrder = sortedQueueOrder(config);
    std::vector<std::deque<int>> queues(config.queues.size());
    std::vector<int> processQueues(result.processes.size(), 2);
    for (std::size_t i = 0; i < result.processes.size(); ++i) {
        processQueues[i] = queueLevelForPriority(result.processes[i].priority);
    }

    int currentTime = 0;
    int completed = 0;
    std::size_t nextArrivalIndex = 0;

    auto pushArrivedProcesses = [&](int time) {
        while (nextArrivalIndex < processOrder.size() && result.processes[processOrder[nextArrivalIndex]].arrival <= time) {
            const int index = processOrder[nextArrivalIndex];
            queues[processQueues[index]].push_back(index);
            ++nextArrivalIndex;
        }
    };

    auto readyQueueByPriority = [&]() -> int {
        for (int queueIndex : queueOrder) {
            if (!queues[queueIndex].empty()) {
                return queueIndex;
            }
        }
        return -1;
    };

    auto higherPriorityArrivalTime = [&](int activeQueue) {
        int nextTime = std::numeric_limits<int>::max();
        const int activeDispatchPriority = config.queues[activeQueue].dispatchPriority;
        for (std::size_t i = nextArrivalIndex; i < processOrder.size(); ++i) {
            const int processIndex = processOrder[i];
            const int queueIndex = processQueues[processIndex];
            if (config.queues[queueIndex].dispatchPriority < activeDispatchPriority) {
                nextTime = result.processes[processIndex].arrival;
                break;
            }
        }
        return nextTime;
    };

    pushArrivedProcesses(currentTime);

    while (completed < static_cast<int>(result.processes.size())) {
        const int activeQueue = readyQueueByPriority();
        if (activeQueue == -1) {
            const int nextArrival = result.processes[processOrder[nextArrivalIndex]].arrival;
            appendSegment(result.timeline, "IDLE", currentTime, nextArrival);
            currentTime = nextArrival;
            pushArrivedProcesses(currentTime);
            continue;
        }

        const int index = queues[activeQueue].front();
        queues[activeQueue].pop_front();
        auto& process = result.processes[index];

        if (process.response == -1) {
            process.response = currentTime - process.arrival;
        }

        int duration = process.remaining;
        const int configuredTimeSlice = config.queues[activeQueue].timeSlice;
        if (configuredTimeSlice > 0) {
            duration = std::min(duration, configuredTimeSlice);
        }

        const int nextHigherArrival = higherPriorityArrivalTime(activeQueue);
        if (nextHigherArrival != std::numeric_limits<int>::max()) {
            duration = std::min(duration, nextHigherArrival - currentTime);
        }

        duration = std::max(1, duration);
        appendSegment(result.timeline, process.name, currentTime, currentTime + duration);
        currentTime += duration;
        process.remaining -= duration;

        pushArrivedProcesses(currentTime);

        if (process.remaining > 0) {
            queues[activeQueue].push_back(index);
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
