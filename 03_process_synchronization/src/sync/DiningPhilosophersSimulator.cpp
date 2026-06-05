X#include "DiningPhilosophersSimulator.h"

#include "EventRecorder.h"

#include <algorithm>
#include <chrono>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace sync_demo {

DiningPhilosophersConfig defaultDiningPhilosophersConfig() {
    return {};
}

std::string validateDiningPhilosophersConfig(const DiningPhilosophersConfig& config) {
    if (config.philosopherCount < 2) {
        return "哲学家数量至少为 2。";
    }
    if (config.roundsPerPhilosopher <= 0) {
        return "每位哲学家的轮数必须大于 0。";
    }
    if (config.thinkDelayMs < 0 || config.eatDelayMs < 0) {
        return "线程延迟不能为负数。";
    }
    if (config.philosopherCount > 10) {
        return "为便于展示，哲学家数量请不要超过 10。";
    }
    return {};
}

SimulationResult simulateDiningPhilosophers(const DiningPhilosophersConfig& config) {
    SimulationResult result;
    result.scenarioName = scenarioName(Scenario::DiningPhilosophers);

    EventRecorder recorder;
    std::vector<std::mutex> forks(static_cast<std::size_t>(config.philosopherCount));
    std::vector<std::thread> philosophers;

    for (int id = 0; id < config.philosopherCount; ++id) {
        philosophers.emplace_back([&, id] {
            const int left = id;
            const int right = (id + 1) % config.philosopherCount;
            const int first = std::min(left, right);
            const int second = std::max(left, right);

            recorder.log("哲学家 " + std::to_string(id), "启动",
                         "计划进餐 " + std::to_string(config.roundsPerPhilosopher) + " 轮");

            for (int round = 0; round < config.roundsPerPhilosopher; ++round) {
                recorder.log("哲学家 " + std::to_string(id), "思考",
                             "第 " + std::to_string(round + 1) + " 轮");
                std::this_thread::sleep_for(std::chrono::milliseconds(config.thinkDelayMs));

                std::lock_guard<std::mutex> firstLock(forks[static_cast<std::size_t>(first)]);
                recorder.log("哲学家 " + std::to_string(id), "拿起叉子", std::to_string(first));
                std::lock_guard<std::mutex> secondLock(forks[static_cast<std::size_t>(second)]);
                recorder.log("哲学家 " + std::to_string(id), "拿起叉子", std::to_string(second));
                recorder.log("哲学家 " + std::to_string(id), "进餐",
                             "第 " + std::to_string(round + 1) + " 轮");
                std::this_thread::sleep_for(std::chrono::milliseconds(config.eatDelayMs));
                recorder.log("哲学家 " + std::to_string(id), "放下叉子",
                             std::to_string(first) + " 和 " + std::to_string(second));
            }

            recorder.log("哲学家 " + std::to_string(id), "结束", "已完成全部轮次");
        });
    }

    for (auto& thread : philosophers) {
        thread.join();
    }

    result.events = recorder.events();
    result.summaryLines.push_back("哲学家数量: " + std::to_string(config.philosopherCount));
    result.summaryLines.push_back("每位哲学家轮数: " + std::to_string(config.roundsPerPhilosopher));
    result.summaryLines.push_back("总进餐次数: " + std::to_string(config.philosopherCount * config.roundsPerPhilosopher));
    result.summaryLines.push_back("策略: 总是先拿编号较小的叉子，再拿编号较大的叉子，避免死锁");
    result.summaryLines.push_back("同步机制: mutex，模拟完成后统一收集日志展示");
    return result;
}

}  // namespace sync_demo
