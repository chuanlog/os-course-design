#include "sync/SimulationFacade.h"
#include "sync/SyncTypes.h"

#include <iomanip>
#include <iostream>
#include <limits>
#include <string>

namespace {

void printResult(const sync_demo::SimulationResult& result) {
    std::cout << "\n=== " << result.scenarioName << " ===\n";
    std::cout << std::left << std::setw(8) << "序号"
              << std::setw(12) << "时间(ms)"
              << std::setw(18) << "线程/角色"
              << std::setw(16) << "动作"
              << "详情\n";
    for (const auto& event : result.events) {
        std::cout << std::left << std::setw(8) << event.order
                  << std::setw(12) << event.timeMs
                  << std::setw(18) << event.actor
                  << std::setw(16) << event.action
                  << event.detail << '\n';
    }
    std::cout << '\n' << sync_demo::buildSummaryText(result) << '\n';
}

int readInt(const std::string& prompt, int defaultValue) {
    std::cout << prompt << " [默认 " << defaultValue << "]: ";
    std::string line;
    std::getline(std::cin, line);
    if (line.empty()) {
        return defaultValue;
    }
    try {
        std::size_t parsedLength = 0;
        const int value = std::stoi(line, &parsedLength);
        if (parsedLength == line.size()) {
            return value;
        }
    } catch (...) {
    }
    std::cout << "输入非法，使用默认值。\n";
    return defaultValue;
}

sync_demo::ProducerConsumerConfig promptProducerConsumerConfig() {
    auto config = sync_demo::defaultProducerConsumerConfig();
    config.bufferCapacity = readInt("缓冲区容量", config.bufferCapacity);
    config.producerCount = readInt("生产者数量", config.producerCount);
    config.consumerCount = readInt("消费者数量", config.consumerCount);
    config.itemsPerProducer = readInt("每个生产者生产数量", config.itemsPerProducer);
    config.producerDelayMs = readInt("生产延迟ms", config.producerDelayMs);
    config.consumerDelayMs = readInt("消费延迟ms", config.consumerDelayMs);
    return config;
}

sync_demo::ReadersWritersConfig promptReadersWritersConfig() {
    auto config = sync_demo::defaultReadersWritersConfig();
    config.readerCount = readInt("读者数量", config.readerCount);
    config.writerCount = readInt("写者数量", config.writerCount);
    config.readerRounds = readInt("读者轮数", config.readerRounds);
    config.writerRounds = readInt("写者轮数", config.writerRounds);
    config.readerDelayMs = readInt("读者延迟ms", config.readerDelayMs);
    config.writerDelayMs = readInt("写者延迟ms", config.writerDelayMs);
    return config;
}

sync_demo::DiningPhilosophersConfig promptDiningConfig() {
    auto config = sync_demo::defaultDiningPhilosophersConfig();
    config.philosopherCount = readInt("哲学家数量", config.philosopherCount);
    config.roundsPerPhilosopher = readInt("每位哲学家轮数", config.roundsPerPhilosopher);
    config.thinkDelayMs = readInt("思考延迟ms", config.thinkDelayMs);
    config.eatDelayMs = readInt("进餐延迟ms", config.eatDelayMs);
    return config;
}

void runScenario(sync_demo::Scenario scenario) {
    std::string errorMessage;
    auto producerConsumer = sync_demo::defaultProducerConsumerConfig();
    auto readersWriters = sync_demo::defaultReadersWritersConfig();
    auto dining = sync_demo::defaultDiningPhilosophersConfig();
    if (scenario == sync_demo::Scenario::ProducerConsumer) {
        producerConsumer = promptProducerConsumerConfig();
    } else if (scenario == sync_demo::Scenario::ReadersWriters) {
        readersWriters = promptReadersWritersConfig();
    } else if (scenario == sync_demo::Scenario::DiningPhilosophers) {
        dining = promptDiningConfig();
    }
    const auto result = sync_demo::runScenario(scenario, producerConsumer, readersWriters, dining, &errorMessage);
    if (!errorMessage.empty()) {
        std::cerr << errorMessage << '\n';
        return;
    }
    printResult(result);
}

}  // namespace

int main() {
    while (true) {
        std::cout << "\n=== 线程同步与互斥实验（CLI 备用入口）===\n";
        std::cout << "1. 生产者 - 消费者\n";
        std::cout << "2. 读者 - 写者\n";
        std::cout << "3. 哲学家进餐\n";
        std::cout << "0. 退出\n";
        std::cout << "输入选项: ";

        int choice = 0;
        if (!(std::cin >> choice)) {
            std::cerr << "输入失败。\n";
            return 1;
        }
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        if (choice == 0) {
            break;
        }
        if (choice == 1) {
            runScenario(sync_demo::Scenario::ProducerConsumer);
        } else if (choice == 2) {
            runScenario(sync_demo::Scenario::ReadersWriters);
        } else if (choice == 3) {
            runScenario(sync_demo::Scenario::DiningPhilosophers);
        } else {
            std::cout << "无效选项。\n";
        }
    }
    return 0;
}
