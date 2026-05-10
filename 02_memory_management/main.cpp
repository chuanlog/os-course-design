#include <algorithm>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <sstream>
#include <string>
#include <vector>

struct Block {
    int start = 0;
    int size = 0;
};

class PartitionManager {
public:
    enum class Strategy {
        FirstFit,
        BestFit
    };

    PartitionManager(int totalSize, Strategy strategy)
        : totalSize_(totalSize), strategy_(strategy) {
        freeBlocks_.push_back({0, totalSize_});
    }

    bool allocate(const std::string& name, int size) {
        if (size <= 0 || allocated_.count(name) != 0U) {
            return false;
        }

        int selected = -1;
        if (strategy_ == Strategy::FirstFit) {
            for (std::size_t i = 0; i < freeBlocks_.size(); ++i) {
                if (freeBlocks_[i].size >= size) {
                    selected = static_cast<int>(i);
                    break;
                }
            }
        } else {
            int bestSize = std::numeric_limits<int>::max();
            for (std::size_t i = 0; i < freeBlocks_.size(); ++i) {
                if (freeBlocks_[i].size >= size && freeBlocks_[i].size < bestSize) {
                    bestSize = freeBlocks_[i].size;
                    selected = static_cast<int>(i);
                }
            }
        }

        if (selected == -1) {
            return false;
        }

        Block& block = freeBlocks_[selected];
        allocated_[name] = {block.start, size};
        block.start += size;
        block.size -= size;

        if (block.size == 0) {
            freeBlocks_.erase(freeBlocks_.begin() + selected);
        }
        sortFreeBlocks();
        return true;
    }

    bool release(const std::string& name) {
        auto it = allocated_.find(name);
        if (it == allocated_.end()) {
            return false;
        }
        freeBlocks_.push_back(it->second);
        allocated_.erase(it);
        mergeFreeBlocks();
        return true;
    }

    void show() const {
        std::cout << "\n已分配分区:\n";
        if (allocated_.empty()) {
            std::cout << "(空)\n";
        } else {
            std::cout << std::left << std::setw(12) << "进程"
                      << std::setw(12) << "起始地址"
                      << std::setw(12) << "大小" << '\n';
            for (const auto& [name, block] : allocated_) {
                std::cout << std::left << std::setw(12) << name
                          << std::setw(12) << block.start
                          << std::setw(12) << block.size << '\n';
            }
        }

        std::cout << "\n空闲分区:\n";
        if (freeBlocks_.empty()) {
            std::cout << "(空)\n";
        } else {
            std::cout << std::left << std::setw(12) << "起始地址"
                      << std::setw(12) << "大小" << '\n';
            for (const auto& block : freeBlocks_) {
                std::cout << std::left << std::setw(12) << block.start
                          << std::setw(12) << block.size << '\n';
            }
        }
    }

private:
    int totalSize_ = 0;
    Strategy strategy_ = Strategy::FirstFit;
    std::vector<Block> freeBlocks_;
    std::map<std::string, Block> allocated_;

    void sortFreeBlocks() {
        std::sort(freeBlocks_.begin(), freeBlocks_.end(), [](const Block& lhs, const Block& rhs) {
            return lhs.start < rhs.start;
        });
    }

    void mergeFreeBlocks() {
        if (freeBlocks_.empty()) {
            return;
        }
        sortFreeBlocks();
        std::vector<Block> merged;
        merged.push_back(freeBlocks_.front());
        for (std::size_t i = 1; i < freeBlocks_.size(); ++i) {
            Block& last = merged.back();
            if (last.start + last.size == freeBlocks_[i].start) {
                last.size += freeBlocks_[i].size;
            } else {
                merged.push_back(freeBlocks_[i]);
            }
        }
        freeBlocks_ = std::move(merged);
    }
};

std::string framesToString(const std::vector<int>& frames) {
    std::ostringstream oss;
    oss << '[';
    for (std::size_t i = 0; i < frames.size(); ++i) {
        if (i != 0) {
            oss << ' ';
        }
        if (frames[i] == -1) {
            oss << '-';
        } else {
            oss << frames[i];
        }
    }
    oss << ']';
    return oss.str();
}

void simulateFIFO(int frameCount, const std::vector<int>& pages) {
    std::vector<int> frames(frameCount, -1);
    std::vector<int> loadedOrder;
    int pointer = 0;
    int faults = 0;

    std::cout << "\nFIFO 页面置换过程:\n";
    std::cout << std::left << std::setw(10) << "访问页"
              << std::setw(20) << "页框状态"
              << std::setw(10) << "结果" << '\n';

    for (int page : pages) {
        bool hit = std::find(frames.begin(), frames.end(), page) != frames.end();
        if (!hit) {
            ++faults;
            auto emptyIt = std::find(frames.begin(), frames.end(), -1);
            if (emptyIt != frames.end()) {
                *emptyIt = page;
            } else {
                frames[pointer] = page;
                pointer = (pointer + 1) % frameCount;
            }
        }
        std::cout << std::left << std::setw(10) << page
                  << std::setw(20) << framesToString(frames)
                  << std::setw(10) << (hit ? "命中" : "缺页") << '\n';
    }

    std::cout << "缺页次数: " << faults << '\n';
    std::cout << std::fixed << std::setprecision(2)
              << "缺页率: " << (static_cast<double>(faults) / pages.size()) * 100.0 << "%\n";
}

