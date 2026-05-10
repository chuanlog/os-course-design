#include "MainWindow.h"

#include "../filesystem/FileSystemFacade.h"
#include "FileListTable.h"

#include <FL/Fl.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Int_Input.H>
#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Text_Editor.H>
#include <FL/Fl_Tree.H>
#include <FL/Fl_Tree_Item.H>
#include <FL/fl_ask.H>

#include <cstdint>
#include <sstream>
#include <string>

namespace {

void* encodeId(int id) {
    return reinterpret_cast<void*>(static_cast<std::intptr_t>(id));
}

int decodeId(void* value) {
    return static_cast<int>(reinterpret_cast<std::intptr_t>(value));
}

}  // namespace

MainWindow::MainWindow(int width, int height, const char* title)
    : Fl_Double_Window(width, height, title) {
    buildUi();
    refreshAll();
    end();
    resizable(this);
}

void MainWindow::onFormatImage(Fl_Widget*, void* data) {
    static_cast<MainWindow*>(data)->formatImage();
}

void MainWindow::onLoadImage(Fl_Widget*, void* data) {
    static_cast<MainWindow*>(data)->loadImage();
}

void MainWindow::onRefresh(Fl_Widget*, void* data) {
    static_cast<MainWindow*>(data)->refreshAll();
}

void MainWindow::onShowBitmap(Fl_Widget*, void* data) {
    static_cast<MainWindow*>(data)->showBitmap();
}

void MainWindow::onUpDirectory(Fl_Widget*, void* data) {
    static_cast<MainWindow*>(data)->goToParent();
}

void MainWindow::onEnterSelection(Fl_Widget*, void* data) {
    static_cast<MainWindow*>(data)->enterSelection();
}

void MainWindow::onCreateFile(Fl_Widget*, void* data) {
    static_cast<MainWindow*>(data)->createFile();
}

void MainWindow::onCreateFolder(Fl_Widget*, void* data) {
    static_cast<MainWindow*>(data)->createFolder();
}

void MainWindow::onRenameEntry(Fl_Widget*, void* data) {
    static_cast<MainWindow*>(data)->renameEntry();
}

void MainWindow::onDeleteEntry(Fl_Widget*, void* data) {
    static_cast<MainWindow*>(data)->deleteEntry();
}

void MainWindow::onSaveContent(Fl_Widget*, void* data) {
    static_cast<MainWindow*>(data)->saveContent();
}

void MainWindow::onTreeSelected(Fl_Widget*, void* data) {
    auto* self = static_cast<MainWindow*>(data);
    auto* item = self->directoryTree_->callback_item();
    if (item == nullptr) {
        return;
    }
    const int entryId = decodeId(item->user_data());
    self->selectDirectory(entryId);
}

void MainWindow::onFileListSelected(Fl_Widget*, void* data) {
    auto* self = static_cast<MainWindow*>(data);
    const int row = self->fileListTable_->callback_row();
    if (row < 0) {
        return;
    }
    self->selectEntry(self->fileListTable_->entryIdAtRow(row));
    if (Fl::event_clicks() > 0) {
        self->enterSelection();
    }
}

