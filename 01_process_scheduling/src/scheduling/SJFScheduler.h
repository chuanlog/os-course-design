#pragma once

#include "SchedulingTypes.h"

#include <vector>

namespace scheduling {

SimulationResult simulateSJF(const std::vector<ProcessInput>& input);

}  // namespace scheduling
