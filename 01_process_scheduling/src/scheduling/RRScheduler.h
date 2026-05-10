#pragma once

#include "SchedulingTypes.h"

#include <vector>

namespace scheduling {

SimulationResult simulateRR(const std::vector<ProcessInput>& input, int quantum);

}  // namespace scheduling
