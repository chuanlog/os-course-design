#include "MainWindow.h"

#include "../memory/MemoryFacade.h"
#include "../memory/PageReplacement.h"

#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Int_Input.H>
#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_Text_Display.H>
#include <FL/fl_ask.H>

#include <algorithm>
#include <iomanip>
#include <random>
#include <sstream>
#include <string>
#include <vector>

namespace {

int alignedSize(int rawSize, int unit) {
    return ((rawSize + unit - 1) / unit) * unit;
}

std::string joinPages(const std::vector<int>& pages) {
    std::ostringstream oss;
    for (std::size_t i = 0; i < pages.size(); ++i) {
        if (i != 0U) {
            oss << ' ';
        }
        oss << pages[i];
    }
    return oss.str();
}

}  // namespace

MainWindow::MainWindow(int width, int height, const char* title)
    : Fl_Double_Window(width, height, title) {
    buildUi();
    updateModeVisibility();
    end();
    resizable(this);
}

void MainWindow::onModeChanged(Fl_Widget*, void* data) {
    static_cast<MainWindow*>(data)->updateModeVisibility();
}

void MainWindow::onInitPartition(Fl_Widget*, void* data) {
    static_cast<MainWindow*>(data)->initPartition();
}

void MainWindow::onAllocatePartition(Fl_Widget*, void* data) {
    static_cast<MainWindow*>(data)->allocatePartition();
}

void MainWindow::onReleasePartition(Fl_Widget*, void* data) {
    static_cast<MainWindow*>(data)->releasePartition();
}

void MainWindow::onLoadPartitionDemo(Fl_Widget*, void* data) {
    static_cast<MainWindow*>(data)->loadPartitionDemo();
}

void MainWindow::onGeneratePartitionSequence(Fl_Widget*, void* data) {
    static_cast<MainWindow*>(data)->generatePartitionSequence();
}

void MainWindow::onStartPartitionPlayback(Fl_Widget*, void* data) {
    static_cast<MainWindow*>(data)->startPartitionPlayback();
}

void MainWindow::onStopPartitionPlayback(Fl_Widget*, void* data) {
    static_cast<MainWindow*>(data)->stopPartitionPlayback();
}

void MainWindow::onRunPaging(Fl_Widget*, void* data) {
    static_cast<MainWindow*>(data)->runPaging();
}

void MainWindow::onLoadPagingDemo(Fl_Widget*, void* data) {
    static_cast<MainWindow*>(data)->loadPagingDemo();
}

void MainWindow::onGeneratePagingRandom(Fl_Widget*, void* data) {
    static_cast<MainWindow*>(data)->generatePagingRandom();
}

void MainWindow::onPartitionPlaybackTick(void* data) {
    static_cast<MainWindow*>(data)->handlePartitionPlaybackTick();
}