void MainWindow::buildUi() {
    begin();

    auto* titleBox = new Fl_Box(20, 15, 380, 30, "文件系统模拟器");
    titleBox->labelfont(FL_HELVETICA_BOLD);
    titleBox->labelsize(22);
    titleBox->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);

    imagePathInput_ = new Fl_Input(95, 52, 360, 28, "镜像文件:");
    imagePathInput_->value("virtual_disk.bin");
    totalBlocksInput_ = new Fl_Int_Input(560, 52, 70, 28, "总块数:");
    totalBlocksInput_->value("128");
    blockSizeInput_ = new Fl_Int_Input(730, 52, 70, 28, "块大小:");
    blockSizeInput_->value("256");

    auto* formatButton = new Fl_Button(830, 48, 120, 32, "格式化磁盘");
    auto* loadButton = new Fl_Button(960, 48, 110, 32, "加载镜像");
    auto* refreshButton = new Fl_Button(1080, 48, 90, 32, "刷新");
    auto* bitmapButton = new Fl_Button(1180, 48, 80, 32, "位图");
    formatButton->callback(&MainWindow::onFormatImage, this);
    loadButton->callback(&MainWindow::onLoadImage, this);
    refreshButton->callback(&MainWindow::onRefresh, this);
    bitmapButton->callback(&MainWindow::onShowBitmap, this);

    auto* leftHeader = new Fl_Box(20, 92, 220, 22, "目录树");
    leftHeader->labelfont(FL_HELVETICA_BOLD);
    leftHeader->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    directoryTree_ = new Fl_Tree(20, 118, 300, 560);
    directoryTree_->showroot(1);
    directoryTree_->callback(&MainWindow::onTreeSelected, this);

    auto* rightHeader = new Fl_Box(340, 92, 220, 22, "文件列表");
    rightHeader->labelfont(FL_HELVETICA_BOLD);
    rightHeader->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    currentPathBox_ = new Fl_Box(340, 118, 620, 24, "/");
    currentPathBox_->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);

    auto* upButton = new Fl_Button(970, 114, 90, 30, "上一级");
    auto* enterButton = new Fl_Button(1070, 114, 90, 30, "进入");
    upButton->callback(&MainWindow::onUpDirectory, this);
    enterButton->callback(&MainWindow::onEnterSelection, this);

    fileListTable_ = new FileListTable(340, 150, 920, 260);
    fileListTable_->callback(&MainWindow::onFileListSelected, this);
    fileListTable_->when(FL_WHEN_RELEASE);

    nameInput_ = new Fl_Input(400, 425, 220, 28, "名称:");
    auto* createFileButton = new Fl_Button(640, 422, 110, 32, "新建文件");
    auto* createFolderButton = new Fl_Button(760, 422, 120, 32, "新建文件夹");
    auto* renameButton = new Fl_Button(890, 422, 100, 32, "重命名");
    auto* deleteButton = new Fl_Button(1000, 422, 90, 32, "删除");
    auto* saveContentButton = new Fl_Button(1100, 422, 120, 32, "保存内容");
    createFileButton->callback(&MainWindow::onCreateFile, this);
    createFolderButton->callback(&MainWindow::onCreateFolder, this);
    renameButton->callback(&MainWindow::onRenameEntry, this);
    deleteButton->callback(&MainWindow::onDeleteEntry, this);
    saveContentButton->callback(&MainWindow::onSaveContent, this);

    selectedInfoBox_ = new Fl_Box(340, 462, 920, 22, "未选择对象");
    selectedInfoBox_->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);

    auto* contentHeader = new Fl_Box(340, 492, 220, 22, "文件内容编辑区");
    contentHeader->labelfont(FL_HELVETICA_BOLD);
    contentHeader->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    contentBuffer_ = new Fl_Text_Buffer();
    contentEditor_ = new Fl_Text_Editor(340, 518, 920, 160, "");
    contentEditor_->buffer(contentBuffer_);
    contentEditor_->textfont(FL_COURIER);
    contentEditor_->textsize(13);

    auto* summaryHeader = new Fl_Box(20, 690, 220, 22, "摘要信息");
    summaryHeader->labelfont(FL_HELVETICA_BOLD);
    summaryHeader->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    summaryBuffer_ = new Fl_Text_Buffer();
    summaryDisplay_ = new Fl_Text_Display(20, 715, 1240, 145, "");
    summaryDisplay_->buffer(summaryBuffer_);
    summaryDisplay_->textfont(FL_COURIER);
    summaryDisplay_->textsize(13);
    summaryDisplay_->wrap_mode(Fl_Text_Display::WRAP_AT_BOUNDS, 0);
    summaryDisplay_->scrollbar_width(14);

    statusBox_ = new Fl_Box(20, 872, 1240, 24, "就绪。可以先格式化或加载单个二进制镜像文件。");
    statusBox_->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    statusBox_->labelfont(FL_HELVETICA_BOLD);

    end();
}

bool MainWindow::parseInteger(const Fl_Input* input, int* value) const {
    if (input == nullptr || value == nullptr) {
        return false;
    }
    try {
        std::size_t parsed = 0;
        const std::string text = input->value();
        const int result = std::stoi(text, &parsed);
        if (parsed != text.size()) {
            return false;
        }
        *value = result;
        return true;
    } catch (...) {
        return false;
    }
}

void MainWindow::formatImage() {
    simplefs::FormatConfig config;
    if (!parseInteger(totalBlocksInput_, &config.totalBlocks) || !parseInteger(blockSizeInput_, &config.blockSize)) {
        fl_alert("总块数和块大小必须是整数。\n");
        return;
    }
    std::string errorMessage;
    if (!fileSystem_.format(imagePathInput_->value(), config, &errorMessage)) {
        fl_alert("%s", errorMessage.c_str());
        return;
    }
    currentDirectoryId_ = simplefs::kRootId;
    selectedEntryId_ = simplefs::kInvalidId;
    refreshAll();
    updateStatus("磁盘镜像已格式化，只会读写这一个二进制文件。");
}

