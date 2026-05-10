#pragma once

#include "SchedulingTypes.h"

#include <string>
#include <vector>

namespace scheduling {

MLFQConfig defaultMLFQConfig();
std::string validateMLFQConfig(const MLFQConfig& config);
SimulationResult simulateMLFQ(const std::vector<ProcessInput>& input, const MLFQConfig& config);

}  // namespace scheduling
