#pragma once

#include "SchedulingTypes.h"

#include <vector>

namespace scheduling {

SimulationResult simulatePriority(const std::vector<ProcessInput>& input);

}  // namespace scheduling
