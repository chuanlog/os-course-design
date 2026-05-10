#include "PartitionManager.h"

#include <algorithm>
#include <limits>

namespace memory_management {

PartitionManager::PartitionManager(int totalSize, PartitionStrategy strategy) {
    reset(totalSize, strategy);
}

void PartitionManager::reset(int totalSize, PartitionStrategy strategy) {
    totalSize_ = totalSize;
    strategy_ = strategy;
    freeBlocks_.clear();
    allocated_.clear();
    if (totalSize_ > 0) {
        freeBlocks_.push_back({"Free", 0, totalSize_, false});
    }
}

PartitionOperationResult PartitionManager::allocate(const std::string& name, int size) {
    PartitionOperationResult result;
    const std::string validationMessage = validatePartitionRequest(name, size);
    if (!validationMessage.empty()) {
        result.message = validationMessage;
        result.snapshot = snapshot("分配失败");
        return result;
    }
    if (allocated_.count(name) != 0U) {
        result.message = "进程名称重复，不能重复分配。";
        result.snapshot = snapshot("分配失败");
        return result;
    }

    int selected = -1;
    if (strategy_ == PartitionStrategy::FirstFit) {
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
        result.message = "没有足够大的空闲分区可供分配。";
        result.snapshot = snapshot("分配失败");
        return result;
    }

    MemoryBlock& block = freeBlocks_[selected];
    allocated_[name] = {name, block.start, size, true};
    block.start += size;
    block.size -= size;
    if (block.size == 0) {
        freeBlocks_.erase(freeBlocks_.begin() + selected);
    }
    sortFreeBlocks();

    result.success = true;
    result.message = "分配成功。";
    result.snapshot = snapshot("分配 " + name + " 大小 " + std::to_string(size));
    return result;
}

PartitionOperationResult PartitionManager::release(const std::string& name) {
    PartitionOperationResult result;
    auto it = allocated_.find(name);
    if (it == allocated_.end()) {
        result.message = "未找到对应进程，释放失败。";
        result.snapshot = snapshot("释放失败");
        return result;
    }

    freeBlocks_.push_back({"Free", it->second.start, it->second.size, false});
    allocated_.erase(it);
    mergeFreeBlocks();

    result.success = true;
    result.message = "释放成功。";
    result.snapshot = snapshot("释放 " + name);
    return result;
}

PartitionSnapshot PartitionManager::snapshot(const std::string& action) const {
    PartitionSnapshot state;
    state.action = action;
    for (const auto& [name, block] : allocated_) {
        state.allocatedBlocks.push_back(block);
    }
    state.freeBlocks = freeBlocks_;
    return state;
}

void PartitionManager::sortFreeBlocks() {
    std::sort(freeBlocks_.begin(), freeBlocks_.end(), [](const MemoryBlock& lhs, const MemoryBlock& rhs) {
        return lhs.start < rhs.start;
    });
}

void PartitionManager::mergeFreeBlocks() {
    if (freeBlocks_.empty()) {
        return;
    }
    sortFreeBlocks();
    std::vector<MemoryBlock> merged;
    merged.push_back(freeBlocks_.front());
    for (std::size_t i = 1; i < freeBlocks_.size(); ++i) {
        MemoryBlock& last = merged.back();
        if (last.start + last.size == freeBlocks_[i].start) {
            last.size += freeBlocks_[i].size;
        } else {
            merged.push_back(freeBlocks_[i]);
        }
    }
    freeBlocks_ = std::move(merged);
}

std::string validatePartitionInit(int totalSize) {
    if (totalSize <= 0) {
        return "总内存大小必须大于 0。";
    }
    return {};
}

std::string validatePartitionRequest(const std::string& name, int size) {
    if (name.empty()) {
        return "进程名称不能为空。";
    }
    if (size <= 0) {
        return "申请大小必须大于 0。";
    }
    return {};
}

std::string partitionStrategyName(PartitionStrategy strategy) {
    return strategy == PartitionStrategy::FirstFit ? "FF" : "BF";
}

}  // namespace memory_management