void MainWindow::buildUi() {
    begin();

    auto* titleBox = new Fl_Box(20, 15, 420, 30, "内存管理模拟器");
    titleBox->labelfont(FL_HELVETICA_BOLD);
    titleBox->labelsize(22);
    titleBox->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);

    auto* modeLabel = new Fl_Box(20, 52, 80, 24, "模式:");
    modeLabel->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);

    modeChoice_ = new Fl_Choice(90, 48, 240, 28);
    modeChoice_->add("动态分区分配");
    modeChoice_->add("页面置换");
    modeChoice_->value(0);
    modeChoice_->callback(&MainWindow::onModeChanged, this);

    partitionGroup_ = new Fl_Group(20, 90, 1240, 560, "动态分区分配");
    partitionGroup_->box(FL_ENGRAVED_BOX);
    partitionGroup_->labelfont(FL_HELVETICA_BOLD);
    partitionGroup_->begin();

    totalMemoryInput_ = new Fl_Int_Input(120, 120, 90, 28, "总内存:");
    totalMemoryInput_->value("640");

    partitionStrategyChoice_ = new Fl_Choice(340, 120, 100, 28, "策略:");
    partitionStrategyChoice_->add("FF");
    partitionStrategyChoice_->add("BF");
    partitionStrategyChoice_->value(0);

    auto* initButton = new Fl_Button(470, 118, 110, 32, "初始化");
    auto* partitionDemoButton = new Fl_Button(590, 118, 140, 32, "载入分区示例");
    initButton->callback(&MainWindow::onInitPartition, this);
    partitionDemoButton->callback(&MainWindow::onLoadPartitionDemo, this);

    partitionNameInput_ = new Fl_Input(120, 165, 100, 28, "进程名:");
    partitionSizeInput_ = new Fl_Int_Input(340, 165, 100, 28, "大小:");
    auto* allocateButton = new Fl_Button(470, 163, 110, 32, "申请内存");
    auto* releaseButton = new Fl_Button(590, 163, 110, 32, "释放内存");
    allocateButton->callback(&MainWindow::onAllocatePartition, this);
    releaseButton->callback(&MainWindow::onReleasePartition, this);

    partitionRandomStepsInput_ = new Fl_Int_Input(120, 210, 70, 28, "随机步数:");
    partitionRandomStepsInput_->value("8");
    partitionPlaybackIntervalInput_ = new Fl_Int_Input(340, 210, 80, 28, "间隔ms:");
    partitionPlaybackIntervalInput_->value("700");

    auto* generateSequenceButton = new Fl_Button(470, 208, 130, 32, "生成序列");
    auto* startPlaybackButton = new Fl_Button(610, 208, 130, 32, "开始回放");
    auto* stopPlaybackButton = new Fl_Button(750, 208, 130, 32, "停止回放");
    generateSequenceButton->callback(&MainWindow::onGeneratePartitionSequence, this);
    startPlaybackButton->callback(&MainWindow::onStartPartitionPlayback, this);
    stopPlaybackButton->callback(&MainWindow::onStopPartitionPlayback, this);

    memoryLayoutWidget_ = new MemoryLayoutWidget(20, 255, 1240, 145);

    auto* allocatedHeader = new Fl_Box(20, 410, 220, 22, "已分配分区");
    allocatedHeader->labelfont(FL_HELVETICA_BOLD);
    allocatedHeader->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    allocatedTable_ = new BlocksTable(20, 435, 600, 190);

    auto* freeHeader = new Fl_Box(660, 410, 220, 22, "空闲分区");
    freeHeader->labelfont(FL_HELVETICA_BOLD);
    freeHeader->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    freeTable_ = new BlocksTable(660, 435, 600, 190);

    partitionGroup_->end();

    pagingGroup_ = new Fl_Group(20, 90, 1240, 560, "页面置换");
    pagingGroup_->box(FL_ENGRAVED_BOX);
    pagingGroup_->labelfont(FL_HELVETICA_BOLD);
    pagingGroup_->begin();

    frameCountInput_ = new Fl_Int_Input(120, 120, 90, 28, "页框数:");
    frameCountInput_->value("3");

    pageAlgorithmChoice_ = new Fl_Choice(360, 120, 140, 28, "算法:");
    pageAlgorithmChoice_->add("FIFO");
    pageAlgorithmChoice_->add("LRU");
    pageAlgorithmChoice_->add("LFU");
    pageAlgorithmChoice_->add("CLOCK");
    pageAlgorithmChoice_->add("RANDOM");
    pageAlgorithmChoice_->value(0);

    auto* pagingRunButton = new Fl_Button(520, 118, 110, 32, "运行模拟");
    auto* pagingDemoButton = new Fl_Button(640, 118, 140, 32, "载入置换示例");
    pagingRunButton->callback(&MainWindow::onRunPaging, this);
    pagingDemoButton->callback(&MainWindow::onLoadPagingDemo, this);

    pagingRandomLengthInput_ = new Fl_Int_Input(940, 120, 80, 28, "随机长度:");
    pagingRandomLengthInput_->value("12");
    auto* pagingRandomButton = new Fl_Button(1040, 118, 180, 32, "随机生成输入");
    pagingRandomButton->callback(&MainWindow::onGeneratePagingRandom, this);

    pageSequenceInput_ = new Fl_Input(120, 165, 1100, 28, "访问序列:");
    pageSequenceInput_->value("1 2 3 4 1 2 5 1 2 3 4 5");

    auto* pagingHeader = new Fl_Box(20, 220, 220, 22, "页面置换步骤");
    pagingHeader->labelfont(FL_HELVETICA_BOLD);
    pagingHeader->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    pageStepsTable_ = new PageStepsTable(20, 245, 650, 380);

    pagingGroup_->end();

    auto* summaryHeader = new Fl_Box(20, 665, 220, 22, "结果摘要");
    summaryHeader->labelfont(FL_HELVETICA_BOLD);
    summaryHeader->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);

    summaryBuffer_ = new Fl_Text_Buffer();
    summaryDisplay_ = new Fl_Text_Display(20, 690, 1240, 190, "");
    summaryDisplay_->buffer(summaryBuffer_);
    summaryDisplay_->textfont(FL_COURIER);
    summaryDisplay_->textsize(13);
    summaryDisplay_->wrap_mode(Fl_Text_Display::WRAP_AT_BOUNDS, 0);

    statusBox_ = new Fl_Box(20, 890, 1240, 20, "就绪。请选择模式并开始模拟。");
    statusBox_->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    statusBox_->labelfont(FL_HELVETICA_BOLD);

    end();
}

