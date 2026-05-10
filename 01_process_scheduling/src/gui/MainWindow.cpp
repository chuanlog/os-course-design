#include "MainWindow.h"

#include "../scheduling/SchedulingUtils.h"

#include <FL/Fl_Box.H>
#include <FL/Fl_Browser.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Hold_Browser.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Int_Input.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_Text_Display.H>
#include <FL/fl_ask.H>

#include <algorithm>
#include <random>
#include <sstream>
#include <string>

namespace {

const char* kQueueNames[3] = {"高队列", "中队列", "低队列"};

const char* disciplineLabel(scheduling::QueueDiscipline discipline) {
    switch (discipline) {
        case scheduling::QueueDiscipline::FCFS:
            return "FCFS";
        case scheduling::QueueDiscipline::SJF:
            return "SJF";
        case scheduling::QueueDiscipline::RR:
            return "RR";
        case scheduling::QueueDiscipline::Priority:
            return "Priority";
    }
    return "FCFS";
}

int disciplineChoiceIndex(scheduling::QueueDiscipline discipline) {
    switch (discipline) {
        case scheduling::QueueDiscipline::FCFS:
            return 0;
        case scheduling::QueueDiscipline::SJF:
            return 1;
        case scheduling::QueueDiscipline::RR:
            return 2;
        case scheduling::QueueDiscipline::Priority:
            return 3;
    }
    return 0;
}

scheduling::QueueDiscipline disciplineFromChoiceIndex(int value) {
    switch (value) {
        case 0:
            return scheduling::QueueDiscipline::FCFS;
        case 1:
            return scheduling::QueueDiscipline::SJF;
        case 2:
            return scheduling::QueueDiscipline::RR;
        case 3:
            return scheduling::QueueDiscipline::Priority;
        default:
            return scheduling::QueueDiscipline::FCFS;
    }
}

}  // namespace

MainWindow::MainWindow(int width, int height, const char* title)
    : Fl_Double_Window(width, height, title) {
    buildUi();
    loadDemoData();
    applyDefaultMLQConfig();
    applyDefaultMLFQConfig();
    updateConfigPanelVisibility();
    end();
    resizable(this);
}

void MainWindow::onAddProcess(Fl_Widget*, void* data) {
    static_cast<MainWindow*>(data)->addProcess();
}

void MainWindow::onRemoveSelected(Fl_Widget*, void* data) {
    static_cast<MainWindow*>(data)->removeSelectedProcess();
}

void MainWindow::onClearProcesses(Fl_Widget*, void* data) {
    static_cast<MainWindow*>(data)->clearProcesses();
}

void MainWindow::onLoadDemo(Fl_Widget*, void* data) {
    static_cast<MainWindow*>(data)->loadDemoData();
}

void MainWindow::onGenerateRandom(Fl_Widget*, void* data) {
    static_cast<MainWindow*>(data)->generateRandomProcesses();
}

void MainWindow::onRunSimulation(Fl_Widget*, void* data) {
    static_cast<MainWindow*>(data)->runSimulation();
}

void MainWindow::onAlgorithmChanged(Fl_Widget*, void* data) {
    static_cast<MainWindow*>(data)->updateConfigPanelVisibility();
}

void MainWindow::onProcessSelected(Fl_Widget*, void* data) {
    static_cast<MainWindow*>(data)->updateSelectedProcessInfo();
}

