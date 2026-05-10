#pragma once

#include "../filesystem/VirtualFileSystem.h"

#include <FL/Fl_Double_Window.H>

#include <string>
#include <vector>

class Fl_Box;
class Fl_Button;
class Fl_Input;
class Fl_Int_Input;
class Fl_Text_Buffer;
class Fl_Text_Display;
class Fl_Text_Editor;
class Fl_Tree;
class Fl_Widget;
class Fl_Window;
class FileListTable;

class MainWindow : public Fl_Double_Window {
public:
    MainWindow(int width, int height, const char* title);

private:
    static void onFormatImage(Fl_Widget* widget, void* data);
    static void onLoadImage(Fl_Widget* widget, void* data);
    static void onRefresh(Fl_Widget* widget, void* data);
    static void onShowBitmap(Fl_Widget* widget, void* data);
    static void onUpDirectory(Fl_Widget* widget, void* data);
    static void onEnterSelection(Fl_Widget* widget, void* data);
    static void onCreateFile(Fl_Widget* widget, void* data);
    static void onCreateFolder(Fl_Widget* widget, void* data);
    static void onRenameEntry(Fl_Widget* widget, void* data);
    static void onDeleteEntry(Fl_Widget* widget, void* data);
    static void onSaveContent(Fl_Widget* widget, void* data);
    static void onTreeSelected(Fl_Widget* widget, void* data);
    static void onFileListSelected(Fl_Widget* widget, void* data);

    void buildUi();
    void loadImage();
    void formatImage();
    void refreshAll();
    void rebuildTree();
    void rebuildFileList();
    void updateCurrentPath();
    void updateSummary(const std::string& text);
    void updateStatus(const std::string& text);
    void clearSelection();
    void selectDirectory(int entryId);
    void selectEntry(int entryId);
    void enterSelection();
    void goToParent();
    void createFile();
    void createFolder();
    void renameEntry();
    void deleteEntry();
    void saveContent();
    void showBitmap();
    bool parseInteger(const Fl_Input* input, int* value) const;
    void appendDirectoryToTree(const simplefs::FsEntry& dirEntry);
    void ensureBitmapWindow();

    simplefs::VirtualFileSystem fileSystem_;
    int currentDirectoryId_ = simplefs::kRootId;
    int selectedEntryId_ = simplefs::kInvalidId;
    std::vector<simplefs::FsEntry> visibleEntries_;

    Fl_Input* imagePathInput_ = nullptr;
    Fl_Int_Input* totalBlocksInput_ = nullptr;
    Fl_Int_Input* blockSizeInput_ = nullptr;
    Fl_Tree* directoryTree_ = nullptr;
    FileListTable* fileListTable_ = nullptr;
    Fl_Input* nameInput_ = nullptr;
    Fl_Box* currentPathBox_ = nullptr;
    Fl_Box* selectedInfoBox_ = nullptr;
    Fl_Text_Editor* contentEditor_ = nullptr;
    Fl_Text_Buffer* contentBuffer_ = nullptr;
    Fl_Text_Display* summaryDisplay_ = nullptr;
    Fl_Text_Buffer* summaryBuffer_ = nullptr;
    Fl_Box* statusBox_ = nullptr;
    Fl_Window* bitmapWindow_ = nullptr;
    Fl_Text_Display* bitmapDisplay_ = nullptr;
    Fl_Text_Buffer* bitmapBuffer_ = nullptr;
};