void MainWindow::updateModeVisibility() {
    if (modeChoice_->value() == 0) {
        partitionGroup_->show();
        pagingGroup_->hide();
        updateStatus("当前模式：动态分区分配。");
    } else {
        partitionGroup_->hide();
        pagingGroup_->show();
        updateStatus("当前模式：页面置换。");
    }
    redraw();
}

void MainWindow::initPartition() {
    stopPartitionPlayback();

    int totalSize = 0;
    if (!parseInteger(totalMemoryInput_, &totalSize)) {
        fl_alert("总内存必须是整数。");
        return;
    }
    const std::string validationMessage = memory_management::validatePartitionInit(totalSize);
    if (!validationMessage.empty()) {
        fl_alert("%s", validationMessage.c_str());
        return;
    }

    partitionManager_.reset(totalSize, selectedPartitionStrategy());
    partitionInitialized_ = true;
    const auto state = partitionManager_.snapshot("初始化");
    updatePartitionView(state);
    std::ostringstream oss;
    oss << "初始化成功\n总内存: " << totalSize << "\n策略: " << memory_management::partitionStrategyName(selectedPartitionStrategy()) << '\n';
    if (!partitionSequence_.empty()) {
        oss << '\n' << buildPartitionSequenceSummary();
    }
    updateSummary(oss.str());
    updateStatus("已初始化分区管理器。");
}

void MainWindow::allocatePartition() {
    stopPartitionPlayback();
    if (!partitionInitialized_) {
        fl_alert("请先初始化分区管理器。");
        return;
    }
    int size = 0;
    if (!parseInteger(partitionSizeInput_, &size)) {
        fl_alert("申请大小必须是整数。");
        return;
    }
    const auto result = partitionManager_.allocate(partitionNameInput_->value(), size);
    updatePartitionView(result.snapshot);
    std::ostringstream oss;
    oss << result.snapshot.action << '\n' << result.message << "\n\n"
        << "当前内存布局:\n"
        << memory_management::blocksToText(memory_management::buildMemoryLayout(result.snapshot, partitionManager_.totalSize()));
    updateSummary(oss.str());
    updateStatus(result.message);
}

void MainWindow::releasePartition() {
    stopPartitionPlayback();
    if (!partitionInitialized_) {
        fl_alert("请先初始化分区管理器。");
        return;
    }
    const auto result = partitionManager_.release(partitionNameInput_->value());
    updatePartitionView(result.snapshot);
    std::ostringstream oss;
    oss << result.snapshot.action << '\n' << result.message << "\n\n"
        << "当前内存布局:\n"
        << memory_management::blocksToText(memory_management::buildMemoryLayout(result.snapshot, partitionManager_.totalSize()));
    updateSummary(oss.str());
    updateStatus(result.message);
}

void MainWindow::loadPartitionDemo() {
    stopPartitionPlayback();
    totalMemoryInput_->value("640");
    partitionStrategyChoice_->value(0);
    partitionNameInput_->value("P1");
    partitionSizeInput_->value("120");
    initPartition();
    partitionManager_.allocate("P1", 120);
    partitionManager_.allocate("P2", 180);
    partitionManager_.allocate("P3", 80);
    const auto state = partitionManager_.snapshot("载入分区示例");
    updatePartitionView(state);
    updateSummary("已载入动态分区示例。\n可以继续申请或释放内存观察 FF/BF 的分配结果。\n");
    updateStatus("已载入分区示例数据。");
}

