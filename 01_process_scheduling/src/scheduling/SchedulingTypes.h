#pragma once

#include <string>
#include <vector>

namespace scheduling {

struct ProcessInput {
    std::string name;
    int arrival = 0;
    int burst = 0;
    int priority = 0;
};

struct ProcessState {
    std::string name;
    int arrival = 0;
    int burst = 0;
    int priority = 0;
    int remaining = 0;
    int completion = 0;
    int turnaround = 0;
    int waiting = 0;
    int response = -1;
    bool finished = false;
};

struct TimelineSegment {
    std::string name;
    int start = 0;
    int end = 0;
};

struct SimulationResult {
    std::string algorithmName;
    std::vector<ProcessState> processes;
    std::vector<TimelineSegment> timeline;
    double avgTurnaround = 0.0;
    double avgWaiting = 0.0;
    double avgResponse = 0.0;
};

enum class QueueDiscipline {
    FCFS,
    SJF,
    RR,
    Priority,
};

struct MLQQueueConfig {
    std::string name;
    int dispatchPriority = 0;
    int timeSlice = 0;
};

struct MLQConfig {
    std::vector<MLQQueueConfig> queues;
};

struct MLFQQueueConfig {
    std::string name;
    QueueDiscipline discipline = QueueDiscipline::RR;
    int timeSlice = 1;
};

struct MLFQConfig {
    std::vector<MLFQQueueConfig> queues;
};

enum class Algorithm {
    FCFS,
    SJF,
    RR,
    Priority,
    MLQ,
    MLFQ,
};

}  // namespace scheduling
