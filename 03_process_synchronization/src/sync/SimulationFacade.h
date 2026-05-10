#pragma once

#include "DiningPhilosophersSimulator.h"
#include "ProducerConsumerSimulator.h"
#include "ReadersWritersSimulator.h"

namespace sync_demo {

SimulationResult runScenario(Scenario scenario,
                             const ProducerConsumerConfig& producerConsumerConfig,
                             const ReadersWritersConfig& readersWritersConfig,
                             const DiningPhilosophersConfig& diningConfig,
                             std::string* errorMessage);

}  // namespace sync_demo