void MainWindow::generatePartitionSequence() {
    stopPartitionPlayback();

    int stepCount = 0;
    int intervalMs = 0;
    if (!parseInteger(partitionRandomStepsInput_, &stepCount) || stepCount <= 0) {
        fl_alert("随机步数必须是正整数。");
        return;
    }
    if (!parseInteger(partitionPlaybackIntervalInput_, &intervalMs) || intervalMs <= 0) {
        fl_alert("间隔毫秒数必须是正整数。");
        return;
    }
    if (stepCount > 30) {
        fl_alert("为便于展示，随机步数请不要超过 30。");
        return;
    }

    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> memoryDist(14, 36);
    std::uniform_int_distribution<int> sizeUnitsDist(2, 8);
    std::bernoulli_distribution allocBias(0.65);

    partitionPlaybackMemorySize_ = memoryDist(rng) * 32;
    partitionPlaybackIntervalMs_ = intervalMs;
    partitionSequence_.clear();
    partitionSequence_.reserve(static_cast<std::size_t>(stepCount));

    std::vector<std::string> aliveNames;
    int nextNameId = 1;
    for (int i = 0; i < stepCount; ++i) {
        const bool shouldAllocate = aliveNames.empty() || allocBias(rng);
        if (shouldAllocate) {
            PartitionPlaybackOperation operation;
            operation.allocate = true;
            operation.name = "P" + std::to_string(nextNameId++);
            operation.size = alignedSize(sizeUnitsDist(rng) * 16, 16);
            partitionSequence_.push_back(operation);
            aliveNames.push_back(operation.name);
        } else {
            std::uniform_int_distribution<int> releaseDist(0, static_cast<int>(aliveNames.size()) - 1);
            const int index = releaseDist(rng);
            PartitionPlaybackOperation operation;
            operation.allocate = false;
            operation.name = aliveNames[static_cast<std::size_t>(index)];
            partitionSequence_.push_back(operation);
            aliveNames.erase(aliveNames.begin() + index);
        }
    }

    totalMemoryInput_->value(std::to_string(partitionPlaybackMemorySize_).c_str());
    clearPartitionView();
    partitionInitialized_ = false;
    updateSummary(buildPartitionSequenceSummary());
    updateStatus("已随机生成内存大小和申请序列，可点击开始回放。");
}

void MainWindow::startPartitionPlayback() {
    stopPartitionPlayback();
    if (partitionSequence_.empty()) {
        fl_alert("请先生成随机申请序列。");
        return;
    }

    int intervalMs = 0;
    if (!parseInteger(partitionPlaybackIntervalInput_, &intervalMs) || intervalMs <= 0) {
        fl_alert("间隔毫秒数必须是正整数。");
        return;
    }
    partitionPlaybackIntervalMs_ = intervalMs;

    totalMemoryInput_->value(std::to_string(partitionPlaybackMemorySize_).c_str());
    partitionManager_.reset(partitionPlaybackMemorySize_, selectedPartitionStrategy());
    partitionInitialized_ = true;
    partitionPlaybackRunning_ = true;
    partitionPlaybackIndex_ = 0;
    updatePartitionView(partitionManager_.snapshot("回放初始化"));
    updateSummary(buildPartitionSequenceSummary(-1));
    updateStatus("开始按时间间隔回放随机申请序列。");
    handlePartitionPlaybackTick();
}

void MainWindow::stopPartitionPlayback() {
    if (partitionPlaybackRunning_) {
        partitionPlaybackRunning_ = false;
        Fl::remove_timeout(&MainWindow::onPartitionPlaybackTick, this);
        updateStatus("已停止随机序列回放。");
    }
}

