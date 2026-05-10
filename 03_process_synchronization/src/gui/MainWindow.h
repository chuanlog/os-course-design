#pragma once

#include "../sync/SimulationFacade.h"
#include "EventTable.h"

#include <FL/Fl_Double_Window.H>

class Fl_Box;
class Fl_Button;
class Fl_Choice;
class Fl_Group;
class Fl_Input;
class Fl_Int_Input;
class Fl_Text_Buffer;
class Fl_Text_Display;
class Fl_Widget;

class MainWindow : public Fl_Double_Window {
public:
    MainWindow(int width, int height, const char* title);

private:
    static void onScenarioChanged(Fl_Widget* widget, void* data);
    static void onLoadDefaults(Fl_Widget* widget, void* data);
    static void onRunSimulation(Fl_Widget* widget, void* data);

    void buildUi();
    void updateScenarioVisibility();
    void loadDefaults();
    void runSimulation();
    void clearResults();
    void updateSummary(const std::string& text);
    void updateStatus(const std::string& text);
    sync_demo::Scenario selectedScenario() const;
    bool parseInteger(const Fl_Input* input, int* value) const;
    sync_demo::ProducerConsumerConfig readProducerConsumerConfig(std::string* errorMessage) const;
    sync_demo::ReadersWritersConfig readReadersWritersConfig(std::string* errorMessage) const;
    sync_demo::DiningPhilosophersConfig readDiningConfig(std::string* errorMessage) const;

    Fl_Choice* scenarioChoice_ = nullptr;
    Fl_Button* loadDefaultsButton_ = nullptr;
    Fl_Button* runButton_ = nullptr;

    Fl_Group* producerConsumerGroup_ = nullptr;
    Fl_Int_Input* bufferCapacityInput_ = nullptr;
    Fl_Int_Input* producerCountInput_ = nullptr;
    Fl_Int_Input* consumerCountInput_ = nullptr;
    Fl_Int_Input* itemsPerProducerInput_ = nullptr;
    Fl_Int_Input* producerDelayInput_ = nullptr;
    Fl_Int_Input* consumerDelayInput_ = nullptr;

    Fl_Group* readersWritersGroup_ = nullptr;
    Fl_Int_Input* readerCountInput_ = nullptr;
    Fl_Int_Input* writerCountInput_ = nullptr;
    Fl_Int_Input* readerRoundsInput_ = nullptr;
    Fl_Int_Input* writerRoundsInput_ = nullptr;
    Fl_Int_Input* readerDelayInput_ = nullptr;
    Fl_Int_Input* writerDelayInput_ = nullptr;

    Fl_Group* diningGroup_ = nullptr;
    Fl_Int_Input* philosopherCountInput_ = nullptr;
    Fl_Int_Input* philosopherRoundsInput_ = nullptr;
    Fl_Int_Input* thinkDelayInput_ = nullptr;
    Fl_Int_Input* eatDelayInput_ = nullptr;

    EventTable* eventTable_ = nullptr;
    Fl_Text_Display* summaryDisplay_ = nullptr;
    Fl_Text_Buffer* summaryBuffer_ = nullptr;
    Fl_Box* statusBox_ = nullptr;
};
