#include "FileSystemFacade.h"

#include <sstream>

namespace simplefs {

std::string normalizePath(const std::string& path) {
    if (path.empty()) {
        return "/";
    }
    std::string normalized;
    normalized.reserve(path.size() + 1);
    bool previousSlash = false;
    for (char ch : path) {
        const char current = (ch == '\\') ? '/' : ch;
        if (current == '/') {
            if (!previousSlash) {
                normalized.push_back('/');
                previousSlash = true;
            }
        } else {
            normalized.push_back(current);
            previousSlash = false;
        }
    }
    if (normalized.empty()) {
        normalized = "/";
    }
    if (normalized.front() != '/') {
        normalized.insert(normalized.begin(), '/');
    }
    if (normalized.size() > 1 && normalized.back() == '/') {
        normalized.pop_back();
    }
    return normalized;
}

std::vector<std::string> splitPath(const std::string& path) {
    std::vector<std::string> parts;
    std::string current;
    for (char ch : normalizePath(path)) {
        if (ch == '/') {
            if (!current.empty()) {
                parts.push_back(current);
                current.clear();
            }
        } else {
            current.push_back(ch);
        }
    }
    if (!current.empty()) {
        parts.push_back(current);
    }
    return parts;
}

std::string basenameOf(const std::string& path) {
    const auto parts = splitPath(path);
    if (parts.empty()) {
        return {};
    }
    return parts.back();
}

std::string parentPathOf(const std::string& path) {
    const auto normalized = normalizePath(path);
    const auto parts = splitPath(normalized);
    if (parts.empty() || parts.size() == 1) {
        return "/";
    }
    std::ostringstream oss;
    for (std::size_t i = 0; i + 1 < parts.size(); ++i) {
        oss << '/' << parts[i];
    }
    return oss.str().empty() ? "/" : oss.str();
}

std::string buildStatsText(const FsStats& stats) {
    std::ostringstream oss;
    oss << "镜像文件: " << stats.imagePath << '\n';
    oss << "总块数: " << stats.totalBlocks << '\n';
    oss << "块大小: " << stats.blockSize << " 字节\n";
    oss << "已用块数: " << stats.usedBlocks << '\n';
    oss << "空闲块数: " << stats.freeBlocks << '\n';
    oss << "文件数量: " << stats.fileCount << '\n';
    oss << "文件夹数量: " << stats.directoryCount << '\n';
    oss << "目录项容量: " << stats.maxEntries << '\n';
    oss << "已使用目录项: " << stats.entryCount << '\n';
    return oss.str();
}

std::string buildBitmapText(const std::string& bitmap) {
    std::ostringstream oss;
    oss << "位示图（1 表示占用，0 表示空闲）\n";
    for (std::size_t i = 0; i < bitmap.size(); ++i) {
        oss << bitmap[i] << ' ';
        if ((i + 1) % 16 == 0) {
            oss << '\n';
        }
    }
    if (!bitmap.empty() && bitmap.size() % 16 != 0) {
        oss << '\n';
    }
    return oss.str();
}

}  // namespace simplefs
