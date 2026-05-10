#include <algorithm>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <map>
#include <numeric>
#include <sstream>
#include <string>
#include <vector>

struct FileEntry {
    std::string name;
    std::string content;
    std::vector<int> blocks;
    std::string createdAt;
    std::string updatedAt;
};

std::string nowString() {
    std::time_t now = std::time(nullptr);
    std::tm localTime{};
#if defined(_WIN32)
    localtime_s(&localTime, &now);
#else
    localtime_r(&now, &localTime);
#endif
    std::ostringstream oss;
    oss << std::put_time(&localTime, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

class MiniFileSystem {
public:
    MiniFileSystem(int totalBlocks, int blockSize)
        : totalBlocks_(totalBlocks), blockSize_(blockSize), bitmap_(totalBlocks, false) {}

    bool create(const std::string& name) {
        if (name.empty() || files_.count(name) != 0U) {
            return false;
        }
        const std::string timestamp = nowString();
        files_[name] = FileEntry{name, "", {}, timestamp, timestamp};
        return true;
    }

    bool writeFile(const std::string& name, const std::string& content) {
        auto it = files_.find(name);
        if (it == files_.end()) {
            return false;
        }

        int requiredBlocks = blocksNeeded(static_cast<int>(content.size()));
        int reusable = static_cast<int>(it->second.blocks.size());
        if (requiredBlocks > freeBlockCount() + reusable) {
            return false;
        }

        releaseBlocks(it->second.blocks);
        std::vector<int> newBlocks = allocateBlocks(requiredBlocks);
        if (static_cast<int>(newBlocks.size()) != requiredBlocks) {
            markBlocks(newBlocks, false);
            return false;
        }

        it->second.blocks = std::move(newBlocks);
        it->second.content = content;
        it->second.updatedAt = nowString();
        return true;
    }

    bool readFile(const std::string& name) const {
        auto it = files_.find(name);
        if (it == files_.end()) {
            return false;
        }
        std::cout << "内容: " << it->second.content << '\n';
        return true;
    }

    bool deleteFile(const std::string& name) {
        auto it = files_.find(name);
        if (it == files_.end()) {
            return false;
        }
        releaseBlocks(it->second.blocks);
        files_.erase(it);
        return true;
    }

    void listFiles() const {
        std::cout << "\n当前目录文件列表:\n";
        if (files_.empty()) {
            std::cout << "(空)\n";
            return;
        }

        std::cout << std::left << std::setw(16) << "文件名"
                  << std::setw(12) << "大小"
                  << std::setw(20) << "占用块"
                  << std::setw(20) << "创建时间"
                  << std::setw(20) << "更新时间" << '\n';
        for (const auto& [name, entry] : files_) {
            std::ostringstream blockList;
            blockList << '[';
            for (std::size_t i = 0; i < entry.blocks.size(); ++i) {
                if (i != 0) {
                    blockList << ' ';
                }
                blockList << entry.blocks[i];
            }
            blockList << ']';

            std::cout << std::left << std::setw(16) << name
                      << std::setw(12) << entry.content.size()
                      << std::setw(20) << blockList.str()
                      << std::setw(20) << entry.createdAt
                      << std::setw(20) << entry.updatedAt << '\n';
        }
    }

    void showStat() const {
        std::cout << "\n文件系统统计信息:\n";
        std::cout << "总块数: " << totalBlocks_ << '\n';
        std::cout << "块大小: " << blockSize_ << " 字节\n";
        std::cout << "已用块数: " << usedBlockCount() << '\n';
        std::cout << "空闲块数: " << freeBlockCount() << '\n';
        std::cout << "文件数量: " << files_.size() << '\n';
    }

    void showBitmap() const {
        std::cout << "\n位示图（1 表示已占用，0 表示空闲）:\n";
        for (int i = 0; i < totalBlocks_; ++i) {
            std::cout << (bitmap_[i] ? '1' : '0') << ' ';
            if ((i + 1) % 16 == 0) {
                std::cout << '\n';
            }
        }
        if (totalBlocks_ % 16 != 0) {
            std::cout << '\n';
        }
    }

private:
    int totalBlocks_ = 0;
    int blockSize_ = 0;
    std::vector<bool> bitmap_;
    std::map<std::string, FileEntry> files_;

    int blocksNeeded(int bytes) const {
        if (bytes == 0) {
            return 0;
        }
        return (bytes + blockSize_ - 1) / blockSize_;
    }

    int usedBlockCount() const {
        return static_cast<int>(std::count(bitmap_.begin(), bitmap_.end(), true));
    }

    int freeBlockCount() const {
        return totalBlocks_ - usedBlockCount();
    }

    void markBlocks(const std::vector<int>& blocks, bool used) {
        for (int block : blocks) {
            bitmap_[block] = used;
        }
    }

    void releaseBlocks(const std::vector<int>& blocks) {
        markBlocks(blocks, false);
    }

    std::vector<int> allocateBlocks(int count) {
        std::vector<int> result;
        for (int i = 0; i < totalBlocks_ && static_cast<int>(result.size()) < count; ++i) {
            if (!bitmap_[i]) {
                bitmap_[i] = true;
                result.push_back(i);
            }
        }
        return result;
    }
};

std::string trimLeadingSpace(std::string value) {
    auto it = std::find_if(value.begin(), value.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    });
    value.erase(value.begin(), it);
    return value;
}

void printHelp() {
    std::cout << "支持命令:\n";
    std::cout << "  create <文件名>\n";
    std::cout << "  write <文件名> <内容>\n";
    std::cout << "  read <文件名>\n";
    std::cout << "  delete <文件名>\n";
    std::cout << "  ls\n";
    std::cout << "  stat\n";
    std::cout << "  bitmap\n";
    std::cout << "  help\n";
    std::cout << "  exit\n";
}

int main() {
    std::cout << "=== 文件系统模拟实验 ===\n";
    std::cout << "请输入磁盘块数量: ";
    int totalBlocks = 0;
    std::cin >> totalBlocks;
    std::cout << "请输入每个磁盘块大小(字节): ";
    int blockSize = 0;
    std::cin >> blockSize;

    if (totalBlocks <= 0 || blockSize <= 0) {
        std::cerr << "初始化参数非法。\n";
        return 1;
    }

    MiniFileSystem fileSystem(totalBlocks, blockSize);
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
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

        if (command == "create") {
            std::string name;
            iss >> name;
            if (fileSystem.create(name)) {
                std::cout << "创建成功。\n";
            } else {
                std::cout << "创建失败。\n";
            }
        } else if (command == "write") {
            std::string name;
            iss >> name;
            std::string content;
            std::getline(iss, content);
            content = trimLeadingSpace(content);
            if (fileSystem.writeFile(name, content)) {
                std::cout << "写入成功。\n";
            } else {
                std::cout << "写入失败，可能是文件不存在或空间不足。\n";
            }
        } else if (command == "read") {
            std::string name;
            iss >> name;
            if (!fileSystem.readFile(name)) {
                std::cout << "读取失败。\n";
            }
        } else if (command == "delete") {
            std::string name;
            iss >> name;
            if (fileSystem.deleteFile(name)) {
                std::cout << "删除成功。\n";
            } else {
                std::cout << "删除失败。\n";
            }
        } else if (command == "ls") {
            fileSystem.listFiles();
        } else if (command == "stat") {
            fileSystem.showStat();
        } else if (command == "bitmap") {
            fileSystem.showBitmap();
        } else if (command == "help") {
            printHelp();
        } else if (command == "exit") {
            break;
        } else if (!command.empty()) {
            std::cout << "未知命令。\n";
        }
    }

    return 0;
}