void MainWindow::handlePartitionPlaybackTick() {
    if (!partitionPlaybackRunning_) {
        return;
    }
    if (partitionPlaybackIndex_ >= static_cast<int>(partitionSequence_.size())) {
        partitionPlaybackRunning_ = false;
        Fl::remove_timeout(&MainWindow::onPartitionPlaybackTick, this);
        updateStatus("随机申请序列回放完成。");
        return;
    }

    const auto& operation = partitionSequence_[static_cast<std::size_t>(partitionPlaybackIndex_)];
    const auto result = operation.allocate
        ? partitionManager_.allocate(operation.name, operation.size)
        : partitionManager_.release(operation.name);
    updatePartitionView(result.snapshot);

    std::ostringstream oss;
    oss << buildPartitionSequenceSummary(partitionPlaybackIndex_) << "\n"
        << "当前步骤结果:\n" << result.snapshot.action << '\n' << result.message << "\n\n"
        << "当前内存布局:\n"
        << memory_management::blocksToText(memory_management::buildMemoryLayout(result.snapshot, partitionManager_.totalSize()));
    updateSummary(oss.str());
    updateStatus("正在执行随机序列步骤 " + std::to_string(partitionPlaybackIndex_ + 1) + "/" + std::to_string(partitionSequence_.size()) + "。");

    ++partitionPlaybackIndex_;
    if (partitionPlaybackRunning_ && partitionPlaybackIndex_ < static_cast<int>(partitionSequence_.size())) {
        Fl::repeat_timeout(static_cast<double>(partitionPlaybackIntervalMs_) / 1000.0, &MainWindow::onPartitionPlaybackTick, this);
    } else {
        partitionPlaybackRunning_ = false;
        updateStatus("随机申请序列回放完成。");
    }
}

void MainWindow::runPaging() {
    int frameCount = 0;
    if (!parseInteger(frameCountInput_, &frameCount)) {
        fl_alert("页框数必须是整数。");
        return;
    }

    std::string errorMessage;
    const auto pages = memory_management::parsePageSequence(pageSequenceInput_->value(), &errorMessage);
    if (!errorMessage.empty()) {
        fl_alert("%s", errorMessage.c_str());
        return;
    }

    const std::string validationMessage = memory_management::validatePageInput(frameCount, pages);
    if (!validationMessage.empty()) {
        fl_alert("%s", validationMessage.c_str());
        return;
    }

    const auto result = memory_management::simulatePageReplacement(selectedPageAlgorithm(), frameCount, pages);
    updatePagingView(result);

    std::ostringstream oss;
    oss << "当前算法: " << result.algorithmName << '\n'
        << "页框数: " << frameCount << '\n'
        << "访问序列: " << pageSequenceInput_->value() << '\n'
        << "缺页次数: " << result.pageFaults << '\n'
        << std::fixed << std::setprecision(2)
        << "缺页率: " << result.pageFaultRate << "%\n\n"
        << "步骤摘要:\n";
    for (const auto& step : result.steps) {
        oss << "页 " << step.page << " -> " << memory_management::pageFramesToString(step.frames)
            << " : " << (step.hit ? "命中" : "缺页") << '\n';
    }
    updateSummary(oss.str());
    updateStatus("已完成页面置换模拟。");
}

void MainWindow::loadPagingDemo() {
    frameCountInput_->value("3");
    pageAlgorithmChoice_->value(0);
    pagingRandomLengthInput_->value("12");
    pageSequenceInput_->value("1 2 3 4 1 2 5 1 2 3 4 5");
    runPaging();
}

