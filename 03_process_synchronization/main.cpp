#include <chrono>
#include <condition_variable>
#include <deque>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

using namespace std::chrono_literals;

std::mutex logMutex;

void logLine(const std::string& message) {
    std::lock_guard<std::mutex> lock(logMutex);
    std::cout << message << std::endl;
}

void runProducerConsumer() {
    logLine("\n=== 生产者 - 消费者问题 ===");

    class Buffer {
    public:
        explicit Buffer(std::size_t capacity) : capacity_(capacity) {}

        void produce(int item, int producerId) {
            std::unique_lock<std::mutex> lock(mutex_);
            notFull_.wait(lock, [&] { return queue_.size() < capacity_; });
            queue_.push_back(item);
            logLine("生产者 P" + std::to_string(producerId) + " 放入产品 " + std::to_string(item) +
                    "，缓冲区大小=" + std::to_string(queue_.size()));
            lock.unlock();
            notEmpty_.notify_one();
        }

        int consume(int consumerId) {
            std::unique_lock<std::mutex> lock(mutex_);
            notEmpty_.wait(lock, [&] { return !queue_.empty(); });
            int item = queue_.front();
            queue_.pop_front();
            logLine("消费者 C" + std::to_string(consumerId) + " 取出产品 " + std::to_string(item) +
                    "，缓冲区大小=" + std::to_string(queue_.size()));
            lock.unlock();
            notFull_.notify_one();
            return item;
        }

    private:
        std::size_t capacity_ = 0;
        std::deque<int> queue_;
        std::mutex mutex_;
        std::condition_variable notFull_;
        std::condition_variable notEmpty_;
    };

    Buffer buffer(4);
    constexpr int producerCount = 2;
    constexpr int consumerCount = 2;
    constexpr int itemsPerProducer = 6;
    constexpr int itemsPerConsumer = producerCount * itemsPerProducer / consumerCount;

    std::vector<std::thread> threads;

    for (int p = 0; p < producerCount; ++p) {
        threads.emplace_back([&, p] {
            for (int i = 0; i < itemsPerProducer; ++i) {
                std::this_thread::sleep_for(60ms);
                int item = p * 100 + i;
                buffer.produce(item, p + 1);
            }
        });
    }

    for (int c = 0; c < consumerCount; ++c) {
        threads.emplace_back([&, c] {
            for (int i = 0; i < itemsPerConsumer; ++i) {
                std::this_thread::sleep_for(90ms);
                (void)buffer.consume(c + 1);
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }
    logLine("生产者 - 消费者示例结束。\n");
}

void runReadersWriters() {
    logLine("\n=== 读者 - 写者问题 ===");

    struct SharedState {
        int data = 0;
        int activeReaders = 0;
        int waitingWriters = 0;
        bool writerActive = false;
        std::mutex mutex;
        std::condition_variable cv;
    } state;

    std::vector<std::thread> threads;

    for (int readerId = 1; readerId <= 3; ++readerId) {
        threads.emplace_back([&, readerId] {
            for (int i = 0; i < 3; ++i) {
                int value = 0;
                {
                    std::unique_lock<std::mutex> lock(state.mutex);
                    state.cv.wait(lock, [&] {
                        return !state.writerActive && state.waitingWriters == 0;
                    });
                    ++state.activeReaders;
                    value = state.data;
                }

                logLine("读者 R" + std::to_string(readerId) + " 正在读取，共享数据=" + std::to_string(value));
                std::this_thread::sleep_for(70ms);

                {
                    std::lock_guard<std::mutex> lock(state.mutex);
                    --state.activeReaders;
                    if (state.activeReaders == 0) {
                        state.cv.notify_all();
                    }
                }
                std::this_thread::sleep_for(50ms);
            }
        });
    }

    for (int writerId = 1; writerId <= 2; ++writerId) {
        threads.emplace_back([&, writerId] {
            for (int i = 0; i < 3; ++i) {
                {
                    std::unique_lock<std::mutex> lock(state.mutex);
                    ++state.waitingWriters;
                    state.cv.wait(lock, [&] {
                        return !state.writerActive && state.activeReaders == 0;
                    });
                    --state.waitingWriters;
                    state.writerActive = true;
                    ++state.data;
                    logLine("写者 W" + std::to_string(writerId) + " 更新共享数据为 " + std::to_string(state.data));
                }

                std::this_thread::sleep_for(100ms);

                {
                    std::lock_guard<std::mutex> lock(state.mutex);
                    state.writerActive = false;
                }
                state.cv.notify_all();
                std::this_thread::sleep_for(80ms);
            }
        });
    }

    for (auto& thread : threads) {
        thread.join();
    }
    logLine("读者 - 写者示例结束。\n");
}

void runDiningPhilosophers() {
    logLine("\n=== 哲学家进餐问题 ===");

    constexpr int philosopherCount = 5;
    std::vector<std::mutex> forks(philosopherCount);
    std::vector<std::thread> philosophers;

    for (int id = 0; id < philosopherCount; ++id) {
        philosophers.emplace_back([&, id] {
            const int left = id;
            const int right = (id + 1) % philosopherCount;
            const int first = std::min(left, right);
            const int second = std::max(left, right);

            for (int round = 0; round < 3; ++round) {
                logLine("哲学家 " + std::to_string(id) + " 正在思考");
                std::this_thread::sleep_for(50ms);

                std::lock_guard<std::mutex> firstLock(forks[first]);
                logLine("哲学家 " + std::to_string(id) + " 拿起叉子 " + std::to_string(first));
                std::lock_guard<std::mutex> secondLock(forks[second]);
                logLine("哲学家 " + std::to_string(id) + " 拿起叉子 " + std::to_string(second));
                logLine("哲学家 " + std::to_string(id) + " 正在吃饭");
                std::this_thread::sleep_for(70ms);
                logLine("哲学家 " + std::to_string(id) + " 放下叉子并结束本轮进餐");
            }
        });
    }

    for (auto& thread : philosophers) {
        thread.join();
    }
    logLine("哲学家进餐示例结束。\n");
}

int main() {
    while (true) {
        std::cout << "=== 进程同步与并发控制实验 ===\n";
        std::cout << "1. 生产者 - 消费者\n";
        std::cout << "2. 读者 - 写者\n";
        std::cout << "3. 哲学家进餐\n";
        std::cout << "4. 依次运行全部示例\n";
        std::cout << "0. 退出\n";
        std::cout << "输入选项: ";

        int choice = 0;
        if (!(std::cin >> choice)) {
            std::cerr << "输入失败。\n";
            return 1;
        }

        if (choice == 0) {
            break;
        }
        if (choice == 1) {
            runProducerConsumer();
        } else if (choice == 2) {
            runReadersWriters();
        } else if (choice == 3) {
            runDiningPhilosophers();
        } else if (choice == 4) {
            runProducerConsumer();
            runReadersWriters();
            runDiningPhilosophers();
        } else {
            std::cout << "无效选项。\n";
        }
    }
    return 0;
}
