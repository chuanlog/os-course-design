#include "FsTypes.h"

#include <ctime>
#include <iomanip>
#include <sstream>

namespace simplefs {

std::string entryTypeName(EntryType type) {
    return type == EntryType::Directory ? "文件夹" : "文件";
}

std::string formatTimestamp(std::uint64_t timestamp) {
    if (timestamp == 0) {
        return "-";
    }
    const std::time_t rawTime = static_cast<std::time_t>(timestamp);
    std::tm localTime{};
#if defined(_WIN32)
    localtime_s(&localTime, &rawTime);
#else
    localtime_r(&rawTime, &localTime);
#endif
    std::ostringstream oss;
    oss << std::put_time(&localTime, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

std::uint64_t currentTimestamp() {
    return static_cast<std::uint64_t>(std::time(nullptr));
}

}  // namespace simplefs