void MainWindow::buildUi() {
    begin();

    auto* titleBox = new Fl_Box(20, 15, 420, 30, "处理机调度模拟器");
    titleBox->labelfont(FL_HELVETICA_BOLD);
    titleBox->labelsize(22);
    titleBox->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);

    auto* inputGroup = new Fl_Group(20, 80, 1240, 120, "进程输入");
    inputGroup->box(FL_ENGRAVED_BOX);
    inputGroup->labelfont(FL_HELVETICA_BOLD);
    inputGroup->begin();

    nameInput_ = new Fl_Input(90, 105, 120, 28, "进程名:");
    arrivalInput_ = new Fl_Int_Input(320, 105, 90, 28, "到达时间:");
    burstInput_ = new Fl_Int_Input(520, 105, 90, 28, "运行时间:");
    priorityInput_ = new Fl_Int_Input(720, 105, 90, 28, "优先级:");
    auto* randomCountLabel = new Fl_Box(970, 149, 50, 24, "个数:");
    randomCountLabel->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    randomCountInput_ = new Fl_Int_Input(1020, 145, 50, 28);
    randomCountInput_->value("5");

    auto* addButton = new Fl_Button(860, 102, 110, 32, "添加进程");
    auto* removeButton = new Fl_Button(980, 102, 110, 32, "删除选中");
    auto* clearButton = new Fl_Button(1100, 102, 110, 32, "清空列表");
    auto* demoButton = new Fl_Button(860, 145, 100, 32, "载入示例");
    auto* randomButton = new Fl_Button(1080, 145, 130, 32, "随机生成");

    addButton->callback(&MainWindow::onAddProcess, this);
    removeButton->callback(&MainWindow::onRemoveSelected, this);
    clearButton->callback(&MainWindow::onClearProcesses, this);
    demoButton->callback(&MainWindow::onLoadDemo, this);
    randomButton->callback(&MainWindow::onGenerateRandom, this);

    inputGroup->end();

    auto* processGroup = new Fl_Group(20, 215, 600, 200, "进程列表");
    processGroup->box(FL_ENGRAVED_BOX);
    processGroup->labelfont(FL_HELVETICA_BOLD);
    processGroup->begin();

    processBrowser_ = new Fl_Hold_Browser(35, 245, 220, 150);
    processBrowser_->textsize(14);
    processBrowser_->callback(&MainWindow::onProcessSelected, this);

    auto* selectedTitle = new Fl_Box(280, 240, 180, 24, "选中进程信息");
    selectedTitle->labelfont(FL_HELVETICA_BOLD);
    selectedTitle->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);

    selectedNameOutput_ = new Fl_Output(350, 270, 220, 28, "进程名:");
    selectedArrivalOutput_ = new Fl_Output(350, 305, 220, 28, "到达时间:");
    selectedBurstOutput_ = new Fl_Output(350, 340, 220, 28, "运行时间:");
    selectedPriorityOutput_ = new Fl_Output(350, 375, 220, 28, "优先级:");

    processGroup->end();

    auto* optionsGroup = new Fl_Group(640, 215, 620, 230, "运行配置");
    optionsGroup->box(FL_ENGRAVED_BOX);
    optionsGroup->labelfont(FL_HELVETICA_BOLD);
    optionsGroup->begin();

    algorithmChoice_ = new Fl_Choice(760, 240, 190, 28, "调度算法:");
    algorithmChoice_->add("FCFS");
    algorithmChoice_->add("SJF");
    algorithmChoice_->add("RR");
    algorithmChoice_->add("Priority");
    algorithmChoice_->add("MLQ");
    algorithmChoice_->add("MLFQ");
    algorithmChoice_->value(0);
    algorithmChoice_->callback(&MainWindow::onAlgorithmChanged, this);

    auto* runButton = new Fl_Button(1020, 238, 190, 30, "运行模拟");
    runButton->labelfont(FL_HELVETICA_BOLD);
    runButton->callback(&MainWindow::onRunSimulation, this);

    rrConfigGroup_ = new Fl_Group(660, 278, 580, 60);
    rrConfigGroup_->begin();
    auto* rrTitle = new Fl_Box(660, 278, 220, 22, "RR 配置");
    rrTitle->labelfont(FL_HELVETICA_BOLD);
    rrTitle->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    quantumInput_ = new Fl_Int_Input(760, 306, 80, 28, "时间片:");
    quantumInput_->value("2");
    rrConfigGroup_->end();

    mlqConfigGroup_ = new Fl_Group(660, 278, 580, 130);
    mlqConfigGroup_->begin();
    auto* mlqTitle = new Fl_Box(660, 278, 320, 22, "MLQ 队列配置（优先级越小越先调度）");
    mlqTitle->labelfont(FL_HELVETICA_BOLD);
    mlqTitle->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    for (int i = 0; i < 3; ++i) {
        const int y = 308 + i * 30;
        auto* rowLabel = new Fl_Box(665, y, 65, 24, kQueueNames[i]);
        rowLabel->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
        mlqPriorityInputs_[i] = new Fl_Int_Input(760, y, 55, 24, "优先级:");
        mlqQuantumInputs_[i] = new Fl_Int_Input(900, y, 60, 24, "时间片:");
    }
    auto* mlqTip = new Fl_Box(980, 308, 240, 84, "说明:\n高队列处理 priority<=2\n中队列处理 priority<=4\n低队列处理 priority>=5\n时间片 0 表示 FCFS");
    mlqTip->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE | FL_ALIGN_TOP);
    mlqConfigGroup_->end();

    mlfqConfigGroup_ = new Fl_Group(660, 278, 580, 130);
    mlfqConfigGroup_->begin();
    auto* mlfqTitle = new Fl_Box(660, 278, 320, 22, "MLFQ 队列配置（新进程先进入高队列）");
    mlfqTitle->labelfont(FL_HELVETICA_BOLD);
    mlfqTitle->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    for (int i = 0; i < 3; ++i) {
        const int y = 308 + i * 30;
        auto* rowLabel = new Fl_Box(665, y, 65, 24, kQueueNames[i]);
        rowLabel->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
        mlfqAlgorithmChoices_[i] = new Fl_Choice(760, y, 120, 24, "算法:");
        mlfqAlgorithmChoices_[i]->add("FCFS");
        mlfqAlgorithmChoices_[i]->add("SJF");
        mlfqAlgorithmChoices_[i]->add("RR");
        mlfqAlgorithmChoices_[i]->add("Priority");
        mlfqQuantumInputs_[i] = new Fl_Int_Input(980, y, 60, 24, "时间片:");
    }
    auto* mlfqTip = new Fl_Box(1060, 308, 170, 84, "说明:\n新到达进程进入高队列\n用完时间片可降级\n高队列可抢占低队列");
    mlfqTip->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE | FL_ALIGN_TOP);
    mlfqConfigGroup_->end();

    optionsGroup->end();

    ganttChart_ = new GanttChartWidget(20, 470, 1240, 170);

    auto* detailHeader = new Fl_Box(20, 655, 720, 22, "调度结果");
    detailHeader->labelfont(FL_HELVETICA_BOLD);
    detailHeader->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);

    detailTable_ = new ResultsTable(20, 680, 720, 220);

    auto* summaryHeader = new Fl_Box(760, 655, 500, 22, "结果摘要");
    summaryHeader->labelfont(FL_HELVETICA_BOLD);
    summaryHeader->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);

    summaryBuffer_ = new Fl_Text_Buffer();
    summaryOutput_ = new Fl_Text_Display(760, 680, 500, 220, "");
    summaryOutput_->buffer(summaryBuffer_);
    summaryOutput_->textfont(FL_COURIER);
    summaryOutput_->textsize(13);
    summaryOutput_->wrap_mode(Fl_Text_Display::WRAP_AT_BOUNDS, 0);

    statusBox_ = new Fl_Box(20, 920, 1240, 20, "就绪。可以直接运行示例数据。");
    statusBox_->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    statusBox_->labelfont(FL_HELVETICA_BOLD);

    end();
}

