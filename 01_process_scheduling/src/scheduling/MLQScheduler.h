#pragma once

#include "SchedulingTypes.h"

#include <string>
#include <vector>

namespace scheduling {

MLQConfig defaultMLQConfig();
std::string validateMLQConfig(const MLQConfig& config);
SimulationResult simulateMLQ(const std::vector<ProcessInput>& input, const MLQConfig& config);

}  // namespace scheduling