void simulateLRU(int frameCount, const std::vector<int>& pages) {
    std::vector<int> frames(frameCount, -1);
    std::vector<int> lastUsed(frameCount, -1);
    int faults = 0;

    std::cout << "\nLRU 页面置换过程:\n";
    std::cout << std::left << std::setw(10) << "访问页"
              << std::setw(20) << "页框状态"
              << std::setw(10) << "结果" << '\n';

    for (std::size_t time = 0; time < pages.size(); ++time) {
        int page = pages[time];
        bool hit = false;
        for (int i = 0; i < frameCount; ++i) {
            if (frames[i] == page) {
                hit = true;
                lastUsed[i] = static_cast<int>(time);
                break;
            }
        }

        if (!hit) {
            ++faults;
            int selected = -1;
            for (int i = 0; i < frameCount; ++i) {
                if (frames[i] == -1) {
                    selected = i;
                    break;
                }
            }
            if (selected == -1) {
                selected = static_cast<int>(std::min_element(lastUsed.begin(), lastUsed.end()) - lastUsed.begin());
            }
            frames[selected] = page;
            lastUsed[selected] = static_cast<int>(time);
        }

        std::cout << std::left << std::setw(10) << page
                  << std::setw(20) << framesToString(frames)
                  << std::setw(10) << (hit ? "命中" : "缺页") << '\n';
    }

    std::cout << "缺页次数: " << faults << '\n';
    std::cout << std::fixed << std::setprecision(2)
              << "缺页率: " << (static_cast<double>(faults) / pages.size()) * 100.0 << "%\n";
}

void runPartitionDemo() {
    std::cout << "请输入总内存大小: ";
    int totalSize = 0;
    std::cin >> totalSize;
    if (totalSize <= 0) {
        std::cerr << "总内存大小非法。\n";
        return;
    }

    std::cout << "选择分配策略: 1.FF  2.BF\n";
    int choice = 0;
    std::cin >> choice;

    PartitionManager::Strategy strategy = (choice == 2)
        ? PartitionManager::Strategy::BestFit
        : PartitionManager::Strategy::FirstFit;
    PartitionManager manager(totalSize, strategy);

    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cout << "输入命令: alloc name size | free name | show | exit\n";

    while (true) {
        std::cout << "mem> ";
        std::string line;
        if (!std::getline(std::cin, line)) {
            break;
        }
        std::istringstream iss(line);
        std::string command;
        iss >> command;

        if (command == "alloc") {
            std::string name;
            int size = 0;
            iss >> name >> size;
            if (manager.allocate(name, size)) {
                std::cout << "分配成功。\n";
            } else {
                std::cout << "分配失败。\n";
            }
        } else if (command == "free") {
            std::string name;
            iss >> name;
            if (manager.release(name)) {
                std::cout << "释放成功。\n";
            } else {
                std::cout << "释放失败。\n";
            }
        } else if (command == "show") {
            manager.show();
        } else if (command == "exit") {
            break;
        } else if (!command.empty()) {
            std::cout << "未知命令。\n";
        }
    }
}

void runPageReplacementDemo() {
    std::cout << "请输入页框数量: ";
    int frameCount = 0;
    std::cin >> frameCount;
    if (frameCount <= 0) {
        std::cerr << "页框数量非法。\n";
        return;
    }

    std::cout << "请输入页面访问序列长度: ";
    int n = 0;
    std::cin >> n;
    if (n <= 0) {
        std::cerr << "访问序列长度非法。\n";
        return;
    }

    std::vector<int> pages(n);
    std::cout << "请输入页面访问序列:\n";
    for (int i = 0; i < n; ++i) {
        std::cin >> pages[i];
    }

    std::cout << "选择算法: 1.FIFO  2.LRU  3.两者对比\n";
    int choice = 0;
    std::cin >> choice;

    if (choice == 1 || choice == 3) {
        simulateFIFO(frameCount, pages);
    }
    if (choice == 2 || choice == 3) {
        simulateLRU(frameCount, pages);
    }
}

int main() {
    while (true) {
        std::cout << "\n=== 内存管理实验 ===\n";
        std::cout << "1. 动态分区分配（FF / BF）\n";
        std::cout << "2. 页面置换（FIFO / LRU）\n";
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
            runPartitionDemo();
        } else if (choice == 2) {
            runPageReplacementDemo();
        } else {
            std::cout << "无效选项。\n";
        }
    }
    return 0;
}
