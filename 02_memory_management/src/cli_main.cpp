#include "memory/MemoryFacade.h"
#include "memory/PageReplacement.h"
#include "memory/PartitionManager.h"

#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

namespace {

void printPartitionSnapshot(const memory_management::PartitionSnapshot& snapshot) {
    std::cout << "\n=== " << snapshot.action << " ===\n";
    std::cout << "\n已分配分区:\n";
    if (snapshot.allocatedBlocks.empty()) {
        std::cout << "(空)\n";
    } else {
        std::cout << std::left << std::setw(12) << "名称"
                  << std::setw(12) << "起始"
                  << std::setw(12) << "大小" << '\n';
        for (const auto& block : snapshot.allocatedBlocks) {
            std::cout << std::left << std::setw(12) << block.name
                      << std::setw(12) << block.start
                      << std::setw(12) << block.size << '\n';
        }
    }

    std::cout << "\n空闲分区:\n";
    if (snapshot.freeBlocks.empty()) {
        std::cout << "(空)\n";
    } else {
        std::cout << std::left << std::setw(12) << "起始"
                  << std::setw(12) << "大小" << '\n';
        for (const auto& block : snapshot.freeBlocks) {
            std::cout << std::left << std::setw(12) << block.start
                      << std::setw(12) << block.size << '\n';
        }
    }
}

void runPartitionCli() {
    int totalSize = 0;
    std::cout << "请输入总内存大小: ";
    std::cin >> totalSize;
    const std::string validationMessage = memory_management::validatePartitionInit(totalSize);
    if (!validationMessage.empty()) {
        std::cerr << validationMessage << '\n';
        return;
    }

    int choice = 0;
    std::cout << "选择分配策略: 1.FF  2.BF\n";
    std::cin >> choice;
    const auto strategy = (choice == 2) ? memory_management::PartitionStrategy::BestFit : memory_management::PartitionStrategy::FirstFit;
    memory_management::PartitionManager manager(totalSize, strategy);

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
            const auto result = manager.allocate(name, size);
            std::cout << result.message << '\n';
            printPartitionSnapshot(result.snapshot);
        } else if (command == "free") {
            std::string name;
            iss >> name;
            const auto result = manager.release(name);
            std::cout << result.message << '\n';
            printPartitionSnapshot(result.snapshot);
        } else if (command == "show") {
            printPartitionSnapshot(manager.snapshot("当前状态"));
        } else if (command == "exit") {
            break;
        } else if (!command.empty()) {
            std::cout << "未知命令。\n";
        }
    }
}

void printPageResult(const memory_management::PageReplacementResult& result) {
    std::cout << "\n=== " << result.algorithmName << " 页面置换结果 ===\n";
    std::cout << std::left << std::setw(10) << "访问页"
              << std::setw(24) << "页框状态"
              << std::setw(10) << "结果" << '\n';
    for (const auto& step : result.steps) {
        std::cout << std::left << std::setw(10) << step.page
                  << std::setw(24) << memory_management::pageFramesToString(step.frames)
                  << std::setw(10) << (step.hit ? "命中" : "缺页") << '\n';
    }
    std::cout << "缺页次数: " << result.pageFaults << '\n';
    std::cout << std::fixed << std::setprecision(2) << "缺页率: " << result.pageFaultRate << "%\n";
}

void runPagingCli() {
    int frameCount = 0;
    std::cout << "请输入页框数量: ";
    std::cin >> frameCount;
    std::cout << "请输入页面访问序列长度: ";
    int n = 0;
    std::cin >> n;
    if (n <= 0) {
        std::cerr << "访问序列长度必须大于 0。\n";
        return;
    }
    std::vector<int> pages(n);
    std::cout << "请输入页面访问序列:\n";
    for (int i = 0; i < n; ++i) {
        std::cin >> pages[i];
    }
    const std::string validationMessage = memory_management::validatePageInput(frameCount, pages);
    if (!validationMessage.empty()) {
        std::cerr << validationMessage << '\n';
        return;
    }

    std::cout << "选择算法: 1.FIFO  2.LRU  3.LFU  4.CLOCK  5.RANDOM  6.全部对比\n";
    int choice = 0;
    std::cin >> choice;
    if (choice == 1 || choice == 6) {
        printPageResult(memory_management::simulateFIFO(frameCount, pages));
    }
    if (choice == 2 || choice == 6) {
        printPageResult(memory_management::simulateLRU(frameCount, pages));
    }
    if (choice == 3 || choice == 6) {
        printPageResult(memory_management::simulateLFU(frameCount, pages));
    }
    if (choice == 4 || choice == 6) {
        printPageResult(memory_management::simulateCLOCK(frameCount, pages));
    }
    if (choice == 5 || choice == 6) {
        printPageResult(memory_management::simulateRandom(frameCount, pages));
    }
}

}  // namespace

int main() {
    while (true) {
        std::cout << "\n=== 内存管理实验（CLI 备用入口）===\n";
        std::cout << "1. 动态分区分配（FF / BF）\n";
        std::cout << "2. 页面置换（FIFO / LRU / LFU / CLOCK / RANDOM）\n";
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
            runPartitionCli();
        } else if (choice == 2) {
            runPagingCli();
        } else {
            std::cout << "无效选项。\n";
        }
    }
    return 0;
}
