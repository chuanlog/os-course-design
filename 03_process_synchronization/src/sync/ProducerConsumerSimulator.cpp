#include "ProducerConsumerSimulator.h"

#include "EventRecorder.h"

#include <chrono>
#include <condition_variable>
#include <deque>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace sync_demo {
namespace {

using namespace std::chrono_literals;

class Buffer {
public:
    Buffer(int capacity, EventRecorder* recorder)
        : capacity_(capacity), recorder_(recorder) {}

    void produce(int item, int producerId) {
        std::unique_lock<std::mutex> lock(mutex_);
        notFull_.wait(lock, [&] { return static_cast<int>(queue_.size()) < capacity_; });
        queue_.push_back(item);
        if (recorder_ != nullptr) {
            recorder_->log("生产者 P" + std::to_string(producerId), "放入产品",
                           "item=" + std::to_string(item) + ", 缓冲区大小=" + std::to_string(queue_.size()));
        }
        lock.unlock();
        notEmpty_.notify_one();
    }

    int consume(int consumerId) {
        std::unique_lock<std::mutex> lock(mutex_);
        notEmpty_.wait(lock, [&] { return !queue_.empty(); });
        const int item = queue_.front();
        queue_.pop_front();
        if (recorder_ != nullptr) {
            recorder_->log("消费者 C" + std::to_string(consumerId), "取出产品",
                           "item=" + std::to_string(item) + ", 缓冲区大小=" + std::to_string(queue_.size()));
        }
        lock.unlock();
        notFull_.notify_one();
        return item;
    }

    int size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return static_cast<int>(queue_.size());
    }

private:
    int capacity_ = 0;
    EventRecorder* recorder_ = nullptr;
    mutable std::mutex mutex_;
    std::deque<int> queue_;
    std::condition_variable notFull_;
    std::condition_variable notEmpty_;
};

}  // namespace

ProducerConsumerConfig defaultProducerConsumerConfig() {
    return {};
}

std::string validateProducerConsumerConfig(const ProducerConsumerConfig& config) {
    if (config.bufferCapacity <= 0) {
        return "缓冲区容量必须大于 0。";
    }
    if (config.producerCount <= 0 || config.consumerCount <= 0) {
        return "生产者和消费者数量都必须大于 0。";
    }
    if (config.itemsPerProducer <= 0) {
        return "每个生产者生产数量必须大于 0。";
    }
    if (config.producerDelayMs < 0 || config.consumerDelayMs < 0) {
        return "线程延迟不能为负数。";
    }
    if (config.producerCount > 8 || config.consumerCount > 8) {
        return "为便于展示，生产者/消费者数量请不要超过 8。";
    }
    return {};
}

SimulationResult simulateProducerConsumer(const ProducerConsumerConfig& config) {
    SimulationResult result;
    result.scenarioName = scenarioName(Scenario::ProducerConsumer);

    EventRecorder recorder;
    Buffer buffer(config.bufferCapacity, &recorder);
    std::vector<std::thread> threads;

    const int totalItems = config.producerCount * config.itemsPerProducer;
    const int baseConsume = totalItems / config.consumerCount;
    const int remainder = totalItems % config.consumerCount;

    for (int producerId = 1; producerId <= config.producerCount; ++producerId) {
        threads.emplace_back([&, producerId] {
            recorder.log("生产者 P" + std::to_string(producerId), "启动",
                         "计划生产 " + std::to_string(config.itemsPerProducer) + " 个产品");
            for (int i = 0; i < config.itemsPerProducer; ++i) {
                std::this_thread::sleep_for(std::chrono::milliseconds(config.producerDelayMs));
                const int item = producerId * 100 + i;
                buffer.produce(item, producerId);
            }
            recorder.log("生产者 P" + std::to_string(producerId), "结束", "已完成全部生产任务");
        });
    }

    for (int consumerId = 1; consumerId <= config.consumerCount; ++consumerId) {
        const int consumeTarget = baseConsume + (consumerId <= remainder ? 1 : 0);
        threads.emplace_back([&, consumerId, consumeTarget] {
            recorder.log("消费者 C" + std::to_string(consumerId), "启动",
                         "计划消费 " + std::to_string(consumeTarget) + " 个产品");
            for (int i = 0; i < consumeTarget; ++i) {
                std::this_thread::sleep_for(std::chrono::milliseconds(config.consumerDelayMs));
                (void)buffer.consume(consumerId);
            }
            recorder.log("消费者 C" + std::to_string(consumerId), "结束", "已完成全部消费任务");
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }

    result.events = recorder.events();
    result.summaryLines.push_back("缓冲区容量: " + std::to_string(config.bufferCapacity));
    result.summaryLines.push_back("生产者数量: " + std::to_string(config.producerCount) + "，消费者数量: " + std::to_string(config.consumerCount));
    result.summaryLines.push_back("总生产产品数: " + std::to_string(totalItems));
    result.summaryLines.push_back("最终缓冲区大小: " + std::to_string(buffer.size()));
    result.summaryLines.push_back("同步机制: mutex + condition_variable，生产和消费完成后统一收集日志展示");
    return result;
}

}  // namespace sync_demo
