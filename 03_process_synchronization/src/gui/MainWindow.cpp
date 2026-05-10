#include "MainWindow.h"

#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Int_Input.H>
#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_Text_Display.H>
#include <FL/fl_ask.H>

#include <string>

MainWindow::MainWindow(int width, int height, const char* title)
    : Fl_Double_Window(width, height, title) {
    buildUi();
    loadDefaults();
    updateScenarioVisibility();
    end();
    resizable(this);
}

void MainWindow::onScenarioChanged(Fl_Widget*, void* data) {
    static_cast<MainWindow*>(data)->updateScenarioVisibility();
}

void MainWindow::onLoadDefaults(Fl_Widget*, void* data) {
    static_cast<MainWindow*>(data)->loadDefaults();
}

void MainWindow::onRunSimulation(Fl_Widget*, void* data) {
    static_cast<MainWindow*>(data)->runSimulation();
}

void MainWindow::buildUi() {
    begin();

    auto* titleBox = new Fl_Box(20, 15, 460, 30, "线程同步与互斥模拟器");
    titleBox->labelfont(FL_HELVETICA_BOLD);
    titleBox->labelsize(22);
    titleBox->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);

    auto* scenarioLabel = new Fl_Box(20, 52, 90, 24, "场景:");
    scenarioLabel->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);

    scenarioChoice_ = new Fl_Choice(90, 48, 260, 28);
    scenarioChoice_->add("生产者 - 消费者");
    scenarioChoice_->add("读者 - 写者");
    scenarioChoice_->add("哲学家进餐");
    scenarioChoice_->value(0);
    scenarioChoice_->callback(&MainWindow::onScenarioChanged, this);

    loadDefaultsButton_ = new Fl_Button(380, 46, 140, 32, "载入默认参数");
    loadDefaultsButton_->callback(&MainWindow::onLoadDefaults, this);
    runButton_ = new Fl_Button(540, 46, 140, 32, "运行模拟");
    runButton_->callback(&MainWindow::onRunSimulation, this);

    auto* noteBox = new Fl_Box(720, 45, 540, 34,
                               "说明：为避免 GUI 与并发线程竞争，程序先完成多线程模拟，再统一收集并更新结果。"
    );
    noteBox->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE | FL_ALIGN_WRAP);

    auto* configHeader = new Fl_Box(20, 92, 220, 22, "运行配置");
    configHeader->labelfont(FL_HELVETICA_BOLD);
    configHeader->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);

    producerConsumerGroup_ = new Fl_Group(20, 120, 1240, 130, "生产者 - 消费者配置");
    producerConsumerGroup_->box(FL_ENGRAVED_BOX);
    producerConsumerGroup_->labelfont(FL_HELVETICA_BOLD);
    producerConsumerGroup_->begin();
    bufferCapacityInput_ = new Fl_Int_Input(130, 150, 80, 28, "缓冲区容量:");
    producerCountInput_ = new Fl_Int_Input(360, 150, 80, 28, "生产者数量:");
    consumerCountInput_ = new Fl_Int_Input(600, 150, 80, 28, "消费者数量:");
    itemsPerProducerInput_ = new Fl_Int_Input(880, 150, 80, 28, "每个生产者数量:");
    producerDelayInput_ = new Fl_Int_Input(230, 195, 80, 28, "生产延迟ms:");
    consumerDelayInput_ = new Fl_Int_Input(500, 195, 80, 28, "消费延迟ms:");
    producerConsumerGroup_->end();

    readersWritersGroup_ = new Fl_Group(20, 120, 1240, 130, "读者 - 写者配置");
    readersWritersGroup_->box(FL_ENGRAVED_BOX);
    readersWritersGroup_->labelfont(FL_HELVETICA_BOLD);
    readersWritersGroup_->begin();
    readerCountInput_ = new Fl_Int_Input(130, 150, 80, 28, "读者数量:");
    writerCountInput_ = new Fl_Int_Input(360, 150, 80, 28, "写者数量:");
    readerRoundsInput_ = new Fl_Int_Input(600, 150, 80, 28, "读者轮数:");
    writerRoundsInput_ = new Fl_Int_Input(840, 150, 80, 28, "写者轮数:");
    readerDelayInput_ = new Fl_Int_Input(240, 195, 80, 28, "读者延迟ms:");
    writerDelayInput_ = new Fl_Int_Input(520, 195, 80, 28, "写者延迟ms:");
    readersWritersGroup_->end();

    diningGroup_ = new Fl_Group(20, 120, 1240, 130, "哲学家进餐配置");
    diningGroup_->box(FL_ENGRAVED_BOX);
    diningGroup_->labelfont(FL_HELVETICA_BOLD);
    diningGroup_->begin();
    philosopherCountInput_ = new Fl_Int_Input(150, 150, 80, 28, "哲学家数量:");
    philosopherRoundsInput_ = new Fl_Int_Input(400, 150, 80, 28, "每人轮数:");
    thinkDelayInput_ = new Fl_Int_Input(680, 150, 80, 28, "思考延迟ms:");
    eatDelayInput_ = new Fl_Int_Input(930, 150, 80, 28, "进餐延迟ms:");
    diningGroup_->end();

    auto* eventsHeader = new Fl_Box(20, 270, 220, 22, "并发事件日志");
    eventsHeader->labelfont(FL_HELVETICA_BOLD);
    eventsHeader->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    eventTable_ = new EventTable(20, 295, 1240, 390);

    auto* summaryHeader = new Fl_Box(20, 700, 220, 22, "结果摘要");
    summaryHeader->labelfont(FL_HELVETICA_BOLD);
    summaryHeader->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    summaryBuffer_ = new Fl_Text_Buffer();
    summaryDisplay_ = new Fl_Text_Display(20, 725, 1240, 150, "");
    summaryDisplay_->buffer(summaryBuffer_);
    summaryDisplay_->textfont(FL_COURIER);
    summaryDisplay_->textsize(13);
    summaryDisplay_->wrap_mode(Fl_Text_Display::WRAP_AT_BOUNDS, 0);

    statusBox_ = new Fl_Box(20, 885, 1240, 20, "就绪。请选择同步场景并运行模拟。");
    statusBox_->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    statusBox_->labelfont(FL_HELVETICA_BOLD);

    end();
}

