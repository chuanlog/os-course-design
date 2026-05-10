#pragma once

#include "SyncTypes.h"

#include <string>

namespace sync_demo {

SimulationResult simulateProducerConsumer(const ProducerConsumerConfig& config);
ProducerConsumerConfig defaultProducerConsumerConfig();
std::string validateProducerConsumerConfig(const ProducerConsumerConfig& config);

}  // namespace sync_demo
