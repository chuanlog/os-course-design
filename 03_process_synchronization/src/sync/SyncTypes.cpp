#include "SyncTypes.h"

#include <sstream>

namespace sync_demo {

std::string scenarioName(Scenario scenario) {
    switch (scenario) {
        case Scenario::ProducerConsumer:
            return "生产者 - 消费者";
        case Scenario::ReadersWriters:
            return "读者 - 写者";
        case Scenario::DiningPhilosophers:
            return "哲学家进餐";
        default:
            return "未知场景";
    }
}

std::string buildSummaryText(const SimulationResult& result) {
    std::ostringstream oss;
    oss << "场景: " << result.scenarioName << '\n';
    oss << "事件总数: " << result.events.size() << "\n\n";
    oss << "结果汇总:\n";
    for (const auto& line : result.summaryLines) {
        oss << "- " << line << '\n';
    }
    return oss.str();
}

}  // namespace sync_demo
