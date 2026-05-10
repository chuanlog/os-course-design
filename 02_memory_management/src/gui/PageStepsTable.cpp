#include "PageStepsTable.h"

#include "../memory/MemoryFacade.h"

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

PageStepsTable::PageStepsTable(int x, int y, int w, int h, const char* label)
    : Fl_Table_Row(x, y, w, h, label), headers_{"访问页", "页框状态", "结果"} {
    cols(static_cast<int>(headers_.size()));
    rows(0);
    col_header(1);
    row_header(1);
    col_resize(1);
    row_resize(0);
    row_header_width(55);
    row_height_all(28);
    col_header_height(30);
    col_width(0, 90);
    col_width(1, 220);
    col_width(2, 90);
    end();
}

void PageStepsTable::setResult(const memory_management::PageReplacementResult& result) {
    rows_ = result.steps;
    rows(static_cast<int>(rows_.size()));
    redraw();
}

void PageStepsTable::clearResult() {
    rows_.clear();
    rows(0);
    redraw();
}

void PageStepsTable::draw_cell(TableContext context, int row, int col, int x, int y, int w, int h) {
    switch (context) {
        case CONTEXT_STARTPAGE:
            fl_font(FL_HELVETICA, 13);
            return;
        case CONTEXT_COL_HEADER:
            drawCellBackground(x, y, w, h, fl_rgb_color(230, 235, 245));
            drawCellText(headers_[col], x, y, w, h, FL_ALIGN_CENTER);
            return;
        case CONTEXT_ROW_HEADER:
            drawCellBackground(x, y, w, h, fl_rgb_color(245, 245, 245));
            drawCellText(std::to_string(row + 1), x, y, w, h, FL_ALIGN_CENTER);
            return;
        case CONTEXT_CELL: {
            const Fl_Color bg = row_selected(row) ? fl_rgb_color(220, 235, 255) : FL_WHITE;
            drawCellBackground(x, y, w, h, bg);
            drawCellText(cellValue(row, col), x, y, w, h, FL_ALIGN_CENTER);
            return;
        }
        default:
            return;
    }
}

std::string PageStepsTable::cellValue(int row, int col) const {
    if (row < 0 || row >= static_cast<int>(rows_.size())) {
        return {};
    }
    const auto& step = rows_[row];
    switch (col) {
        case 0:
            return std::to_string(step.page);
        case 1:
            return memory_management::pageFramesToString(step.frames);
        case 2:
            return step.hit ? "命中" : "缺页";
        default:
            return {};
    }
}
