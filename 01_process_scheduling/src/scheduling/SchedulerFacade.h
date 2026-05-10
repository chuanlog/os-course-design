#pragma once

#include "MLFQScheduler.h"
#include "MLQScheduler.h"

#include <vector>

namespace scheduling {

std::vector<SimulationResult> runScheduling(
    const std::vector<ProcessInput>& processes,
    Algorithm algorithm,
    int quantum,
    const MLQConfig& mlqConfig = defaultMLQConfig(),
    const MLFQConfig& mlfqConfig = defaultMLFQConfig());

}  // namespace scheduling
