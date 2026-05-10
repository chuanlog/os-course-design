#pragma once

#include "SyncTypes.h"

#include <string>

namespace sync_demo {

SimulationResult simulateReadersWriters(const ReadersWritersConfig& config);
ReadersWritersConfig defaultReadersWritersConfig();
std::string validateReadersWritersConfig(const ReadersWritersConfig& config);

}  // namespace sync_demo
