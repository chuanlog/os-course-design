#pragma once

#include "../memory/PartitionManager.h"
#include "MemoryLayoutWidget.h"
#include "BlocksTable.h"
#include "PageStepsTable.h"

#include <FL/Fl_Double_Window.H>

#include <string>
#include <vector>

class Fl_Box;
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
    static void onModeChanged(Fl_Widget* widget, void* data);
    static void onInitPartition(Fl_Widget* widget, void* data);
    static void onAllocatePartition(Fl_Widget* widget, void* data);
    static void onReleasePartition(Fl_Widget* widget, void* data);
    static void onLoadPartitionDemo(Fl_Widget* widget, void* data);
    static void onGeneratePartitionSequence(Fl_Widget* widget, void* data);
    static void onStartPartitionPlayback(Fl_Widget* widget, void* data);
    static void onStopPartitionPlayback(Fl_Widget* widget, void* data);
    static void onRunPaging(Fl_Widget* widget, void* data);
    static void onLoadPagingDemo(Fl_Widget* widget, void* data);
    static void onGeneratePagingRandom(Fl_Widget* widget, void* data);
    static void onPartitionPlaybackTick(void* data);

    void buildUi();
    void updateModeVisibility();
    void initPartition();
    void allocatePartition();
    void releasePartition();
    void loadPartitionDemo();
    void generatePartitionSequence();
    void startPartitionPlayback();
    void stopPartitionPlayback();
    void handlePartitionPlaybackTick();
    void runPaging();
    void loadPagingDemo();
    void generatePagingRandom();
    void updatePartitionView(const memory_management::PartitionSnapshot& snapshot);
    void clearPartitionView();
    void updatePagingView(const memory_management::PageReplacementResult& result);
    void clearPagingView();
    void updateSummary(const std::string& text);
    void updateStatus(const std::string& text);
    std::string buildPartitionSequenceSummary(int highlightIndex = -1) const;
    memory_management::PartitionStrategy selectedPartitionStrategy() const;
    memory_management::PageAlgorithm selectedPageAlgorithm() const;
    bool parseInteger(const Fl_Input* input, int* value) const;

    struct PartitionPlaybackOperation {
        bool allocate = true;
        std::string name;
        int size = 0;
    };

    memory_management::PartitionManager partitionManager_;
    bool partitionInitialized_ = false;
    bool partitionPlaybackRunning_ = false;
    int partitionPlaybackIndex_ = 0;
    int partitionPlaybackIntervalMs_ = 600;
    int partitionPlaybackMemorySize_ = 640;
    std::vector<PartitionPlaybackOperation> partitionSequence_;

    Fl_Choice* modeChoice_ = nullptr;

    Fl_Group* partitionGroup_ = nullptr;
    Fl_Int_Input* totalMemoryInput_ = nullptr;
    Fl_Choice* partitionStrategyChoice_ = nullptr;
    Fl_Input* partitionNameInput_ = nullptr;
    Fl_Int_Input* partitionSizeInput_ = nullptr;
    Fl_Int_Input* partitionRandomStepsInput_ = nullptr;
    Fl_Int_Input* partitionPlaybackIntervalInput_ = nullptr;
    MemoryLayoutWidget* memoryLayoutWidget_ = nullptr;
    BlocksTable* allocatedTable_ = nullptr;
    BlocksTable* freeTable_ = nullptr;

    Fl_Group* pagingGroup_ = nullptr;
    Fl_Int_Input* frameCountInput_ = nullptr;
    Fl_Choice* pageAlgorithmChoice_ = nullptr;
    Fl_Input* pageSequenceInput_ = nullptr;
    Fl_Int_Input* pagingRandomLengthInput_ = nullptr;
    PageStepsTable* pageStepsTable_ = nullptr;

    Fl_Text_Display* summaryDisplay_ = nullptr;
    Fl_Text_Buffer* summaryBuffer_ = nullptr;
    Fl_Box* statusBox_ = nullptr;
};