void MainWindow::loadImage() {
    std::string errorMessage;
    if (!fileSystem_.load(imagePathInput_->value(), &errorMessage)) {
        fl_alert("%s", errorMessage.c_str());
        return;
    }
    currentDirectoryId_ = simplefs::kRootId;
    selectedEntryId_ = simplefs::kInvalidId;
    refreshAll();
    updateStatus("镜像加载成功。");
}

void MainWindow::refreshAll() {
    if (!fileSystem_.isLoaded()) {
        directoryTree_->clear();
        fileListTable_->clearEntries();
        visibleEntries_.clear();
        selectedEntryId_ = simplefs::kInvalidId;
        contentBuffer_->text("");
        currentPathBox_->copy_label("/");
        selectedInfoBox_->copy_label("未加载镜像文件");
        updateSummary("尚未加载镜像文件。\n请先格式化或加载一个单独的二进制镜像文件。\n");
        redraw();
        return;
    }
    rebuildTree();
    rebuildFileList();
    updateCurrentPath();
    updateSummary(simplefs::buildStatsText(fileSystem_.stats()));
}

void MainWindow::appendDirectoryToTree(const simplefs::FsEntry& dirEntry) {
    const std::string fullPath = fileSystem_.fullPath(dirEntry.id);
    std::string treePath = "磁盘";
    if (fullPath != "/") {
        treePath += fullPath;
    }
    auto* item = directoryTree_->add(treePath.c_str());
    if (item != nullptr) {
        item->user_data(encodeId(dirEntry.id));
    }
}

void MainWindow::rebuildTree() {
    directoryTree_->clear();
    auto* rootItem = directoryTree_->add("磁盘");
    if (rootItem != nullptr) {
        rootItem->user_data(encodeId(simplefs::kRootId));
    }
    for (const auto& dirEntry : fileSystem_.directories()) {
        if (dirEntry.id != simplefs::kRootId) {
            appendDirectoryToTree(dirEntry);
        }
    }
    directoryTree_->redraw();
}

void MainWindow::rebuildFileList() {
    visibleEntries_ = fileSystem_.listDirectory(currentDirectoryId_);
    fileListTable_->setEntries(visibleEntries_);
}

void MainWindow::updateCurrentPath() {
    currentPathBox_->copy_label(fileSystem_.fullPath(currentDirectoryId_).c_str());
}

void MainWindow::updateSummary(const std::string& text) {
    summaryBuffer_->text(text.c_str());
}

void MainWindow::updateStatus(const std::string& text) {
    statusBox_->copy_label(text.c_str());
    redraw();
}

void MainWindow::clearSelection() {
    selectedEntryId_ = simplefs::kInvalidId;
    nameInput_->value("");
    selectedInfoBox_->copy_label("未选择对象");
    contentBuffer_->text("");
    contentEditor_->deactivate();
    if (fileListTable_ != nullptr) {
        fileListTable_->select_all_rows(0);
        fileListTable_->redraw();
    }
}

void MainWindow::selectDirectory(int entryId) {
    const auto* entry = fileSystem_.getEntry(entryId);
    if (entry == nullptr || entry->type != simplefs::EntryType::Directory) {
        return;
    }
    currentDirectoryId_ = entryId;
    rebuildFileList();
    updateCurrentPath();
    clearSelection();
    updateStatus("已切换目录。");
}

void MainWindow::selectEntry(int entryId) {
    const auto* entry = fileSystem_.getEntry(entryId);
    if (entry == nullptr) {
        clearSelection();
        return;
    }
    selectedEntryId_ = entryId;
    if (fileListTable_ != nullptr) {
        fileListTable_->select_all_rows(0);
        for (int row = 0; row < static_cast<int>(visibleEntries_.size()); ++row) {
            if (visibleEntries_[static_cast<std::size_t>(row)].id == entryId) {
                fileListTable_->select_row(row, 1);
                break;
            }
        }
        fileListTable_->redraw();
    }
    nameInput_->value(entry->name.c_str());
    std::ostringstream info;
    info << "当前选择: " << entry->name
         << " | 类型: " << simplefs::entryTypeName(entry->type)
         << " | 路径: " << fileSystem_.fullPath(entry->id)
         << " | 大小: " << entry->size
         << " | 占用块: " << entry->blocks.size();
    selectedInfoBox_->copy_label(info.str().c_str());

    if (entry->type == simplefs::EntryType::File) {
        std::string errorMessage;
        const auto content = fileSystem_.readFileContent(entryId, &errorMessage);
        contentBuffer_->text(content.c_str());
        contentEditor_->activate();
    } else {
        contentBuffer_->text("");
        contentEditor_->deactivate();
    }
}

