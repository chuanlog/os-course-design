#pragma once

#include "SchedulingTypes.h"

#include <vector>

namespace scheduling {

SimulationResult simulateFCFS(const std::vector<ProcessInput>& input);

}  // namespace scheduling
