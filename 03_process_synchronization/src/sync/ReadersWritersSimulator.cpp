#include "ReadersWritersSimulator.h"

#include "EventRecorder.h"

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace sync_demo {

ReadersWritersConfig defaultReadersWritersConfig() {
    return {};
}

std::string validateReadersWritersConfig(const ReadersWritersConfig& config) {
    if (config.readerCount <= 0 || config.writerCount <= 0) {
        return "读者和写者数量都必须大于 0。";
    }
    if (config.readerRounds <= 0 || config.writerRounds <= 0) {
        return "读写轮数必须大于 0。";
    }
    if (config.readerDelayMs < 0 || config.writerDelayMs < 0) {
        return "线程延迟不能为负数。";
    }
    if (config.readerCount > 8 || config.writerCount > 8) {
        return "为便于展示，读者/写者数量请不要超过 8。";
    }
    return {};
}

SimulationResult simulateReadersWriters(const ReadersWritersConfig& config) {
    SimulationResult result;
    result.scenarioName = scenarioName(Scenario::ReadersWriters);

    struct SharedState {
        int data = 0;
        int activeReaders = 0;
        int waitingWriters = 0;
        bool writerActive = false;
        std::mutex mutex;
        std::condition_variable cv;
    } state;

    EventRecorder recorder;
    std::vector<std::thread> threads;

    for (int readerId = 1; readerId <= config.readerCount; ++readerId) {
        threads.emplace_back([&, readerId] {
            recorder.log("读者 R" + std::to_string(readerId), "启动",
                         "计划读取 " + std::to_string(config.readerRounds) + " 轮");
            for (int round = 0; round < config.readerRounds; ++round) {
                int value = 0;
                {
                    std::unique_lock<std::mutex> lock(state.mutex);
                    state.cv.wait(lock, [&] {
                        return !state.writerActive && state.waitingWriters == 0;
                    });
                    ++state.activeReaders;
                    value = state.data;
                    recorder.log("读者 R" + std::to_string(readerId), "开始读取",
                                 "第 " + std::to_string(round + 1) + " 轮，共享数据=" + std::to_string(value));
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(config.readerDelayMs));

                {
                    std::lock_guard<std::mutex> lock(state.mutex);
                    --state.activeReaders;
                    recorder.log("读者 R" + std::to_string(readerId), "结束读取",
                                 "剩余活跃读者=" + std::to_string(state.activeReaders));
                    if (state.activeReaders == 0) {
                        state.cv.notify_all();
                    }
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(config.readerDelayMs / 2));
            }
            recorder.log("读者 R" + std::to_string(readerId), "结束", "已完成全部读取任务");
        });
    }

    for (int writerId = 1; writerId <= config.writerCount; ++writerId) {
        threads.emplace_back([&, writerId] {
            recorder.log("写者 W" + std::to_string(writerId), "启动",
                         "计划写入 " + std::to_string(config.writerRounds) + " 轮");
            for (int round = 0; round < config.writerRounds; ++round) {
                int newValue = 0;
                {
                    std::unique_lock<std::mutex> lock(state.mutex);
                    ++state.waitingWriters;
                    state.cv.wait(lock, [&] {
                        return !state.writerActive && state.activeReaders == 0;
                    });
                    --state.waitingWriters;
                    state.writerActive = true;
                    ++state.data;
                    newValue = state.data;
                    recorder.log("写者 W" + std::to_string(writerId), "开始写入",
                                 "第 " + std::to_string(round + 1) + " 轮，共享数据更新为 " + std::to_string(newValue));
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(config.writerDelayMs));

                {
                    std::lock_guard<std::mutex> lock(state.mutex);
                    state.writerActive = false;
                    recorder.log("写者 W" + std::to_string(writerId), "结束写入",
                                 "当前共享数据=" + std::to_string(state.data));
                }
                state.cv.notify_all();
                std::this_thread::sleep_for(std::chrono::milliseconds(config.writerDelayMs / 2));
            }
            recorder.log("写者 W" + std::to_string(writerId), "结束", "已完成全部写入任务");
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    result.events = recorder.events();
    result.summaryLines.push_back("读者数量: " + std::to_string(config.readerCount) + "，写者数量: " + std::to_string(config.writerCount));
    result.summaryLines.push_back("读者轮数: " + std::to_string(config.readerRounds) + "，写者轮数: " + std::to_string(config.writerRounds));
    result.summaryLines.push_back("最终共享数据值: " + std::to_string(state.data));
    result.summaryLines.push_back("策略: 写者优先，避免写者长期饥饿");
    result.summaryLines.push_back("同步机制: mutex + condition_variable，模拟完成后统一收集日志展示");
    return result;
}

}  // namespace sync_demo