void MainWindow::loadDemoData() {
    processes_ = {
        {"P1", 0, 5, 2},
        {"P2", 1, 3, 1},
        {"P3", 2, 8, 4},
        {"P4", 3, 6, 3},
        {"P5", 4, 4, 6},
    };
    updateProcessBrowser();
    clearProcessInputs();
    updateStatus("已载入示例数据。可以直接点击运行模拟。");
}

void MainWindow::applyDefaultMLQConfig() {
    const auto config = scheduling::defaultMLQConfig();
    for (int i = 0; i < 3; ++i) {
        static std::string priorityText;
        static std::string quantumText;
        priorityText = std::to_string(config.queues[i].dispatchPriority);
        quantumText = std::to_string(config.queues[i].timeSlice);
        mlqPriorityInputs_[i]->value(priorityText.c_str());
        mlqQuantumInputs_[i]->value(quantumText.c_str());
    }
}

void MainWindow::applyDefaultMLFQConfig() {
    const auto config = scheduling::defaultMLFQConfig();
    for (int i = 0; i < 3; ++i) {
        static std::string quantumText;
        mlfqAlgorithmChoices_[i]->value(disciplineChoiceIndex(config.queues[i].discipline));
        quantumText = std::to_string(config.queues[i].timeSlice);
        mlfqQuantumInputs_[i]->value(quantumText.c_str());
    }
}

