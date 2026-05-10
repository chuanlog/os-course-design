#pragma once

#include "SchedulingTypes.h"

#include <string>
#include <vector>

namespace scheduling {

std::vector<ProcessState> resetProcesses(const std::vector<ProcessInput>& input);
void appendSegment(std::vector<TimelineSegment>& timeline, const std::string& name, int start, int end);
void calculateMetrics(SimulationResult& result);
std::vector<int> sortByArrivalThenName(const std::vector<ProcessState>& processes);
std::string validateProcesses(const std::vector<ProcessInput>& processes);

}  // namespace scheduling