void MainWindow::enterSelection() {
    const auto* entry = fileSystem_.getEntry(selectedEntryId_);
    if (entry != nullptr && entry->type == simplefs::EntryType::Directory) {
        selectDirectory(entry->id);
    }
}

void MainWindow::goToParent() {
    if (!fileSystem_.isLoaded()) {
        return;
    }
    const auto* entry = fileSystem_.getEntry(currentDirectoryId_);
    if (entry != nullptr && entry->parentId != simplefs::kInvalidId) {
        selectDirectory(entry->parentId);
    }
}

void MainWindow::createFile() {
    if (!fileSystem_.isLoaded()) {
        fl_alert("请先加载或格式化镜像。\n");
        return;
    }
    std::string errorMessage;
    if (!fileSystem_.createFile(currentDirectoryId_, nameInput_->value(), &errorMessage)) {
        fl_alert("%s", errorMessage.c_str());
        return;
    }
    refreshAll();
    updateStatus("文件创建成功。");
}

void MainWindow::createFolder() {
    if (!fileSystem_.isLoaded()) {
        fl_alert("请先加载或格式化镜像。\n");
        return;
    }
    std::string errorMessage;
    if (!fileSystem_.createDirectory(currentDirectoryId_, nameInput_->value(), &errorMessage)) {
        fl_alert("%s", errorMessage.c_str());
        return;
    }
    refreshAll();
    updateStatus("文件夹创建成功。");
}

void MainWindow::renameEntry() {
    if (selectedEntryId_ == simplefs::kInvalidId) {
        fl_alert("请先选择要重命名的对象。\n");
        return;
    }
    std::string errorMessage;
    if (!fileSystem_.renameEntry(selectedEntryId_, nameInput_->value(), &errorMessage)) {
        fl_alert("%s", errorMessage.c_str());
        return;
    }
    refreshAll();
    updateStatus("重命名成功。");
}

void MainWindow::deleteEntry() {
    if (selectedEntryId_ == simplefs::kInvalidId) {
        fl_alert("请先选择要删除的对象。\n");
        return;
    }
    std::string errorMessage;
    if (!fileSystem_.deleteEntryRecursive(selectedEntryId_, &errorMessage)) {
        fl_alert("%s", errorMessage.c_str());
        return;
    }
    clearSelection();
    refreshAll();
    updateStatus("删除成功。目录会递归删除其所有子项。");
}

void MainWindow::saveContent() {
    if (selectedEntryId_ == simplefs::kInvalidId) {
        fl_alert("请先选择一个文件。\n");
        return;
    }
    const auto* entry = fileSystem_.getEntry(selectedEntryId_);
    if (entry == nullptr || entry->type != simplefs::EntryType::File) {
        fl_alert("当前选择不是文件。\n");
        return;
    }
    std::string errorMessage;
    if (!fileSystem_.writeFileContent(selectedEntryId_, contentBuffer_->text(), &errorMessage)) {
        fl_alert("%s", errorMessage.c_str());
        return;
    }
    refreshAll();
    selectEntry(selectedEntryId_);
    updateStatus("文件内容已写回单个二进制镜像文件。");
}

void MainWindow::showBitmap() {
    if (!fileSystem_.isLoaded()) {
        fl_alert("请先加载或格式化镜像。\n");
        return;
    }
    ensureBitmapWindow();
    const std::string text = simplefs::buildStatsText(fileSystem_.stats()) + "\n" +
                             simplefs::buildBitmapText(fileSystem_.bitmapString());
    bitmapBuffer_->text(text.c_str());
    updateSummary(text);
    bitmapWindow_->show();
    updateStatus("已显示位示图信息。");
}

void MainWindow::ensureBitmapWindow() {
    if (bitmapWindow_ != nullptr) {
        return;
    }
    bitmapWindow_ = new Fl_Double_Window(760, 520, "位示图查看器");
    bitmapBuffer_ = new Fl_Text_Buffer();
    auto* titleBox = new Fl_Box(16, 12, 200, 24, "位示图与空间统计");
    titleBox->labelfont(FL_HELVETICA_BOLD);
    titleBox->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    bitmapDisplay_ = new Fl_Text_Display(16, 42, 728, 462);
    bitmapDisplay_->buffer(bitmapBuffer_);
    bitmapDisplay_->textfont(FL_COURIER);
    bitmapDisplay_->textsize(13);
    bitmapDisplay_->wrap_mode(Fl_Text_Display::WRAP_AT_BOUNDS, 0);
    bitmapDisplay_->scrollbar_width(14);
    bitmapWindow_->resizable(bitmapDisplay_);
    bitmapWindow_->end();
}
