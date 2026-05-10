#pragma once

#include "../scheduling/MLFQScheduler.h"
#include "../scheduling/MLQScheduler.h"
#include "../scheduling/SchedulerFacade.h"
#include "GanttChartWidget.h"
#include "ResultsTable.h"

#include <FL/Fl_Double_Window.H>

#include <vector>

class Fl_Box;
class Fl_Browser;
class Fl_Choice;
class Fl_Group;
class Fl_Input;
class Fl_Int_Input;
class Fl_Output;
class Fl_Text_Buffer;
class Fl_Text_Display;
class Fl_Widget;

class MainWindow : public Fl_Double_Window {
public:
    MainWindow(int width, int height, const char* title);

private:
    static void onAddProcess(Fl_Widget* widget, void* data);
    static void onRemoveSelected(Fl_Widget* widget, void* data);
    static void onClearProcesses(Fl_Widget* widget, void* data);
    static void onLoadDemo(Fl_Widget* widget, void* data);
    static void onGenerateRandom(Fl_Widget* widget, void* data);
    static void onRunSimulation(Fl_Widget* widget, void* data);
    static void onAlgorithmChanged(Fl_Widget* widget, void* data);
    static void onProcessSelected(Fl_Widget* widget, void* data);

    void buildUi();
    void loadDemoData();
    void applyDefaultMLQConfig();
    void applyDefaultMLFQConfig();
    void clearProcessInputs();
    void clearSelectedProcessInfo();
    void updateProcessBrowser();
    void updateSelectedProcessInfo();
    void updateDetailTable(const scheduling::SimulationResult& result);
    void clearDetailTable();
    void updateSummaryOutput(
        const scheduling::SimulationResult& result,
        const scheduling::MLQConfig& mlqConfig,
        const scheduling::MLFQConfig& mlfqConfig);
    void updateStatus(const std::string& message);
    void updateConfigPanelVisibility();
    void addProcess();
    void generateRandomProcesses();
    void removeSelectedProcess();
    void clearProcesses();
    void runSimulation();
    scheduling::Algorithm selectedAlgorithm() const;
    scheduling::MLQConfig currentMLQConfig() const;
    scheduling::MLFQConfig currentMLFQConfig() const;
    bool parseInteger(const Fl_Input* input, int* value) const;

    std::vector<scheduling::ProcessInput> processes_;

    Fl_Input* nameInput_ = nullptr;
    Fl_Int_Input* arrivalInput_ = nullptr;
    Fl_Int_Input* burstInput_ = nullptr;
    Fl_Int_Input* priorityInput_ = nullptr;
    Fl_Int_Input* randomCountInput_ = nullptr;
    Fl_Choice* algorithmChoice_ = nullptr;
    Fl_Group* rrConfigGroup_ = nullptr;
    Fl_Group* mlqConfigGroup_ = nullptr;
    Fl_Group* mlfqConfigGroup_ = nullptr;
    Fl_Int_Input* quantumInput_ = nullptr;
    Fl_Int_Input* mlqPriorityInputs_[3] = {nullptr, nullptr, nullptr};
    Fl_Int_Input* mlqQuantumInputs_[3] = {nullptr, nullptr, nullptr};
    Fl_Choice* mlfqAlgorithmChoices_[3] = {nullptr, nullptr, nullptr};
    Fl_Int_Input* mlfqQuantumInputs_[3] = {nullptr, nullptr, nullptr};
    Fl_Browser* processBrowser_ = nullptr;
    Fl_Output* selectedNameOutput_ = nullptr;
    Fl_Output* selectedArrivalOutput_ = nullptr;
    Fl_Output* selectedBurstOutput_ = nullptr;
    Fl_Output* selectedPriorityOutput_ = nullptr;
    ResultsTable* detailTable_ = nullptr;
    Fl_Text_Display* summaryOutput_ = nullptr;
    Fl_Text_Buffer* summaryBuffer_ = nullptr;
    GanttChartWidget* ganttChart_ = nullptr;
    Fl_Box* statusBox_ = nullptr;
};
