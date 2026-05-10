#include "filesystem/FileSystemFacade.h"
#include "filesystem/VirtualFileSystem.h"

#include <iostream>
#include <limits>
#include <sstream>
#include <string>

namespace {

const char* kDefaultImage = "virtual_disk.bin";

void printHelp() {
    std::cout << "支持命令:\n"
              << "  format [总块数] [块大小]\n"
              << "  mkdir <路径>\n"
              << "  create <路径>\n"
              << "  write <路径> <内容>\n"
              << "  read <路径>\n"
              << "  rename <路径> <新名称>\n"
              << "  delete <路径>\n"
              << "  ls [路径]\n"
              << "  stat\n"
              << "  bitmap\n"
              << "  help\n"
              << "  exit\n";
}

bool ensureLoaded(simplefs::VirtualFileSystem* fs) {
    if (fs->isLoaded()) {
        return true;
    }
    std::string errorMessage;
    return fs->load(kDefaultImage, &errorMessage);
}

void printListing(simplefs::VirtualFileSystem& fs, int dirId) {
    const auto entries = fs.listDirectory(dirId);
    std::cout << "名称\t类型\t大小\t块数\t更新时间\n";
    for (const auto& entry : entries) {
        std::cout << entry.name << '\t'
                  << simplefs::entryTypeName(entry.type) << '\t'
                  << entry.size << '\t'
                  << entry.blocks.size() << '\t'
                  << simplefs::formatTimestamp(entry.updatedAt) << '\n';
    }
}

}  // namespace

int main() {
    simplefs::VirtualFileSystem fs;
    std::string errorMessage;
    fs.load(kDefaultImage, &errorMessage);

    std::cout << "=== 文件系统模拟实验（CLI 备用入口）===\n";
    std::cout << "默认镜像文件: " << kDefaultImage << "\n";
    if (!fs.isLoaded()) {
        std::cout << "提示: 当前尚未格式化镜像，可执行 format 128 256 创建。\n";
    }
    printHelp();

    while (true) {
        std::cout << "fs> ";
        std::string line;
        if (!std::getline(std::cin, line)) {
            break;
        }
        std::istringstream iss(line);
        std::string command;
        iss >> command;

        if (command == "format") {
            simplefs::FormatConfig config;
            if (!(iss >> config.totalBlocks)) {
                config.totalBlocks = 128;
            }
            if (!(iss >> config.blockSize)) {
                config.blockSize = 256;
            }
            if (fs.format(kDefaultImage, config, &errorMessage)) {
                std::cout << "格式化成功。\n";
            } else {
                std::cout << errorMessage << '\n';
            }
        } else if (command == "mkdir") {
            if (!ensureLoaded(&fs)) {
                std::cout << "请先格式化或加载镜像。\n";
                continue;
            }
            std::string path;
            iss >> path;
            const auto parent = simplefs::parentPathOf(path);
            const auto name = simplefs::basenameOf(path);
            const int parentId = fs.findEntryIdByPath(parent);
            if (fs.createDirectory(parentId, name, &errorMessage)) {
                std::cout << "创建文件夹成功。\n";
            } else {
                std::cout << errorMessage << '\n';
            }
        } else if (command == "create") {
            if (!ensureLoaded(&fs)) {
                std::cout << "请先格式化或加载镜像。\n";
                continue;
            }
            std::string path;
            iss >> path;
            const auto parent = simplefs::parentPathOf(path);
            const auto name = simplefs::basenameOf(path);
            const int parentId = fs.findEntryIdByPath(parent);
            if (fs.createFile(parentId, name, &errorMessage)) {
                std::cout << "创建文件成功。\n";
            } else {
                std::cout << errorMessage << '\n';
            }
        } else if (command == "write") {
            if (!ensureLoaded(&fs)) {
                std::cout << "请先格式化或加载镜像。\n";
                continue;
            }
            std::string path;
            iss >> path;
            std::string content;
            std::getline(iss, content);
            if (!content.empty() && content.front() == ' ') {
                content.erase(content.begin());
            }
            const int id = fs.findEntryIdByPath(path);
            if (fs.writeFileContent(id, content, &errorMessage)) {
                std::cout << "写入成功。\n";
            } else {
                std::cout << errorMessage << '\n';
            }
        } else if (command == "read") {
            if (!ensureLoaded(&fs)) {
                std::cout << "请先格式化或加载镜像。\n";
                continue;
            }
            std::string path;
            iss >> path;
            std::cout << fs.readFileContent(fs.findEntryIdByPath(path), &errorMessage) << '\n';
            if (!errorMessage.empty()) {
                std::cout << errorMessage << '\n';
            }
        } else if (command == "rename") {
            if (!ensureLoaded(&fs)) {
                std::cout << "请先格式化或加载镜像。\n";
                continue;
            }
            std::string path;
            std::string newName;
            iss >> path >> newName;
            if (fs.renameEntry(fs.findEntryIdByPath(path), newName, &errorMessage)) {
                std::cout << "重命名成功。\n";
            } else {
                std::cout << errorMessage << '\n';
            }
        } else if (command == "delete") {
            if (!ensureLoaded(&fs)) {
                std::cout << "请先格式化或加载镜像。\n";
                continue;
            }
            std::string path;
            iss >> path;
            if (fs.deleteEntryRecursive(fs.findEntryIdByPath(path), &errorMessage)) {
                std::cout << "删除成功。\n";
            } else {
                std::cout << errorMessage << '\n';
            }
        } else if (command == "ls") {
            if (!ensureLoaded(&fs)) {
                std::cout << "请先格式化或加载镜像。\n";
                continue;
            }
            std::string path;
            iss >> path;
            if (path.empty()) {
                path = "/";
            }
            const int dirId = fs.findEntryIdByPath(path);
            if (dirId == simplefs::kInvalidId) {
                std::cout << "路径不存在。\n";
            } else {
                printListing(fs, dirId);
            }
        } else if (command == "stat") {
            if (!ensureLoaded(&fs)) {
                std::cout << "请先格式化或加载镜像。\n";
                continue;
            }
            std::cout << simplefs::buildStatsText(fs.stats());
        } else if (command == "bitmap") {
            if (!ensureLoaded(&fs)) {
                std::cout << "请先格式化或加载镜像。\n";
                continue;
            }
            std::cout << simplefs::buildBitmapText(fs.bitmapString());
        } else if (command == "help") {
            printHelp();
        } else if (command == "exit") {
            break;
        } else if (!command.empty()) {
            std::cout << "未知命令。\n";
        }
        errorMessage.clear();
    }
    return 0;
}
