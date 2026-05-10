#pragma once

#include <string>
#include <vector>

namespace sync_demo {

enum class Scenario {
    ProducerConsumer,
    ReadersWriters,
    DiningPhilosophers,
};

struct EventRecord {
    int order = 0;
    int timeMs = 0;
    std::string actor;
    std::string action;
    std::string detail;
};

struct SimulationResult {
    std::string scenarioName;
    std::vector<EventRecord> events;
    std::vector<std::string> summaryLines;
};

struct ProducerConsumerConfig {
    int bufferCapacity = 4;
    int producerCount = 2;
    int consumerCount = 2;
    int itemsPerProducer = 6;
    int producerDelayMs = 40;
    int consumerDelayMs = 60;
};

struct ReadersWritersConfig {
    int readerCount = 3;
    int writerCount = 2;
    int readerRounds = 3;
    int writerRounds = 3;
    int readerDelayMs = 40;
    int writerDelayMs = 70;
};

struct DiningPhilosophersConfig {
    int philosopherCount = 5;
    int roundsPerPhilosopher = 3;
    int thinkDelayMs = 40;
    int eatDelayMs = 60;
};

std::string scenarioName(Scenario scenario);
std::string buildSummaryText(const SimulationResult& result);

}  // namespace sync_demo
