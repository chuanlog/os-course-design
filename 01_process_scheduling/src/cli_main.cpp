#include "scheduling/MLFQScheduler.h"
#include "scheduling/MLQScheduler.h"
#include "scheduling/SchedulerFacade.h"
#include "scheduling/SchedulingUtils.h"

#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <vector>

using scheduling::Algorithm;
using scheduling::MLFQConfig;
using scheduling::MLQConfig;
using scheduling::ProcessInput;
using scheduling::QueueDiscipline;
using scheduling::SimulationResult;

namespace {

const char* disciplineLabel(QueueDiscipline discipline) {
    switch (discipline) {
        case QueueDiscipline::FCFS:
            return "FCFS";
        case QueueDiscipline::SJF:
            return "SJF";
        case QueueDiscipline::RR:
            return "RR";
        case QueueDiscipline::Priority:
            return "Priority";
    }
    return "FCFS";
}

QueueDiscipline disciplineFromChoice(int choice) {
    switch (choice) {
        case 1:
            return QueueDiscipline::FCFS;
        case 2:
            return QueueDiscipline::SJF;
        case 3:
            return QueueDiscipline::RR;
        case 4:
            return QueueDiscipline::Priority;
        default:
            throw std::invalid_argument("队列算法选项非法。");
    }
}

void printTimeline(const SimulationResult& result) {
    std::cout << "\n调度执行顺序（Gantt 图）：\n";
    for (const auto& segment : result.timeline) {
        std::cout << '[' << segment.start << ", " << segment.end << ") " << segment.name << '\n';
    }
}

void printResult(const SimulationResult& result) {
    std::cout << "\n=== " << result.algorithmName << " 调度结果 ===\n";
    printTimeline(result);

    std::cout << "\n" << std::left << std::setw(10) << "进程"
              << std::setw(10) << "到达"
              << std::setw(10) << "运行"
              << std::setw(10) << "优先级"
              << std::setw(10) << "完成"
              << std::setw(10) << "周转"
              << std::setw(10) << "等待"
              << std::setw(10) << "响应" << '\n';

    for (const auto& process : result.processes) {
        std::cout << std::left << std::setw(10) << process.name
                  << std::setw(10) << process.arrival
                  << std::setw(10) << process.burst
                  << std::setw(10) << process.priority
                  << std::setw(10) << process.completion
                  << std::setw(10) << process.turnaround
                  << std::setw(10) << process.waiting
                  << std::setw(10) << process.response << '\n';
    }

    std::cout << std::fixed << std::setprecision(2)
              << "\n平均周转时间: " << result.avgTurnaround << '\n'
              << "平均等待时间: " << result.avgWaiting << '\n'
              << "平均响应时间: " << result.avgResponse << '\n';
}

Algorithm readAlgorithmChoice() {
    std::cout << "\n请选择算法:\n";
    std::cout << "1. FCFS\n";
    std::cout << "2. SJF\n";
    std::cout << "3. RR\n";
    std::cout << "4. Priority\n";
    std::cout << "5. MLQ\n";
    std::cout << "6. MLFQ\n";
    std::cout << "输入选项: ";

    int choice = 0;
    std::cin >> choice;
    switch (choice) {
        case 1:
            return Algorithm::FCFS;
        case 2:
            return Algorithm::SJF;
        case 3:
            return Algorithm::RR;
        case 4:
            return Algorithm::Priority;
        case 5:
            return Algorithm::MLQ;
        case 6:
            return Algorithm::MLFQ;
        default:
            throw std::invalid_argument("无效选项。");
    }
}

MLQConfig readMLQConfig() {
    MLQConfig config = scheduling::defaultMLQConfig();
    std::cout << "请输入 3 个 MLQ 队列的 调度优先级 和 时间片。时间片填 0 表示 FCFS。\n";
    for (auto& queue : config.queues) {
        std::cout << queue.name << " 调度优先级 时间片: ";
        std::cin >> queue.dispatchPriority >> queue.timeSlice;
    }

    const std::string validationMessage = scheduling::validateMLQConfig(config);
    if (!validationMessage.empty()) {
        throw std::invalid_argument(validationMessage);
    }
    return config;
}

MLFQConfig readMLFQConfig() {
    MLFQConfig config = scheduling::defaultMLFQConfig();
    std::cout << "请输入 3 个 MLFQ 队列的 调度算法 和 时间片。\n";
    std::cout << "队列算法选项: 1.FCFS 2.SJF 3.RR 4.Priority\n";
    for (auto& queue : config.queues) {
        int choice = 0;
        std::cout << queue.name << " 算法选项 时间片: ";
        std::cin >> choice >> queue.timeSlice;
        queue.discipline = disciplineFromChoice(choice);
    }

    const std::string validationMessage = scheduling::validateMLFQConfig(config);
    if (!validationMessage.empty()) {
        throw std::invalid_argument(validationMessage);
    }

    std::cout << "当前 MLFQ 配置:\n";
    for (const auto& queue : config.queues) {
        std::cout << "- " << queue.name << ": " << disciplineLabel(queue.discipline)
                  << ", 时间片 " << queue.timeSlice << '\n';
    }
    return config;
}

}  // namespace

int main() {
    std::cout << "=== 处理机调度实验（CLI 备用入口）===\n";
    std::cout << "请输入进程数量: ";

    int n = 0;
    if (!(std::cin >> n) || n <= 0) {
        std::cerr << "进程数量非法。\n";
        return 1;
    }

    std::vector<ProcessInput> processes(n);
    std::cout << "请依次输入每个进程的 名称 到达时间 运行时间 优先级:\n";
    for (int i = 0; i < n; ++i) {
        std::cin >> processes[i].name >> processes[i].arrival >> processes[i].burst >> processes[i].priority;
    }

    const std::string validationMessage = scheduling::validateProcesses(processes);
    if (!validationMessage.empty()) {
        std::cerr << validationMessage << '\n';
        return 1;
    }

    try {
        const Algorithm algorithm = readAlgorithmChoice();
        int quantum = 1;
        MLQConfig mlqConfig = scheduling::defaultMLQConfig();
        MLFQConfig mlfqConfig = scheduling::defaultMLFQConfig();

        if (algorithm == Algorithm::RR) {
            std::cout << "请输入 RR 时间片长度: ";
            std::cin >> quantum;
        } else if (algorithm == Algorithm::MLQ) {
            mlqConfig = readMLQConfig();
        } else if (algorithm == Algorithm::MLFQ) {
            mlfqConfig = readMLFQConfig();
        }

        const auto results = scheduling::runScheduling(processes, algorithm, quantum, mlqConfig, mlfqConfig);
        if (results.empty()) {
            std::cerr << "未生成调度结果。\n";
            return 1;
        }
        printResult(results.front());
    } catch (const std::exception& ex) {
        std::cerr << ex.what() << '\n';
        return 1;
    }

    return 0;
}
