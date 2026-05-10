#pragma once

#include "MemoryTypes.h"

#include <map>
#include <string>
#include <vector>

namespace memory_management {

class PartitionManager {
public:
    PartitionManager() = default;
    PartitionManager(int totalSize, PartitionStrategy strategy);

    void reset(int totalSize, PartitionStrategy strategy);
    PartitionOperationResult allocate(const std::string& name, int size);
    PartitionOperationResult release(const std::string& name);
    PartitionSnapshot snapshot(const std::string& action) const;

    int totalSize() const { return totalSize_; }
    PartitionStrategy strategy() const { return strategy_; }

private:
    int totalSize_ = 0;
    PartitionStrategy strategy_ = PartitionStrategy::FirstFit;
    std::vector<MemoryBlock> freeBlocks_;
    std::map<std::string, MemoryBlock> allocated_;

    void sortFreeBlocks();
    void mergeFreeBlocks();
};

std::string validatePartitionInit(int totalSize);
std::string validatePartitionRequest(const std::string& name, int size);
std::string partitionStrategyName(PartitionStrategy strategy);

}  // namespace memory_management