void MainWindow::generatePagingRandom() {
    int length = 0;
    if (!parseInteger(pagingRandomLengthInput_, &length) || length <= 0) {
        fl_alert("随机长度必须是正整数。");
        return;
    }
    if (length > 50) {
        fl_alert("为便于展示，随机长度请不要超过 50。");
        return;
    }

    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> frameDist(3, 6);
    const int frameCount = frameDist(rng);
    const int pageSpace = std::max(12, std::min(24, length + 6));
    const int workingSetSize = std::min(pageSpace, std::max(frameCount + 1, 4));
    std::uniform_int_distribution<int> workingSetStartDist(0, pageSpace - workingSetSize);
    std::uniform_int_distribution<int> workingSetOffsetDist(0, workingSetSize - 1);
    std::uniform_int_distribution<int> shiftDist(0, 2);
    std::bernoulli_distribution reuseRecentBias(0.50);
    std::bernoulli_distribution neighborBias(0.25);
    std::bernoulli_distribution switchWorkingSetBias(0.18);

    std::vector<int> pages;
    pages.reserve(static_cast<std::size_t>(length));
    std::vector<int> recentPages;
    recentPages.reserve(6);
    int workingSetStart = workingSetStartDist(rng);
    int current = workingSetStart + workingSetOffsetDist(rng);
    for (int i = 0; i < length; ++i) {
        if (i > 0 && switchWorkingSetBias(rng)) {
            workingSetStart = workingSetStartDist(rng);
        }

        if (!recentPages.empty() && reuseRecentBias(rng)) {
            std::uniform_int_distribution<int> recentDist(0, static_cast<int>(recentPages.size()) - 1);
            current = recentPages[static_cast<std::size_t>(recentDist(rng))];
        } else if (i > 0 && neighborBias(rng)) {
            current = std::clamp(current + shiftDist(rng) - 1, workingSetStart, workingSetStart + workingSetSize - 1);
        } else {
            current = workingSetStart + workingSetOffsetDist(rng);
        }
        pages.push_back(current);
        recentPages.push_back(current);
        if (recentPages.size() > 6) {
            recentPages.erase(recentPages.begin());
        }
    }

    frameCountInput_->value(std::to_string(frameCount).c_str());
    pageSequenceInput_->value(joinPages(pages).c_str());
    clearPagingView();

    std::ostringstream oss;
    oss << "已随机生成页面置换输入\n"
        << "页框数: " << frameCount << '\n'
        << "访问序列长度: " << length << '\n'
        << "页面编号范围: 0 ~ " << (pageSpace - 1) << '\n'
        << "生成模型: 工作集切换 + 最近访问复用 + 相邻页访问\n"
        << "访问序列: " << joinPages(pages) << '\n';
    updateSummary(oss.str());
    updateStatus("已随机生成页框数和访问序列，可直接运行模拟。");
}

void MainWindow::updatePartitionView(const memory_management::PartitionSnapshot& snapshot) {
    allocatedTable_->setBlocks(snapshot.allocatedBlocks);
    freeTable_->setBlocks(snapshot.freeBlocks);
    memoryLayoutWidget_->setBlocks(memory_management::buildMemoryLayout(snapshot, partitionManager_.totalSize()), partitionManager_.totalSize());
}

void MainWindow::clearPartitionView() {
    allocatedTable_->clearBlocks();
    freeTable_->clearBlocks();
    memoryLayoutWidget_->clearBlocks();
}

void MainWindow::updatePagingView(const memory_management::PageReplacementResult& result) {
    pageStepsTable_->setResult(result);
}

void MainWindow::clearPagingView() {
    pageStepsTable_->clearResult();
}

void MainWindow::updateSummary(const std::string& text) {
    if (summaryBuffer_ != nullptr) {
        summaryBuffer_->text(text.c_str());
    }
}

void MainWindow::updateStatus(const std::string& text) {
    statusBox_->copy_label(text.c_str());
    redraw();
}

std::string MainWindow::buildPartitionSequenceSummary(int highlightIndex) const {
    std::ostringstream oss;
    oss << "随机总内存: " << partitionPlaybackMemorySize_ << '\n'
        << "回放间隔: " << partitionPlaybackIntervalMs_ << " ms\n"
        << "随机申请序列:\n";
    for (std::size_t i = 0; i < partitionSequence_.size(); ++i) {
        const auto& operation = partitionSequence_[i];
        oss << (static_cast<int>(i) == highlightIndex ? "> " : "  ")
            << (i + 1) << ". ";
        if (operation.allocate) {
            oss << "alloc " << operation.name << ' ' << operation.size;
        } else {
            oss << "free " << operation.name;
        }
        oss << '\n';
    }
    return oss.str();
}

memory_management::PartitionStrategy MainWindow::selectedPartitionStrategy() const {
    return partitionStrategyChoice_->value() == 0 ? memory_management::PartitionStrategy::FirstFit : memory_management::PartitionStrategy::BestFit;
}

memory_management::PageAlgorithm MainWindow::selectedPageAlgorithm() const {
    switch (pageAlgorithmChoice_->value()) {
        case 0:
            return memory_management::PageAlgorithm::FIFO;
        case 1:
            return memory_management::PageAlgorithm::LRU;
        case 2:
            return memory_management::PageAlgorithm::LFU;
        case 3:
            return memory_management::PageAlgorithm::CLOCK;
        case 4:
            return memory_management::PageAlgorithm::Random;
        default:
            return memory_management::PageAlgorithm::FIFO;
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
