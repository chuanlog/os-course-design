#pragma once

#include "MemoryTypes.h"

#include <string>
#include <vector>

namespace memory_management {

std::vector<int> parsePageSequence(const std::string& text, std::string* errorMessage);
std::vector<MemoryBlock> buildMemoryLayout(const PartitionSnapshot& snapshot, int totalSize);
std::string blocksToText(const std::vector<MemoryBlock>& blocks);
std::string pageFramesToString(const std::vector<int>& frames);

}  // namespace memory_management
