#pragma once

#include "../sync/SyncTypes.h"

#include <FL/Fl_Table_Row.H>

#include <string>
#include <vector>

class EventTable : public Fl_Table_Row {
public:
    EventTable(int x, int y, int w, int h, const char* label = nullptr);

    void setEvents(const std::vector<sync_demo::EventRecord>& events);
    void clearEvents();

protected:
    void draw_cell(TableContext context, int row, int col, int x, int y, int w, int h) override;

private:
    std::string cellValue(int row, int col) const;

    std::vector<std::string> headers_;
    std::vector<sync_demo::EventRecord> rows_;
};
