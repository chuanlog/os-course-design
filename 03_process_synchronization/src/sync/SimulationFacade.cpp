#include "SimulationFacade.h"

namespace sync_demo {

SimulationResult runScenario(Scenario scenario,
                             const ProducerConsumerConfig& producerConsumerConfig,
                             const ReadersWritersConfig& readersWritersConfig,
                             const DiningPhilosophersConfig& diningConfig,
                             std::string* errorMessage) {
    if (errorMessage != nullptr) {
        errorMessage->clear();
    }

    switch (scenario) {
        case Scenario::ProducerConsumer: {
            const std::string validation = validateProducerConsumerConfig(producerConsumerConfig);
            if (!validation.empty()) {
                if (errorMessage != nullptr) {
                    *errorMessage = validation;
                }
                return {};
            }
            return simulateProducerConsumer(producerConsumerConfig);
        }
        case Scenario::ReadersWriters: {
            const std::string validation = validateReadersWritersConfig(readersWritersConfig);
            if (!validation.empty()) {
                if (errorMessage != nullptr) {
                    *errorMessage = validation;
                }
                return {};
            }
            return simulateReadersWriters(readersWritersConfig);
        }
        case Scenario::DiningPhilosophers: {
            const std::string validation = validateDiningPhilosophersConfig(diningConfig);
            if (!validation.empty()) {
                if (errorMessage != nullptr) {
                    *errorMessage = validation;
                }
                return {};
            }
            return simulateDiningPhilosophers(diningConfig);
        }
        default:
            if (errorMessage != nullptr) {
                *errorMessage = "未知模拟场景。";
            }
            return {};
    }
}

}  // namespace sync_demo
