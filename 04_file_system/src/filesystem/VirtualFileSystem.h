#pragma once

#include "FsTypes.h"

#include <string>
#include <vector>

namespace simplefs {

class VirtualFileSystem {
public:
    bool format(const std::string& imagePath, const FormatConfig& config, std::string* errorMessage);
    bool load(const std::string& imagePath, std::string* errorMessage);
    bool isLoaded() const { return loaded_; }

    const std::string& imagePath() const { return imagePath_; }
    int rootId() const { return kRootId; }

    const FsEntry* getEntry(int id) const;
    std::vector<FsEntry> allEntries() const;
    std::vector<FsEntry> directories() const;
    std::vector<FsEntry> listDirectory(int parentId) const;
    int findEntryIdByPath(const std::string& path) const;
    std::string fullPath(int entryId) const;

    bool createFile(int parentId, const std::string& name, std::string* errorMessage);
    bool createDirectory(int parentId, const std::string& name, std::string* errorMessage);
    bool renameEntry(int entryId, const std::string& newName, std::string* errorMessage);
    bool deleteEntryRecursive(int entryId, std::string* errorMessage);
    bool writeFileContent(int entryId, const std::string& content, std::string* errorMessage);
    std::string readFileContent(int entryId, std::string* errorMessage) const;

    FsStats stats() const;
    std::string bitmapString() const;

private:
    bool save(std::string* errorMessage) const;
    void reset();
    int entryIndexById(int id) const;
    bool validateEntryName(const std::string& name, std::string* errorMessage) const;
    bool ensureParentDirectory(int parentId, std::string* errorMessage) const;
    bool hasSiblingWithName(int parentId, const std::string& name, int excludedId) const;
    int usedBlockCount() const;
    void freeBlocks(const std::vector<int>& blocks);
    std::vector<int> allocateBlocks(int count);
    std::vector<int> collectSubtreeIds(int entryId) const;
    void zeroBlocks(const std::vector<int>& blocks);

    bool loaded_ = false;
    std::string imagePath_;
    FormatConfig config_;
    int nextEntryId_ = 1;
    std::vector<std::uint8_t> bitmap_;
    std::vector<FsEntry> entries_;
    std::vector<char> data_;
};

}  // namespace simplefs