void MainWindow::updateScenarioVisibility() {
    const auto scenario = selectedScenario();
    producerConsumerGroup_->hide();
    readersWritersGroup_->hide();
    diningGroup_->hide();

    if (scenario == sync_demo::Scenario::ProducerConsumer) {
        producerConsumerGroup_->show();
    } else if (scenario == sync_demo::Scenario::ReadersWriters) {
        readersWritersGroup_->show();
    } else {
        diningGroup_->show();
    }
    clearResults();
    updateStatus("已切换场景，准备运行模拟。");
    redraw();
}

void MainWindow::loadDefaults() {
    const auto producerConsumer = sync_demo::defaultProducerConsumerConfig();
    bufferCapacityInput_->value(std::to_string(producerConsumer.bufferCapacity).c_str());
    producerCountInput_->value(std::to_string(producerConsumer.producerCount).c_str());
    consumerCountInput_->value(std::to_string(producerConsumer.consumerCount).c_str());
    itemsPerProducerInput_->value(std::to_string(producerConsumer.itemsPerProducer).c_str());
    producerDelayInput_->value(std::to_string(producerConsumer.producerDelayMs).c_str());
    consumerDelayInput_->value(std::to_string(producerConsumer.consumerDelayMs).c_str());

    const auto readersWriters = sync_demo::defaultReadersWritersConfig();
    readerCountInput_->value(std::to_string(readersWriters.readerCount).c_str());
    writerCountInput_->value(std::to_string(readersWriters.writerCount).c_str());
    readerRoundsInput_->value(std::to_string(readersWriters.readerRounds).c_str());
    writerRoundsInput_->value(std::to_string(readersWriters.writerRounds).c_str());
    readerDelayInput_->value(std::to_string(readersWriters.readerDelayMs).c_str());
    writerDelayInput_->value(std::to_string(readersWriters.writerDelayMs).c_str());

    const auto dining = sync_demo::defaultDiningPhilosophersConfig();
    philosopherCountInput_->value(std::to_string(dining.philosopherCount).c_str());
    philosopherRoundsInput_->value(std::to_string(dining.roundsPerPhilosopher).c_str());
    thinkDelayInput_->value(std::to_string(dining.thinkDelayMs).c_str());
    eatDelayInput_->value(std::to_string(dining.eatDelayMs).c_str());

    clearResults();
    updateStatus("已载入默认参数。");
}

