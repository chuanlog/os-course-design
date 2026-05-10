#pragma once

#include "../filesystem/FsTypes.h"

#include <FL/Fl_Table_Row.H>

#include <string>
#include <vector>

class FileListTable : public Fl_Table_Row {
public:
    FileListTable(int x, int y, int w, int h, const char* label = nullptr);

    void setEntries(const std::vector<simplefs::FsEntry>& entries);
    void clearEntries();
    int entryIdAtRow(int row) const;

protected:
    void draw_cell(TableContext context, int row, int col, int x, int y, int w, int h) override;

private:
    std::string cellValue(int row, int col) const;

    std::vector<std::string> headers_;
    std::vector<simplefs::FsEntry> rows_;
};
