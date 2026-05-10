#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace simplefs {

constexpr int kRootId = 0;
constexpr int kInvalidId = -1;
constexpr std::size_t kMaxNameLength = 63;
constexpr std::size_t kMaxBlockRefs = 128;

struct FormatConfig {
    int totalBlocks = 128;
    int blockSize = 256;
    int maxEntries = 256;
};

enum class EntryType {
    File = 1,
    Directory = 2,
};

struct FsEntry {
    int id = kInvalidId;
    int parentId = kInvalidId;
    EntryType type = EntryType::File;
    std::string name;
    int size = 0;
    std::vector<int> blocks;
    std::uint64_t createdAt = 0;
    std::uint64_t updatedAt = 0;
};

struct FsStats {
    std::string imagePath;
    int totalBlocks = 0;
    int blockSize = 0;
    int usedBlocks = 0;
    int freeBlocks = 0;
    int fileCount = 0;
    int directoryCount = 0;
    int maxEntries = 0;
    int entryCount = 0;
};

std::string entryTypeName(EntryType type);
std::string formatTimestamp(std::uint64_t timestamp);
std::uint64_t currentTimestamp();

}  // namespace simplefs
