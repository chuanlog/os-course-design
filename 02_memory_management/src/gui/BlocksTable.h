#pragma once

#include "../memory/MemoryTypes.h"

#include <FL/Fl_Table_Row.H>

#include <string>
#include <vector>

class BlocksTable : public Fl_Table_Row {
public:
    BlocksTable(int x, int y, int w, int h, const char* label = nullptr);

    void setBlocks(const std::vector<memory_management::MemoryBlock>& blocks);
    void clearBlocks();

protected:
    void draw_cell(TableContext context, int row, int col, int x, int y, int w, int h) override;

private:
    std::string cellValue(int row, int col) const;

    std::vector<std::string> headers_;
    std::vector<memory_management::MemoryBlock> rows_;
};