void MainWindow::runSimulation() {
    std::string errorMessage;
    const auto producerConsumerConfig = readProducerConsumerConfig(&errorMessage);
    if (!errorMessage.empty()) {
        fl_alert("%s", errorMessage.c_str());
        return;
    }
    const auto readersWritersConfig = readReadersWritersConfig(&errorMessage);
    if (!errorMessage.empty()) {
        fl_alert("%s", errorMessage.c_str());
        return;
    }
    const auto diningConfig = readDiningConfig(&errorMessage);
    if (!errorMessage.empty()) {
        fl_alert("%s", errorMessage.c_str());
        return;
    }

    updateStatus("正在执行多线程模拟，请稍候...");
    Fl::check();

    const auto result = sync_demo::runScenario(selectedScenario(), producerConsumerConfig, readersWritersConfig, diningConfig, &errorMessage);
    if (!errorMessage.empty()) {
        fl_alert("%s", errorMessage.c_str());
        updateStatus("模拟启动失败。请检查输入参数。");
        return;
    }

    eventTable_->setEvents(result.events);
    updateSummary(sync_demo::buildSummaryText(result));
    updateStatus("模拟已完成，结果已统一更新到界面。");
}

void MainWindow::clearResults() {
    if (eventTable_ != nullptr) {
        eventTable_->clearEvents();
    }
    if (summaryBuffer_ != nullptr) {
        summaryBuffer_->text("");
    }
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

sync_demo::Scenario MainWindow::selectedScenario() const {
    switch (scenarioChoice_->value()) {
        case 0:
            return sync_demo::Scenario::ProducerConsumer;
        case 1:
            return sync_demo::Scenario::ReadersWriters;
        case 2:
            return sync_demo::Scenario::DiningPhilosophers;
        default:
            return sync_demo::Scenario::ProducerConsumer;
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

sync_demo::ProducerConsumerConfig MainWindow::readProducerConsumerConfig(std::string* errorMessage) const {
    sync_demo::ProducerConsumerConfig config;
    if (!parseInteger(bufferCapacityInput_, &config.bufferCapacity) ||
        !parseInteger(producerCountInput_, &config.producerCount) ||
        !parseInteger(consumerCountInput_, &config.consumerCount) ||
        !parseInteger(itemsPerProducerInput_, &config.itemsPerProducer) ||
        !parseInteger(producerDelayInput_, &config.producerDelayMs) ||
        !parseInteger(consumerDelayInput_, &config.consumerDelayMs)) {
        if (errorMessage != nullptr) {
            *errorMessage = "生产者 - 消费者配置中存在非法整数输入。";
        }
    } else if (errorMessage != nullptr) {
        errorMessage->clear();
    }
    return config;
}

sync_demo::ReadersWritersConfig MainWindow::readReadersWritersConfig(std::string* errorMessage) const {
    sync_demo::ReadersWritersConfig config;
    if (!parseInteger(readerCountInput_, &config.readerCount) ||
        !parseInteger(writerCountInput_, &config.writerCount) ||
        !parseInteger(readerRoundsInput_, &config.readerRounds) ||
        !parseInteger(writerRoundsInput_, &config.writerRounds) ||
        !parseInteger(readerDelayInput_, &config.readerDelayMs) ||
        !parseInteger(writerDelayInput_, &config.writerDelayMs)) {
        if (errorMessage != nullptr) {
            *errorMessage = "读者 - 写者配置中存在非法整数输入。";
        }
    } else if (errorMessage != nullptr) {
        errorMessage->clear();
    }
    return config;
}

sync_demo::DiningPhilosophersConfig MainWindow::readDiningConfig(std::string* errorMessage) const {
    sync_demo::DiningPhilosophersConfig config;
    if (!parseInteger(philosopherCountInput_, &config.philosopherCount) ||
        !parseInteger(philosopherRoundsInput_, &config.roundsPerPhilosopher) ||
        !parseInteger(thinkDelayInput_, &config.thinkDelayMs) ||
        !parseInteger(eatDelayInput_, &config.eatDelayMs)) {
        if (errorMessage != nullptr) {
            *errorMessage = "哲学家进餐配置中存在非法整数输入。";
        }
    } else if (errorMessage != nullptr) {
        errorMessage->clear();
    }
    return config;
}
