#include "PageReplacement.h"

#include <algorithm>
#include <random>
#include <vector>

namespace memory_management {
namespace {

void finalizeResult(PageReplacementResult& result, int faults, std::size_t totalPages) {
    result.pageFaults = faults;
    if (totalPages != 0U) {
        result.pageFaultRate = static_cast<double>(faults) / static_cast<double>(totalPages) * 100.0;
    }
}

}  // namespace

PageReplacementResult simulateFIFO(int frameCount, const std::vector<int>& pages) {
    PageReplacementResult result;
    result.algorithmName = "FIFO";

    std::vector<int> frames(frameCount, -1);
    int pointer = 0;
    int faults = 0;

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
        result.steps.push_back({page, frames, hit});
    }

    finalizeResult(result, faults, pages.size());
    return result;
}

PageReplacementResult simulateLRU(int frameCount, const std::vector<int>& pages) {
    PageReplacementResult result;
    result.algorithmName = "LRU";

    std::vector<int> frames(frameCount, -1);
    std::vector<int> lastUsed(frameCount, -1);
    int faults = 0;

    for (std::size_t time = 0; time < pages.size(); ++time) {
        const int page = pages[time];
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

        result.steps.push_back({page, frames, hit});
    }

    finalizeResult(result, faults, pages.size());
    return result;
}

PageReplacementResult simulateLFU(int frameCount, const std::vector<int>& pages) {
    PageReplacementResult result;
    result.algorithmName = "LFU";

    std::vector<int> frames(frameCount, -1);
    std::vector<int> frequency(frameCount, 0);
    std::vector<int> lastTouch(frameCount, -1);
    int faults = 0;

    for (std::size_t time = 0; time < pages.size(); ++time) {
        const int page = pages[time];
        bool hit = false;
        for (int i = 0; i < frameCount; ++i) {
            if (frames[i] == page) {
                hit = true;
                ++frequency[i];
                lastTouch[i] = static_cast<int>(time);
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
                selected = 0;
                for (int i = 1; i < frameCount; ++i) {
                    if (frequency[i] < frequency[selected] ||
                        (frequency[i] == frequency[selected] && lastTouch[i] < lastTouch[selected])) {
                        selected = i;
                    }
                }
            }
            frames[selected] = page;
            frequency[selected] = 1;
            lastTouch[selected] = static_cast<int>(time);
        }

        result.steps.push_back({page, frames, hit});
    }

    finalizeResult(result, faults, pages.size());
    return result;
}

PageReplacementResult simulateCLOCK(int frameCount, const std::vector<int>& pages) {
    PageReplacementResult result;
    result.algorithmName = "CLOCK";

    std::vector<int> frames(frameCount, -1);
    std::vector<int> referenceBits(frameCount, 0);
    int hand = 0;
    int faults = 0;

    for (int page : pages) {
        bool hit = false;
        for (int i = 0; i < frameCount; ++i) {
            if (frames[i] == page) {
                hit = true;
                referenceBits[i] = 1;
                break;
            }
        }

        if (!hit) {
            ++faults;
            int selected = -1;
            for (int i = 0; i < frameCount; ++i) {
                if (frames[i] == -1) {
                    selected = i;
                    hand = (i + 1) % frameCount;
                    break;
                }
            }
            if (selected == -1) {
                while (true) {
                    if (referenceBits[hand] == 0) {
                        selected = hand;
                        hand = (hand + 1) % frameCount;
                        break;
                    }
                    referenceBits[hand] = 0;
                    hand = (hand + 1) % frameCount;
                }
            }
            frames[selected] = page;
            referenceBits[selected] = 1;
        }

        result.steps.push_back({page, frames, hit});
    }

    finalizeResult(result, faults, pages.size());
    return result;
}

PageReplacementResult simulateRandom(int frameCount, const std::vector<int>& pages) {
    PageReplacementResult result;
    result.algorithmName = "RANDOM";

    std::vector<int> frames(frameCount, -1);
    std::mt19937 rng(std::random_device{}());
    int faults = 0;

    for (int page : pages) {
        bool hit = std::find(frames.begin(), frames.end(), page) != frames.end();
        if (!hit) {
            ++faults;
            auto emptyIt = std::find(frames.begin(), frames.end(), -1);
            if (emptyIt != frames.end()) {
                *emptyIt = page;
            } else {
                std::uniform_int_distribution<int> replaceDist(0, frameCount - 1);
                frames[replaceDist(rng)] = page;
            }
        }
        result.steps.push_back({page, frames, hit});
    }

    finalizeResult(result, faults, pages.size());
    return result;
}

PageReplacementResult simulatePageReplacement(PageAlgorithm algorithm, int frameCount, const std::vector<int>& pages) {
    switch (algorithm) {
        case PageAlgorithm::FIFO:
            return simulateFIFO(frameCount, pages);
        case PageAlgorithm::LRU:
            return simulateLRU(frameCount, pages);
        case PageAlgorithm::LFU:
            return simulateLFU(frameCount, pages);
        case PageAlgorithm::CLOCK:
            return simulateCLOCK(frameCount, pages);
        case PageAlgorithm::Random:
            return simulateRandom(frameCount, pages);
        default:
            return simulateFIFO(frameCount, pages);
    }
}

std::string validatePageInput(int frameCount, const std::vector<int>& pages) {
    if (frameCount <= 0) {
        return "页框数量必须大于 0。";
    }
    if (pages.empty()) {
        return "页面访问序列不能为空。";
    }
    return {};
}

std::string pageAlgorithmName(PageAlgorithm algorithm) {
    switch (algorithm) {
        case PageAlgorithm::FIFO:
            return "FIFO";
        case PageAlgorithm::LRU:
            return "LRU";
        case PageAlgorithm::LFU:
            return "LFU";
        case PageAlgorithm::CLOCK:
            return "CLOCK";
        case PageAlgorithm::Random:
            return "RANDOM";
        default:
            return "FIFO";
    }
}

}  // namespace memory_management
