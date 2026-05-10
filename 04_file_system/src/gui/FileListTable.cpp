#include "FileListTable.h"

#include "../filesystem/FsTypes.h"

#include <FL/fl_draw.H>

#include <string>

namespace {

void drawCellBackground(int x, int y, int w, int h, Fl_Color color) {
    fl_push_clip(x, y, w, h);
    fl_color(color);
    fl_rectf(x, y, w, h);
    fl_color(FL_LIGHT2);
    fl_rect(x, y, w, h);
    fl_pop_clip();
}

void drawCellText(const std::string& text, int x, int y, int w, int h, Fl_Align align) {
    fl_push_clip(x, y, w, h);
    fl_color(FL_BLACK);
    fl_draw(text.c_str(), x + 6, y, w - 12, h, align);
    fl_pop_clip();
}

}  // namespace

FileListTable::FileListTable(int x, int y, int w, int h, const char* label)
    : Fl_Table_Row(x, y, w, h, label), headers_{"名称", "类型", "大小", "块数", "更新时间"} {
    cols(static_cast<int>(headers_.size()));
    rows(0);
    col_header(1);
    row_header(0);
    col_header_height(30);
    row_height_all(28);
    col_width(0, 260);
    col_width(1, 90);
    col_width(2, 90);
    col_width(3, 90);
    col_width(4, 290);
    col_resize(1);
    row_resize(0);
    type(SELECT_SINGLE);
    end();
}

void FileListTable::setEntries(const std::vector<simplefs::FsEntry>& entries) {
    rows_ = entries;
    rows(static_cast<int>(rows_.size()));
    redraw();
}

void FileListTable::clearEntries() {
    rows_.clear();
    rows(0);
    redraw();
}

int FileListTable::entryIdAtRow(int row) const {
    if (row < 0 || row >= static_cast<int>(rows_.size())) {
        return simplefs::kInvalidId;
    }
    return rows_[static_cast<std::size_t>(row)].id;
}

void FileListTable::draw_cell(TableContext context, int row, int col, int x, int y, int w, int h) {
    switch (context) {
        case CONTEXT_STARTPAGE:
            fl_font(FL_HELVETICA, 13);
            return;
        case CONTEXT_COL_HEADER:
            drawCellBackground(x, y, w, h, fl_rgb_color(230, 235, 245));
            drawCellText(headers_[static_cast<std::size_t>(col)], x, y, w, h, FL_ALIGN_CENTER);
            return;
        case CONTEXT_CELL: {
            const Fl_Color bg = row_selected(row) ? fl_rgb_color(220, 235, 255) : FL_WHITE;
            drawCellBackground(x, y, w, h, bg);
            const Fl_Align align = (col == 0 || col == 4) ? FL_ALIGN_LEFT : FL_ALIGN_CENTER;
            drawCellText(cellValue(row, col), x, y, w, h, align);
            return;
        }
        default:
            return;
    }
}

std::string FileListTable::cellValue(int row, int col) const {
    if (row < 0 || row >= static_cast<int>(rows_.size())) {
        return {};
    }
    const auto& entry = rows_[static_cast<std::size_t>(row)];
    switch (col) {
        case 0:
            return entry.name;
        case 1:
            return simplefs::entryTypeName(entry.type);
        case 2:
            return std::to_string(entry.size);
        case 3:
            return std::to_string(entry.blocks.size());
        case 4:
            return simplefs::formatTimestamp(entry.updatedAt);
        default:
            return {};
    }
}
