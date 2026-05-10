#pragma once

#include "SyncTypes.h"

#include <string>

namespace sync_demo {

SimulationResult simulateDiningPhilosophers(const DiningPhilosophersConfig& config);
DiningPhilosophersConfig defaultDiningPhilosophersConfig();
std::string validateDiningPhilosophersConfig(const DiningPhilosophersConfig& config);

}  // namespace sync_demo
