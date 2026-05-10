#pragma once

#include "SyncTypes.h"

#include <chrono>
#include <mutex>
#include <string>
#include <vector>

namespace sync_demo {

class EventRecorder {
public:
    EventRecorder() : start_(std::chrono::steady_clock::now()) {}

    void log(const std::string& actor, const std::string& action, const std::string& detail) {
        const auto now = std::chrono::steady_clock::now();
        const int elapsed = static_cast<int>(std::chrono::duration_cast<std::chrono::milliseconds>(now - start_).count());

        std::lock_guard<std::mutex> lock(mutex_);
        events_.push_back({nextOrder_++, elapsed, actor, action, detail});
    }

    std::vector<EventRecord> events() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return events_;
    }

private:
    std::chrono::steady_clock::time_point start_;
    mutable std::mutex mutex_;
    std::vector<EventRecord> events_;
    int nextOrder_ = 1;
};

}  // namespace sync_demo
