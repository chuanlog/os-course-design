#include "MemoryFacade.h"

#include <algorithm>
#include <sstream>
#include <string>
#include <vector>

namespace memory_management {

std::vector<int> parsePageSequence(const std::string& text, std::string* errorMessage) {
    std::vector<int> pages;
    std::istringstream iss(text);
    int value = 0;
    while (iss >> value) {
        pages.push_back(value);
    }
    if (pages.empty() && errorMessage != nullptr) {
        *errorMessage = "请输入以空格分隔的页面访问序列。";
    }
    return pages;
}

std::vector<MemoryBlock> buildMemoryLayout(const PartitionSnapshot& snapshot, int totalSize) {
    std::vector<MemoryBlock> layout = snapshot.allocatedBlocks;
    layout.insert(layout.end(), snapshot.freeBlocks.begin(), snapshot.freeBlocks.end());
    std::sort(layout.begin(), layout.end(), [](const MemoryBlock& lhs, const MemoryBlock& rhs) {
        return lhs.start < rhs.start;
    });

    std::vector<MemoryBlock> normalized;
    int cursor = 0;
    for (const auto& block : layout) {
        if (cursor < block.start) {
            normalized.push_back({"Free", cursor, block.start - cursor, false});
        }
        normalized.push_back(block);
        cursor = block.start + block.size;
    }
    if (cursor < totalSize) {
        normalized.push_back({"Free", cursor, totalSize - cursor, false});
    }
    return normalized;
}

std::string blocksToText(const std::vector<MemoryBlock>& blocks) {
    std::ostringstream oss;
    for (const auto& block : blocks) {
        oss << block.name << " [" << block.start << ", " << (block.start + block.size) << ") 大小=" << block.size;
        if (!block.allocated) {
            oss << " (空闲)";
        }
        oss << '\n';
    }
    return oss.str();
}

std::string pageFramesToString(const std::vector<int>& frames) {
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

}  // namespace memory_management