void MainWindow::clearProcessInputs() {
    nameInput_->value("");
    arrivalInput_->value("0");
    burstInput_->value("1");
    priorityInput_->value("1");
}

void MainWindow::clearSelectedProcessInfo() {
    selectedNameOutput_->value("-");
    selectedArrivalOutput_->value("-");
    selectedBurstOutput_->value("-");
    selectedPriorityOutput_->value("-");
}

void MainWindow::updateProcessBrowser() {
    processBrowser_->clear();
    for (std::size_t i = 0; i < processes_.size(); ++i) {
        const auto& process = processes_[i];
        std::ostringstream oss;
        oss << (i + 1) << ". " << process.name;
        processBrowser_->add(oss.str().c_str());
    }

    if (!processes_.empty()) {
        processBrowser_->select(1);
        updateSelectedProcessInfo();
    } else {
        clearSelectedProcessInfo();
    }
}

void MainWindow::updateSelectedProcessInfo() {
    const int selected = processBrowser_->value();
    const int processIndex = selected - 1;
    if (processIndex < 0 || processIndex >= static_cast<int>(processes_.size())) {
        clearSelectedProcessInfo();
        return;
    }

    const auto& process = processes_[processIndex];
    static std::string nameText;
    static std::string arrivalText;
    static std::string burstText;
    static std::string priorityText;

    nameText = process.name;
    arrivalText = std::to_string(process.arrival);
    burstText = std::to_string(process.burst);
    priorityText = std::to_string(process.priority);

    selectedNameOutput_->value(nameText.c_str());
    selectedArrivalOutput_->value(arrivalText.c_str());
    selectedBurstOutput_->value(burstText.c_str());
    selectedPriorityOutput_->value(priorityText.c_str());
}

void MainWindow::updateDetailTable(const scheduling::SimulationResult& result) {
    detailTable_->setResult(result);
}

void MainWindow::clearDetailTable() {
    detailTable_->clearResult();
}

void MainWindow::updateSummaryOutput(
    const scheduling::SimulationResult& result,
    const scheduling::MLQConfig& mlqConfig,
    const scheduling::MLFQConfig& mlfqConfig) {
    std::ostringstream oss;
    oss << "当前算法: " << result.algorithmName << "\n";

    if (selectedAlgorithm() == scheduling::Algorithm::MLQ) {
        oss << "MLQ 配置:\n";
        for (const auto& queue : mlqConfig.queues) {
            oss << "- " << queue.name << " => 调度优先级 " << queue.dispatchPriority << ", 时间片 " << queue.timeSlice;
            if (queue.timeSlice == 0) {
                oss << " (FCFS)";
            }
            oss << '\n';
        }
        oss << '\n';
    } else if (selectedAlgorithm() == scheduling::Algorithm::MLFQ) {
        oss << "MLFQ 配置:\n";
        for (const auto& queue : mlfqConfig.queues) {
            oss << "- " << queue.name << " => 算法 " << disciplineLabel(queue.discipline)
                << ", 时间片 " << queue.timeSlice << '\n';
        }
        oss << '\n';
    }

    oss << "平均周转时间: " << result.avgTurnaround << "\n";
    oss << "平均等待时间: " << result.avgWaiting << "\n";
    oss << "平均响应时间: " << result.avgResponse << "\n\n";
    oss << "时间线:\n";
    for (const auto& segment : result.timeline) {
        oss << '[' << segment.start << ", " << segment.end << ") " << segment.name << '\n';
    }

    static std::string summaryText;
    summaryText = oss.str();
    summaryBuffer_->text(summaryText.c_str());
}

void MainWindow::updateStatus(const std::string& message) {
    statusBox_->copy_label(message.c_str());
    redraw();
}

