#include "MLFQScheduler.h"

#include "SchedulingUtils.h"

#include <algorithm>
#include <deque>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace scheduling {
namespace {

std::string disciplineName(QueueDiscipline discipline) {
    switch (discipline) {
        case QueueDiscipline::FCFS:
            return "FCFS";
        case QueueDiscipline::SJF:
            return "SJF";
        case QueueDiscipline::RR:
            return "RR";
        case QueueDiscipline::Priority:
            return "Priority";
    }
    return "Unknown";
}

std::string formatAlgorithmName(const MLFQConfig& config) {
    std::ostringstream oss;
    oss << "MLFQ[";
    for (std::size_t i = 0; i < config.queues.size(); ++i) {
        if (i != 0) {
            oss << ", ";
        }
        const auto& queue = config.queues[i];
        oss << queue.name << ':' << disciplineName(queue.discipline) << "/Q" << queue.timeSlice;
    }
    oss << ']';
    return oss.str();
}

int selectQueuePosition(const std::deque<int>& readyQueue, const std::vector<ProcessState>& processes, QueueDiscipline discipline) {
    if (readyQueue.empty()) {
        return -1;
    }
    if (discipline == QueueDiscipline::RR || discipline == QueueDiscipline::FCFS) {
        return 0;
    }

    int selectedPos = 0;
    for (int pos = 1; pos < static_cast<int>(readyQueue.size()); ++pos) {
        const auto& candidate = processes[readyQueue[pos]];
        const auto& selected = processes[readyQueue[selectedPos]];

        bool better = false;
        if (discipline == QueueDiscipline::SJF) {
            better = candidate.remaining < selected.remaining ||
                     (candidate.remaining == selected.remaining && candidate.arrival < selected.arrival) ||
                     (candidate.remaining == selected.remaining && candidate.arrival == selected.arrival && candidate.name < selected.name);
        } else if (discipline == QueueDiscipline::Priority) {
            better = candidate.priority < selected.priority ||
                     (candidate.priority == selected.priority && candidate.arrival < selected.arrival) ||
                     (candidate.priority == selected.priority && candidate.arrival == selected.arrival && candidate.name < selected.name);
        }

        if (better) {
            selectedPos = pos;
        }
    }
    return selectedPos;
}

void requeueProcess(std::deque<int>& readyQueue, int index, QueueDiscipline discipline, bool preemptedByHigherQueue) {
    if (preemptedByHigherQueue && discipline == QueueDiscipline::FCFS) {
        readyQueue.push_front(index);
    } else {
        readyQueue.push_back(index);
    }
}

}  // namespace

MLFQConfig defaultMLFQConfig() {
    return MLFQConfig{{
        {"高队列", QueueDiscipline::RR, 2},
        {"中队列", QueueDiscipline::RR, 4},
        {"低队列", QueueDiscipline::FCFS, 8},
    }};
}

std::string validateMLFQConfig(const MLFQConfig& config) {
    if (config.queues.size() != 3) {
        return "MLFQ 当前要求配置 3 个队列。";
    }

    for (const auto& queue : config.queues) {
        if (queue.timeSlice <= 0) {
            return "MLFQ 每个队列的时间片必须大于 0。";
        }
    }
    return {};
}

SimulationResult simulateMLFQ(const std::vector<ProcessInput>& input, const MLFQConfig& config) {
    const std::string validationMessage = validateMLFQConfig(config);
    if (!validationMessage.empty()) {
        throw std::invalid_argument(validationMessage);
    }

    SimulationResult result;
    result.algorithmName = formatAlgorithmName(config);
    result.processes = resetProcesses(input);

    const auto arrivalOrder = sortByArrivalThenName(result.processes);
    std::vector<std::deque<int>> queues(config.queues.size());
    std::vector<int> currentLevels(result.processes.size(), 0);

    int currentTime = 0;
    int completed = 0;
    std::size_t nextArrivalIndex = 0;

    auto pushArrivedProcesses = [&](int time) {
        while (nextArrivalIndex < arrivalOrder.size() && result.processes[arrivalOrder[nextArrivalIndex]].arrival <= time) {
            const int index = arrivalOrder[nextArrivalIndex];
            currentLevels[index] = 0;
            queues[0].push_back(index);
            ++nextArrivalIndex;
        }
    };

    auto nextReadyQueue = [&]() {
        for (int level = 0; level < static_cast<int>(queues.size()); ++level) {
            if (!queues[level].empty()) {
                return level;
            }
        }
        return -1;
    };

    auto higherQueueArrivalTime = [&](int activeLevel) {
        if (activeLevel == 0 || nextArrivalIndex >= arrivalOrder.size()) {
            return std::numeric_limits<int>::max();
        }
        return result.processes[arrivalOrder[nextArrivalIndex]].arrival;
    };

    pushArrivedProcesses(currentTime);

    while (completed < static_cast<int>(result.processes.size())) {
        const int activeLevel = nextReadyQueue();
        if (activeLevel == -1) {
            const int nextArrival = result.processes[arrivalOrder[nextArrivalIndex]].arrival;
            appendSegment(result.timeline, "IDLE", currentTime, nextArrival);
            currentTime = nextArrival;
            pushArrivedProcesses(currentTime);
            continue;
        }

        const auto discipline = config.queues[activeLevel].discipline;
        const int selectedPos = selectQueuePosition(queues[activeLevel], result.processes, discipline);
        const int index = queues[activeLevel][selectedPos];
        queues[activeLevel].erase(queues[activeLevel].begin() + selectedPos);
        auto& process = result.processes[index];

        if (process.response == -1) {
            process.response = currentTime - process.arrival;
        }

        const int timeSlice = config.queues[activeLevel].timeSlice;
        int duration = std::min(timeSlice, process.remaining);
        bool preemptedByHigherQueue = false;

        const int nextHigherArrival = higherQueueArrivalTime(activeLevel);
        if (nextHigherArrival != std::numeric_limits<int>::max() && nextHigherArrival < currentTime + duration) {
            duration = nextHigherArrival - currentTime;
            preemptedByHigherQueue = true;
        }

        duration = std::max(1, duration);
        appendSegment(result.timeline, process.name, currentTime, currentTime + duration);
        currentTime += duration;
        process.remaining -= duration;

        pushArrivedProcesses(currentTime);

        if (process.remaining == 0) {
            process.completion = currentTime;
            process.finished = true;
            ++completed;
            continue;
        }

        if (!preemptedByHigherQueue && duration == timeSlice && activeLevel + 1 < static_cast<int>(queues.size())) {
            currentLevels[index] = activeLevel + 1;
            queues[currentLevels[index]].push_back(index);
        } else {
            requeueProcess(queues[activeLevel], index, discipline, preemptedByHigherQueue);
        }
    }

    calculateMetrics(result);
    return result;
}

}  // namespace scheduling
