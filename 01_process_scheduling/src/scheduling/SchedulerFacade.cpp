#include "SchedulerFacade.h"

#include "FCFSScheduler.h"
#include "MLFQScheduler.h"
#include "MLQScheduler.h"
#include "PriorityScheduler.h"
#include "RRScheduler.h"
#include "SJFScheduler.h"

#include <stdexcept>

namespace scheduling {

std::vector<SimulationResult> runScheduling(
    const std::vector<ProcessInput>& processes,
    Algorithm algorithm,
    int quantum,
    const MLQConfig& mlqConfig,
    const MLFQConfig& mlfqConfig) {
    std::vector<SimulationResult> results;
    switch (algorithm) {
        case Algorithm::FCFS:
            results.push_back(simulateFCFS(processes));
            break;
        case Algorithm::SJF:
            results.push_back(simulateSJF(processes));
            break;
        case Algorithm::RR:
            if (quantum <= 0) {
                throw std::invalid_argument("RR 时间片必须大于 0。");
            }
            results.push_back(simulateRR(processes, quantum));
            break;
        case Algorithm::Priority:
            results.push_back(simulatePriority(processes));
            break;
        case Algorithm::MLQ:
            results.push_back(simulateMLQ(processes, mlqConfig));
            break;
        case Algorithm::MLFQ:
            results.push_back(simulateMLFQ(processes, mlfqConfig));
            break;
    }
    return results;
}

}  // namespace scheduling