void MainWindow::updateConfigPanelVisibility() {
    const auto algorithm = selectedAlgorithm();

    rrConfigGroup_->hide();
    mlqConfigGroup_->hide();
    mlfqConfigGroup_->hide();
    quantumInput_->deactivate();

    for (int i = 0; i < 3; ++i) {
        mlqPriorityInputs_[i]->deactivate();
        mlqQuantumInputs_[i]->deactivate();
        mlfqAlgorithmChoices_[i]->deactivate();
        mlfqQuantumInputs_[i]->deactivate();
    }

    if (algorithm == scheduling::Algorithm::RR) {
        rrConfigGroup_->show();
        quantumInput_->activate();
    } else if (algorithm == scheduling::Algorithm::MLQ) {
        mlqConfigGroup_->show();
        for (int i = 0; i < 3; ++i) {
            mlqPriorityInputs_[i]->activate();
            mlqQuantumInputs_[i]->activate();
        }
    } else if (algorithm == scheduling::Algorithm::MLFQ) {
        mlfqConfigGroup_->show();
        for (int i = 0; i < 3; ++i) {
            mlfqAlgorithmChoices_[i]->activate();
            mlfqQuantumInputs_[i]->activate();
        }
    }

    redraw();
}

void MainWindow::addProcess() {
    int arrival = 0;
    int burst = 0;
    int priority = 0;
    if (std::string(nameInput_->value()).empty()) {
        fl_alert("进程名称不能为空。");
        return;
    }
    if (!parseInteger(arrivalInput_, &arrival) || !parseInteger(burstInput_, &burst) || !parseInteger(priorityInput_, &priority)) {
        fl_alert("请确保到达时间、运行时间、优先级都是整数。");
        return;
    }

    scheduling::ProcessInput process{nameInput_->value(), arrival, burst, priority};
    const std::vector<scheduling::ProcessInput> candidate{process};
    const std::string validationMessage = scheduling::validateProcesses(candidate);
    if (!validationMessage.empty()) {
        fl_alert("%s", validationMessage.c_str());
        return;
    }

    processes_.push_back(process);
    updateProcessBrowser();
    clearProcessInputs();
    updateStatus("已添加一个进程。可以继续添加或直接运行模拟。");
}

void MainWindow::generateRandomProcesses() {
    int count = 0;
    if (!parseInteger(randomCountInput_, &count) || count <= 0) {
        fl_alert("随机生成的进程个数必须是正整数。");
        return;
    }
    if (count > 50) {
        fl_alert("为便于展示，随机生成的进程个数请不要超过 50。");
        return;
    }

    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> arrivalDist(0, std::max(3, count * 2));
    std::uniform_int_distribution<int> burstDist(1, 10);
    std::uniform_int_distribution<int> priorityDist(1, 9);

    processes_.clear();
    processes_.reserve(static_cast<std::size_t>(count));
    for (int i = 0; i < count; ++i) {
        scheduling::ProcessInput process;
        process.name = "P" + std::to_string(i + 1);
        process.arrival = arrivalDist(rng);
        process.burst = burstDist(rng);
        process.priority = priorityDist(rng);
        processes_.push_back(process);
    }

    std::sort(processes_.begin(), processes_.end(), [](const scheduling::ProcessInput& lhs, const scheduling::ProcessInput& rhs) {
        if (lhs.arrival != rhs.arrival) {
            return lhs.arrival < rhs.arrival;
        }
        return lhs.name < rhs.name;
    });

    updateProcessBrowser();
    clearDetailTable();
    if (summaryBuffer_ != nullptr) {
        summaryBuffer_->text("");
    }
    ganttChart_->clearTimeline();
    updateStatus("已随机生成进程数据，可直接运行模拟。");
}

void MainWindow::removeSelectedProcess() {
    const int selected = processBrowser_->value();
    const int processIndex = selected - 1;
    if (processIndex < 0 || processIndex >= static_cast<int>(processes_.size())) {
        fl_alert("请先在进程列表中选择一个进程。");
        return;
    }

    processes_.erase(processes_.begin() + processIndex);
    updateProcessBrowser();
    updateStatus("已删除选中的进程。");
}

