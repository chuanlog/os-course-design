#pragma once

#include "MemoryTypes.h"

#include <string>
#include <vector>

namespace memory_management {

PageReplacementResult simulateFIFO(int frameCount, const std::vector<int>& pages);
PageReplacementResult simulateLRU(int frameCount, const std::vector<int>& pages);
PageReplacementResult simulateLFU(int frameCount, const std::vector<int>& pages);
PageReplacementResult simulateCLOCK(int frameCount, const std::vector<int>& pages);
PageReplacementResult simulateRandom(int frameCount, const std::vector<int>& pages);
PageReplacementResult simulatePageReplacement(PageAlgorithm algorithm, int frameCount, const std::vector<int>& pages);
std::string validatePageInput(int frameCount, const std::vector<int>& pages);
std::string pageAlgorithmName(PageAlgorithm algorithm);

}  // namespace memory_management
