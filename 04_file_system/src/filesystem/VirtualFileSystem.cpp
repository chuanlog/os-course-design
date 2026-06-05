#include "VirtualFileSystem.h"

#include "FileSystemFacade.h"

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <string>
#include <vector>

namespace simplefs {
namespace {

constexpr const char* kMagic = "SFSIMG1";
constexpr std::uint32_t kVersion = 1;

bool writeFixedString(std::ostream& out, const std::string& value, std::size_t size) {
    std::vector<char> buffer(size, 0);
    const std::size_t copySize = std::min(value.size(), size == 0 ? 0U : size - 1);
    std::memcpy(buffer.data(), value.data(), copySize);
    out.write(buffer.data(), static_cast<std::streamsize>(buffer.size()));
    return static_cast<bool>(out);
}

std::string readFixedString(std::istream& in, std::size_t size) {
    std::vector<char> buffer(size, 0);
    in.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
    if (!in) {
        return {};
    }
    const auto zeroPos = std::find(buffer.begin(), buffer.end(), '\0');
    return std::string(buffer.begin(), zeroPos);
}

template <typename T>
bool writePrimitive(std::ostream& out, const T& value) {
    out.write(reinterpret_cast<const char*>(&value), sizeof(T));
    return static_cast<bool>(out);
}

template <typename T>
bool readPrimitive(std::istream& in, T* value) {
    in.read(reinterpret_cast<char*>(value), sizeof(T));
    return static_cast<bool>(in);
}

FsEntry withDirectoryUsage(const FsEntry& entry, const std::vector<FsEntry>& entries) {
    if (entry.type == EntryType::File) {
        return entry;
    }

    FsEntry result = entry;
    result.size = 0;
    result.blocks.clear();
    for (const auto& child : entries) {
        if (child.parentId != entry.id) {
            continue;
        }
        const FsEntry childUsage = withDirectoryUsage(child, entries);
        result.size += childUsage.size;
        result.blocks.insert(result.blocks.end(), childUsage.blocks.begin(), childUsage.blocks.end());
    }
    return result;
}

}  // namespace

void VirtualFileSystem::reset() {
    loaded_ = false;
    imagePath_.clear();
    config_ = {};
    nextEntryId_ = 1;
    bitmap_.clear();
    entries_.clear();
    data_.clear();
}

int VirtualFileSystem::entryIndexById(int id) const {
    for (std::size_t i = 0; i < entries_.size(); ++i) {
        if (entries_[i].id == id) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

const FsEntry* VirtualFileSystem::getEntry(int id) const {
    const int index = entryIndexById(id);
    return index >= 0 ? &entries_[static_cast<std::size_t>(index)] : nullptr;
}

std::vector<FsEntry> VirtualFileSystem::allEntries() const {
    return entries_;
}

std::vector<FsEntry> VirtualFileSystem::directories() const {
    std::vector<FsEntry> result;
    for (const auto& entry : entries_) {
        if (entry.type == EntryType::Directory) {
            result.push_back(entry);
        }
    }
    std::sort(result.begin(), result.end(), [this](const FsEntry& lhs, const FsEntry& rhs) {
        return fullPath(lhs.id) < fullPath(rhs.id);
    });
    return result;
}

std::vector<FsEntry> VirtualFileSystem::listDirectory(int parentId) const {
    std::vector<FsEntry> children;
    for (const auto& entry : entries_) {
        if (entry.parentId == parentId) {
            children.push_back(withDirectoryUsage(entry, entries_));
        }
    }
    std::sort(children.begin(), children.end(), [](const FsEntry& lhs, const FsEntry& rhs) {
        if (lhs.type != rhs.type) {
            return lhs.type == EntryType::Directory;
        }
        return lhs.name < rhs.name;
    });
    return children;
}

bool VirtualFileSystem::validateEntryName(const std::string& name, std::string* errorMessage) const {
    if (name.empty()) {
        if (errorMessage != nullptr) {
            *errorMessage = "名称不能为空。";
        }
        return false;
    }
    if (name.size() > kMaxNameLength) {
        if (errorMessage != nullptr) {
            *errorMessage = "名称过长，请控制在 63 个字符以内。";
        }
        return false;
    }
    if (name.find('/') != std::string::npos || name.find('\\') != std::string::npos) {
        if (errorMessage != nullptr) {
            *errorMessage = "名称中不能包含路径分隔符。";
        }
        return false;
    }
    return true;
}

bool VirtualFileSystem::ensureParentDirectory(int parentId, std::string* errorMessage) const {
    const FsEntry* entry = getEntry(parentId);
    if (entry == nullptr || entry->type != EntryType::Directory) {
        if (errorMessage != nullptr) {
            *errorMessage = "目标目录不存在。";
        }
        return false;
    }
    return true;
}

bool VirtualFileSystem::hasSiblingWithName(int parentId, const std::string& name, int excludedId) const {
    for (const auto& entry : entries_) {
        if (entry.parentId == parentId && entry.id != excludedId && entry.name == name) {
            return true;
        }
    }
    return false;
}

int VirtualFileSystem::usedBlockCount() const {
    return static_cast<int>(std::count(bitmap_.begin(), bitmap_.end(), static_cast<std::uint8_t>(1)));
}

void VirtualFileSystem::freeBlocks(const std::vector<int>& blocks) {
    for (int block : blocks) {
        if (block >= 0 && block < static_cast<int>(bitmap_.size())) {
            bitmap_[static_cast<std::size_t>(block)] = 0;
            const std::size_t offset = static_cast<std::size_t>(block) * static_cast<std::size_t>(config_.blockSize);
            std::fill(data_.begin() + static_cast<std::ptrdiff_t>(offset),
                      data_.begin() + static_cast<std::ptrdiff_t>(offset + config_.blockSize),
                      0);
        }
    }
}

void VirtualFileSystem::zeroBlocks(const std::vector<int>& blocks) {
    for (int block : blocks) {
        if (block >= 0 && block < static_cast<int>(bitmap_.size())) {
            const std::size_t offset = static_cast<std::size_t>(block) * static_cast<std::size_t>(config_.blockSize);
            std::fill(data_.begin() + static_cast<std::ptrdiff_t>(offset),
                      data_.begin() + static_cast<std::ptrdiff_t>(offset + config_.blockSize),
                      0);
        }
    }
}

std::vector<int> VirtualFileSystem::allocateBlocks(int count) {
    std::vector<int> blocks;
    for (int i = 0; i < static_cast<int>(bitmap_.size()) && static_cast<int>(blocks.size()) < count; ++i) {
        if (bitmap_[static_cast<std::size_t>(i)] == 0) {
            bitmap_[static_cast<std::size_t>(i)] = 1;
            blocks.push_back(i);
        }
    }
    if (static_cast<int>(blocks.size()) != count) {
        for (int block : blocks) {
            bitmap_[static_cast<std::size_t>(block)] = 0;
        }
        blocks.clear();
    }
    return blocks;
}

std::vector<int> VirtualFileSystem::collectSubtreeIds(int entryId) const {
    std::vector<int> result{entryId};
    for (const auto& entry : entries_) {
        if (entry.parentId == entryId) {
            const auto children = collectSubtreeIds(entry.id);
            result.insert(result.end(), children.begin(), children.end());
        }
    }
    return result;
}

bool VirtualFileSystem::format(const std::string& imagePath, const FormatConfig& config, std::string* errorMessage) {
    if (config.totalBlocks <= 0 || config.blockSize <= 0 || config.maxEntries < 2) {
        if (errorMessage != nullptr) {
            *errorMessage = "格式化参数非法。";
        }
        return false;
    }

    reset();
    loaded_ = true;
    imagePath_ = imagePath;
    config_ = config;
    nextEntryId_ = 1;
    bitmap_.assign(static_cast<std::size_t>(config_.totalBlocks), 0);
    data_.assign(static_cast<std::size_t>(config_.totalBlocks) * static_cast<std::size_t>(config_.blockSize), 0);
    entries_.push_back({kRootId, kInvalidId, EntryType::Directory, "root", 0, {}, currentTimestamp(), currentTimestamp()});
    return save(errorMessage);
}

bool VirtualFileSystem::load(const std::string& imagePath, std::string* errorMessage) {
    reset();

    std::ifstream in(imagePath, std::ios::binary);
    if (!in) {
        if (errorMessage != nullptr) {
            *errorMessage = "无法打开镜像文件，请先格式化磁盘镜像。";
        }
        return false;
    }

    std::string magic = readFixedString(in, 8);
    std::uint32_t version = 0;
    if (magic != kMagic || !readPrimitive(in, &version) || version != kVersion) {
        if (errorMessage != nullptr) {
            *errorMessage = "镜像文件格式不正确。";
        }
        return false;
    }

    if (!readPrimitive(in, &config_.totalBlocks) ||
        !readPrimitive(in, &config_.blockSize) ||
        !readPrimitive(in, &config_.maxEntries) ||
        !readPrimitive(in, &nextEntryId_)) {
        if (errorMessage != nullptr) {
            *errorMessage = "读取镜像头失败。";
        }
        return false;
    }

    bitmap_.assign(static_cast<std::size_t>(config_.totalBlocks), 0);
    for (int i = 0; i < config_.totalBlocks; ++i) {
        std::uint8_t used = 0;
        if (!readPrimitive(in, &used)) {
            if (errorMessage != nullptr) {
                *errorMessage = "读取位示图失败。";
            }
            return false;
        }
        bitmap_[static_cast<std::size_t>(i)] = used;
    }

    entries_.clear();
    for (int slot = 0; slot < config_.maxEntries; ++slot) {
        std::uint8_t used = 0;
        if (!readPrimitive(in, &used)) {
            if (errorMessage != nullptr) {
                *errorMessage = "读取目录项失败。";
            }
            return false;
        }

        int id = kInvalidId;
        int parentId = kInvalidId;
        int typeRaw = 0;
        int size = 0;
        int blockCount = 0;
        std::string name = readFixedString(in, kMaxNameLength + 1);
        std::uint64_t createdAt = 0;
        std::uint64_t updatedAt = 0;
        if (!readPrimitive(in, &id) || !readPrimitive(in, &parentId) || !readPrimitive(in, &typeRaw) ||
            !readPrimitive(in, &size) || !readPrimitive(in, &blockCount) ||
            !readPrimitive(in, &createdAt) || !readPrimitive(in, &updatedAt)) {
            if (errorMessage != nullptr) {
                *errorMessage = "读取目录项字段失败。";
            }
            return false;
        }
        std::vector<int> blocks(kMaxBlockRefs, -1);
        for (std::size_t i = 0; i < kMaxBlockRefs; ++i) {
            if (!readPrimitive(in, &blocks[i])) {
                if (errorMessage != nullptr) {
                    *errorMessage = "读取块引用失败。";
                }
                return false;
            }
        }

        if (used != 0) {
            FsEntry entry;
            entry.id = id;
            entry.parentId = parentId;
            entry.type = (typeRaw == static_cast<int>(EntryType::Directory)) ? EntryType::Directory : EntryType::File;
            entry.name = name;
            entry.size = size;
            entry.createdAt = createdAt;
            entry.updatedAt = updatedAt;
            entry.blocks.assign(blocks.begin(), blocks.begin() + std::min<int>(blockCount, static_cast<int>(kMaxBlockRefs)));
            entries_.push_back(entry);
        }
    }

    data_.assign(static_cast<std::size_t>(config_.totalBlocks) * static_cast<std::size_t>(config_.blockSize), 0);
    in.read(data_.data(), static_cast<std::streamsize>(data_.size()));
    if (!in) {
        if (errorMessage != nullptr) {
            *errorMessage = "读取数据区失败。";
        }
        return false;
    }

    loaded_ = true;
    imagePath_ = imagePath;
    return true;
}

bool VirtualFileSystem::save(std::string* errorMessage) const {
    std::ofstream out(imagePath_, std::ios::binary | std::ios::trunc);
    if (!out) {
        if (errorMessage != nullptr) {
            *errorMessage = "无法写入镜像文件。";
        }
        return false;
    }

    if (!writeFixedString(out, kMagic, 8) ||
        !writePrimitive(out, kVersion) ||
        !writePrimitive(out, config_.totalBlocks) ||
        !writePrimitive(out, config_.blockSize) ||
        !writePrimitive(out, config_.maxEntries) ||
        !writePrimitive(out, nextEntryId_)) {
        if (errorMessage != nullptr) {
            *errorMessage = "写入镜像头失败。";
        }
        return false;
    }

    for (std::uint8_t used : bitmap_) {
        if (!writePrimitive(out, used)) {
            if (errorMessage != nullptr) {
                *errorMessage = "写入位示图失败。";
            }
            return false;
        }
    }

    std::vector<FsEntry> sortedEntries = entries_;
    std::sort(sortedEntries.begin(), sortedEntries.end(), [](const FsEntry& lhs, const FsEntry& rhs) {
        return lhs.id < rhs.id;
    });
    std::size_t writtenEntries = 0;
    for (const auto& entry : sortedEntries) {
        if (writtenEntries >= static_cast<std::size_t>(config_.maxEntries)) {
            if (errorMessage != nullptr) {
                *errorMessage = "目录项数量超过镜像容量。";
            }
            return false;
        }
        const std::uint8_t used = 1;
        const int typeRaw = static_cast<int>(entry.type);
        const int blockCount = static_cast<int>(entry.blocks.size());
        if (!writePrimitive(out, used) ||
            !writeFixedString(out, entry.name, kMaxNameLength + 1) ||
            !writePrimitive(out, entry.id) ||
            !writePrimitive(out, entry.parentId) ||
            !writePrimitive(out, typeRaw) ||
            !writePrimitive(out, entry.size) ||
            !writePrimitive(out, blockCount) ||
            !writePrimitive(out, entry.createdAt) ||
            !writePrimitive(out, entry.updatedAt)) {
            if (errorMessage != nullptr) {
                *errorMessage = "写入目录项失败。";
            }
            return false;
        }
        for (std::size_t i = 0; i < kMaxBlockRefs; ++i) {
            const int block = (i < entry.blocks.size()) ? entry.blocks[i] : -1;
            if (!writePrimitive(out, block)) {
                if (errorMessage != nullptr) {
                    *errorMessage = "写入块引用失败。";
                }
                return false;
            }
        }
        ++writtenEntries;
    }

    while (writtenEntries < static_cast<std::size_t>(config_.maxEntries)) {
        const std::uint8_t used = 0;
        if (!writePrimitive(out, used) ||
            !writeFixedString(out, std::string(), kMaxNameLength + 1) ||
            !writePrimitive(out, kInvalidId) ||
            !writePrimitive(out, kInvalidId) ||
            !writePrimitive(out, 0) ||
            !writePrimitive(out, 0) ||
            !writePrimitive(out, 0) ||
            !writePrimitive(out, static_cast<std::uint64_t>(0)) ||
            !writePrimitive(out, static_cast<std::uint64_t>(0))) {
            if (errorMessage != nullptr) {
                *errorMessage = "写入空目录项失败。";
            }
            return false;
        }
        for (std::size_t i = 0; i < kMaxBlockRefs; ++i) {
            const int block = -1;
            if (!writePrimitive(out, block)) {
                if (errorMessage != nullptr) {
                    *errorMessage = "写入空块引用失败。";
                }
                return false;
            }
        }
        ++writtenEntries;
    }

    out.write(data_.data(), static_cast<std::streamsize>(data_.size()));
    if (!out) {
        if (errorMessage != nullptr) {
            *errorMessage = "写入数据区失败。";
        }
        return false;
    }
    return true;
}

int VirtualFileSystem::findEntryIdByPath(const std::string& path) const {
    const auto parts = splitPath(path);
    int currentId = kRootId;
    for (const auto& part : parts) {
        int nextId = kInvalidId;
        for (const auto& entry : entries_) {
            if (entry.parentId == currentId && entry.name == part) {
                nextId = entry.id;
                break;
            }
        }
        if (nextId == kInvalidId) {
            return kInvalidId;
        }
        currentId = nextId;
    }
    return currentId;
}

std::string VirtualFileSystem::fullPath(int entryId) const {
    const FsEntry* entry = getEntry(entryId);
    if (entry == nullptr) {
        return "/";
    }
    if (entry->id == kRootId) {
        return "/";
    }
    std::vector<std::string> parts;
    const FsEntry* current = entry;
    while (current != nullptr && current->id != kRootId) {
        parts.push_back(current->name);
        current = getEntry(current->parentId);
    }
    std::string path;
    for (auto it = parts.rbegin(); it != parts.rend(); ++it) {
        path += '/' + *it;
    }
    return path.empty() ? "/" : path;
}

bool VirtualFileSystem::createFile(int parentId, const std::string& name, std::string* errorMessage) {
    if (!loaded_) {
        if (errorMessage != nullptr) {
            *errorMessage = "请先加载或格式化镜像。";
        }
        return false;
    }
    if (!ensureParentDirectory(parentId, errorMessage) || !validateEntryName(name, errorMessage)) {
        return false;
    }
    if (hasSiblingWithName(parentId, name, kInvalidId)) {
        if (errorMessage != nullptr) {
            *errorMessage = "同名文件或文件夹已存在。";
        }
        return false;
    }
    if (static_cast<int>(entries_.size()) >= config_.maxEntries) {
        if (errorMessage != nullptr) {
            *errorMessage = "目录项已满，无法创建更多对象。";
        }
        return false;
    }
    const auto timestamp = currentTimestamp();
    entries_.push_back({nextEntryId_++, parentId, EntryType::File, name, 0, {}, timestamp, timestamp});
    return save(errorMessage);
}

bool VirtualFileSystem::createDirectory(int parentId, const std::string& name, std::string* errorMessage) {
    if (!loaded_) {
        if (errorMessage != nullptr) {
            *errorMessage = "请先加载或格式化镜像。";
        }
        return false;
    }
    if (!ensureParentDirectory(parentId, errorMessage) || !validateEntryName(name, errorMessage)) {
        return false;
    }
    if (hasSiblingWithName(parentId, name, kInvalidId)) {
        if (errorMessage != nullptr) {
            *errorMessage = "同名文件或文件夹已存在。";
        }
        return false;
    }
    if (static_cast<int>(entries_.size()) >= config_.maxEntries) {
        if (errorMessage != nullptr) {
            *errorMessage = "目录项已满，无法创建更多对象。";
        }
        return false;
    }
    const auto timestamp = currentTimestamp();
    entries_.push_back({nextEntryId_++, parentId, EntryType::Directory, name, 0, {}, timestamp, timestamp});
    return save(errorMessage);
}

bool VirtualFileSystem::renameEntry(int entryId, const std::string& newName, std::string* errorMessage) {
    if (entryId == kRootId) {
        if (errorMessage != nullptr) {
            *errorMessage = "根目录不允许重命名。";
        }
        return false;
    }
    if (!validateEntryName(newName, errorMessage)) {
        return false;
    }
    const int index = entryIndexById(entryId);
    if (index < 0) {
        if (errorMessage != nullptr) {
            *errorMessage = "目标不存在。";
        }
        return false;
    }
    if (hasSiblingWithName(entries_[static_cast<std::size_t>(index)].parentId, newName, entryId)) {
        if (errorMessage != nullptr) {
            *errorMessage = "同级目录中已存在同名对象。";
        }
        return false;
    }
    entries_[static_cast<std::size_t>(index)].name = newName;
    entries_[static_cast<std::size_t>(index)].updatedAt = currentTimestamp();
    return save(errorMessage);
}

bool VirtualFileSystem::deleteEntryRecursive(int entryId, std::string* errorMessage) {
    if (entryId == kRootId) {
        if (errorMessage != nullptr) {
            *errorMessage = "根目录不能删除。";
        }
        return false;
    }
    if (entryIndexById(entryId) < 0) {
        if (errorMessage != nullptr) {
            *errorMessage = "目标不存在。";
        }
        return false;
    }
    const auto toDelete = collectSubtreeIds(entryId);
    for (int id : toDelete) {
        const int index = entryIndexById(id);
        if (index >= 0) {
            freeBlocks(entries_[static_cast<std::size_t>(index)].blocks);
        }
    }
    entries_.erase(std::remove_if(entries_.begin(), entries_.end(), [&](const FsEntry& entry) {
        return std::find(toDelete.begin(), toDelete.end(), entry.id) != toDelete.end();
    }), entries_.end());
    return save(errorMessage);
}

bool VirtualFileSystem::writeFileContent(int entryId, const std::string& content, std::string* errorMessage) {
    const int index = entryIndexById(entryId);
    if (index < 0) {
        if (errorMessage != nullptr) {
            *errorMessage = "文件不存在。";
        }
        return false;
    }
    FsEntry& entry = entries_[static_cast<std::size_t>(index)];
    if (entry.type != EntryType::File) {
        if (errorMessage != nullptr) {
            *errorMessage = "只能向文件写入内容。";
        }
        return false;
    }

    const int requiredBlocks = content.empty() ? 0 : (static_cast<int>(content.size()) + config_.blockSize - 1) / config_.blockSize;
    if (requiredBlocks > static_cast<int>(kMaxBlockRefs)) {
        if (errorMessage != nullptr) {
            *errorMessage = "文件过大，超出当前模拟文件系统的单文件块引用上限。";
        }
        return false;
    }
    const int availableBlocks = config_.totalBlocks - usedBlockCount() + static_cast<int>(entry.blocks.size());
    if (requiredBlocks > availableBlocks) {
        if (errorMessage != nullptr) {
            *errorMessage = "磁盘空间不足。";
        }
        return false;
    }

    freeBlocks(entry.blocks);
    entry.blocks.clear();
    if (requiredBlocks > 0) {
        entry.blocks = allocateBlocks(requiredBlocks);
        if (static_cast<int>(entry.blocks.size()) != requiredBlocks) {
            if (errorMessage != nullptr) {
                *errorMessage = "磁盘空间不足。";
            }
            return false;
        }
    }
    zeroBlocks(entry.blocks);

    int copied = 0;
    for (int block : entry.blocks) {
        const std::size_t offset = static_cast<std::size_t>(block) * static_cast<std::size_t>(config_.blockSize);
        const int chunk = std::min(config_.blockSize, static_cast<int>(content.size()) - copied);
        std::memcpy(data_.data() + static_cast<std::ptrdiff_t>(offset), content.data() + copied, static_cast<std::size_t>(chunk));
        copied += chunk;
    }

    entry.size = static_cast<int>(content.size());
    entry.updatedAt = currentTimestamp();
    return save(errorMessage);
}

std::string VirtualFileSystem::readFileContent(int entryId, std::string* errorMessage) const {
    const FsEntry* entry = getEntry(entryId);
    if (entry == nullptr || entry->type != EntryType::File) {
        if (errorMessage != nullptr) {
            *errorMessage = "目标文件不存在。";
        }
        return {};
    }
    std::string content;
    content.reserve(static_cast<std::size_t>(entry->size));
    int remaining = entry->size;
    for (int block : entry->blocks) {
        const std::size_t offset = static_cast<std::size_t>(block) * static_cast<std::size_t>(config_.blockSize);
        const int chunk = std::min(config_.blockSize, remaining);
        content.append(data_.data() + static_cast<std::ptrdiff_t>(offset),
                       data_.data() + static_cast<std::ptrdiff_t>(offset + chunk));
        remaining -= chunk;
        if (remaining <= 0) {
            break;
        }
    }
    return content;
}

FsStats VirtualFileSystem::stats() const {
    FsStats result;
    result.imagePath = imagePath_;
    result.totalBlocks = config_.totalBlocks;
    result.blockSize = config_.blockSize;
    result.usedBlocks = usedBlockCount();
    result.freeBlocks = config_.totalBlocks - result.usedBlocks;
    result.maxEntries = config_.maxEntries;
    result.entryCount = static_cast<int>(entries_.size());
    for (const auto& entry : entries_) {
        if (entry.type == EntryType::Directory) {
            ++result.directoryCount;
        } else {
            ++result.fileCount;
        }
    }
    return result;
}

std::string VirtualFileSystem::bitmapString() const {
    std::string bitmap;
    bitmap.reserve(bitmap_.size());
    for (std::uint8_t value : bitmap_) {
        bitmap.push_back(value == 0 ? '0' : '1');
    }
    return bitmap;
}

}  // namespace simplefs