void MainWindow::clearProcesses() {
    processes_.clear();
    updateProcessBrowser();
    clearDetailTable();
    if (summaryBuffer_ != nullptr) {
        summaryBuffer_->text("");
    }
    ganttChart_->clearTimeline();
    updateStatus("已清空所有进程和运行结果。");
}

scheduling::MLQConfig MainWindow::currentMLQConfig() const {
    scheduling::MLQConfig config;
    for (int i = 0; i < 3; ++i) {
        int dispatchPriority = 0;
        int timeSlice = 0;
        if (!parseInteger(mlqPriorityInputs_[i], &dispatchPriority) || !parseInteger(mlqQuantumInputs_[i], &timeSlice)) {
            throw std::invalid_argument("MLQ 队列优先级和时间片必须为整数。");
        }
        config.queues.push_back({kQueueNames[i], dispatchPriority, timeSlice});
    }

    const std::string validationMessage = scheduling::validateMLQConfig(config);
    if (!validationMessage.empty()) {
        throw std::invalid_argument(validationMessage);
    }
    return config;
}

scheduling::MLFQConfig MainWindow::currentMLFQConfig() const {
    scheduling::MLFQConfig config;
    for (int i = 0; i < 3; ++i) {
        int timeSlice = 0;
        if (!parseInteger(mlfqQuantumInputs_[i], &timeSlice)) {
            throw std::invalid_argument("MLFQ 队列时间片必须为整数。");
        }
        config.queues.push_back({kQueueNames[i], disciplineFromChoiceIndex(mlfqAlgorithmChoices_[i]->value()), timeSlice});
    }

    const std::string validationMessage = scheduling::validateMLFQConfig(config);
    if (!validationMessage.empty()) {
        throw std::invalid_argument(validationMessage);
    }
    return config;
}

void MainWindow::runSimulation() {
    const std::string validationMessage = scheduling::validateProcesses(processes_);
    if (!validationMessage.empty()) {
        fl_alert("%s", validationMessage.c_str());
        return;
    }

    int quantum = 1;
    scheduling::MLQConfig mlqConfig = scheduling::defaultMLQConfig();
    scheduling::MLFQConfig mlfqConfig = scheduling::defaultMLFQConfig();

    try {
        if (selectedAlgorithm() == scheduling::Algorithm::RR) {
            if (!parseInteger(quantumInput_, &quantum)) {
                throw std::invalid_argument("RR 时间片必须是整数。");
            }
        } else if (selectedAlgorithm() == scheduling::Algorithm::MLQ) {
            mlqConfig = currentMLQConfig();
        } else if (selectedAlgorithm() == scheduling::Algorithm::MLFQ) {
            mlfqConfig = currentMLFQConfig();
        }

        const auto results = scheduling::runScheduling(processes_, selectedAlgorithm(), quantum, mlqConfig, mlfqConfig);
        if (results.empty()) {
            fl_alert("未生成任何调度结果。");
            return;
        }

        updateDetailTable(results.front());
        updateSummaryOutput(results.front(), mlqConfig, mlfqConfig);
        ganttChart_->setTimeline(results.front().timeline);
        updateStatus("调度完成。界面已更新 Gantt 图、表格结果和结果摘要。");
    } catch (const std::exception& ex) {
        fl_alert("运行失败: %s", ex.what());
    }
}

scheduling::Algorithm MainWindow::selectedAlgorithm() const {
    switch (algorithmChoice_->value()) {
        case 0:
            return scheduling::Algorithm::FCFS;
        case 1:
            return scheduling::Algorithm::SJF;
        case 2:
            return scheduling::Algorithm::RR;
        case 3:
            return scheduling::Algorithm::Priority;
        case 4:
            return scheduling::Algorithm::MLQ;
        case 5:
            return scheduling::Algorithm::MLFQ;
        default:
            return scheduling::Algorithm::FCFS;
    }
}

bool MainWindow::parseInteger(const Fl_Input* input, int* value) const {
    if (input == nullptr || value == nullptr) {
        return false;
    }

    try {
        std::size_t parsedLength = 0;
        const std::string text = input->value();
        const int parsed = std::stoi(text, &parsedLength);
        if (parsedLength != text.size()) {
            return false;
        }
        *value = parsed;
        return true;
    } catch (...) {
        return false;
    }
}
