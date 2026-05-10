#pragma once

#include "../scheduling/SchedulingTypes.h"

#include <FL/Fl_Table_Row.H>

#include <string>
#include <vector>

class ResultsTable : public Fl_Table_Row {
public:
    ResultsTable(int x, int y, int w, int h, const char* label = nullptr);

    void setResult(const scheduling::SimulationResult& result);
    void clearResult();

protected:
    void draw_cell(TableContext context, int row, int col, int x, int y, int w, int h) override;

private:
    std::string cellValue(int row, int col) const;

    std::vector<std::string> headers_;
    std::vector<scheduling::ProcessState> rows_;
};
