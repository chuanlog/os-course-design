#include "EventTable.h"

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

EventTable::EventTable(int x, int y, int w, int h, const char* label)
    : Fl_Table_Row(x, y, w, h, label), headers_{"序号", "时间(ms)", "线程/角色", "动作", "详情"} {
    cols(static_cast<int>(headers_.size()));
    rows(0);
    col_header(1);
    row_header(1);
    row_header_width(50);
    col_header_height(30);
    row_height_all(28);
    col_width(0, 80);
    col_width(1, 95);
    col_width(2, 170);
    col_width(3, 160);
    col_width(4, 700);
    col_resize(1);
    row_resize(0);
    end();
}

void EventTable::setEvents(const std::vector<sync_demo::EventRecord>& events) {
    rows_ = events;
    rows(static_cast<int>(rows_.size()));
    redraw();
}

void EventTable::clearEvents() {
    rows_.clear();
    rows(0);
    redraw();
}

void EventTable::draw_cell(TableContext context, int row, int col, int x, int y, int w, int h) {
    switch (context) {
        case CONTEXT_STARTPAGE:
            fl_font(FL_HELVETICA, 13);
            return;
        case CONTEXT_COL_HEADER:
            drawCellBackground(x, y, w, h, fl_rgb_color(230, 235, 245));
            drawCellText(headers_[static_cast<std::size_t>(col)], x, y, w, h, FL_ALIGN_CENTER);
            return;
        case CONTEXT_ROW_HEADER:
            drawCellBackground(x, y, w, h, fl_rgb_color(245, 245, 245));
            drawCellText(std::to_string(row + 1), x, y, w, h, FL_ALIGN_CENTER);
            return;
        case CONTEXT_CELL: {
            const Fl_Color bg = row_selected(row) ? fl_rgb_color(220, 235, 255) : FL_WHITE;
            drawCellBackground(x, y, w, h, bg);
            const Fl_Align align = (col == 4) ? FL_ALIGN_LEFT : FL_ALIGN_CENTER;
            drawCellText(cellValue(row, col), x, y, w, h, align);
            return;
        }
        default:
            return;
    }
}

std::string EventTable::cellValue(int row, int col) const {
    if (row < 0 || row >= static_cast<int>(rows_.size())) {
        return {};
    }
    const auto& event = rows_[static_cast<std::size_t>(row)];
    switch (col) {
        case 0:
            return std::to_string(event.order);
        case 1:
            return std::to_string(event.timeMs);
        case 2:
            return event.actor;
        case 3:
            return event.action;
        case 4:
            return event.detail;
        default:
            return {};
    }
}
