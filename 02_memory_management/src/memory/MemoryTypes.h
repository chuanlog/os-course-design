#pragma once

#include <string>
#include <vector>

namespace memory_management {

enum class PartitionStrategy {
    FirstFit,
    BestFit,
};

enum class PageAlgorithm {
    FIFO,
    LRU,
    LFU,
    CLOCK,
    Random,
};

struct MemoryBlock {
    std::string name;
    int start = 0;
    int size = 0;
    bool allocated = false;
};

struct PartitionSnapshot {
    std::string action;
    std::vector<MemoryBlock> allocatedBlocks;
    std::vector<MemoryBlock> freeBlocks;
};

struct PartitionOperationResult {
    bool success = false;
    std::string message;
    PartitionSnapshot snapshot;
};

struct PageStep {
    int page = 0;
    std::vector<int> frames;
    bool hit = false;
};

struct PageReplacementResult {
    std::string algorithmName;
    std::vector<PageStep> steps;
    int pageFaults = 0;
    double pageFaultRate = 0.0;
};

}  // namespace memory_management
