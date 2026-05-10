#include "ResultsTable.h"

#include <FL/fl_draw.H>

#include <array>
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

ResultsTable::ResultsTable(int x, int y, int w, int h, const char* label)
    : Fl_Table_Row(x, y, w, h, label),
      headers_{"进程", "到达", "运行", "优先级", "完成", "周转", "等待", "响应"} {
    cols(static_cast<int>(headers_.size()));
    rows(0);
    col_header(1);
    row_header(1);
    col_resize(1);
    row_resize(0);
    row_header_width(55);
    row_height_all(28);
    col_header_height(30);
    col_width_all(82);
    col_width(0, 100);
    end();
}

void ResultsTable::setResult(const scheduling::SimulationResult& result) {
    rows_ = result.processes;
    rows(static_cast<int>(rows_.size()));
    redraw();
}

void ResultsTable::clearResult() {
    rows_.clear();
    rows(0);
    redraw();
}

void ResultsTable::draw_cell(TableContext context, int row, int col, int x, int y, int w, int h) {
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

std::string ResultsTable::cellValue(int row, int col) const {
    if (row < 0 || row >= static_cast<int>(rows_.size())) {
        return {};
    }

    const auto& process = rows_[row];
    switch (col) {
        case 0:
            return process.name;
        case 1:
            return std::to_string(process.arrival);
        case 2:
            return std::to_string(process.burst);
        case 3:
            return std::to_string(process.priority);
        case 4:
            return std::to_string(process.completion);
        case 5:
            return std::to_string(process.turnaround);
        case 6:
            return std::to_string(process.waiting);
        case 7:
            return std::to_string(process.response);
        default:
            return {};
    }
}
