#pragma once

#include "../memory/MemoryTypes.h"

#include <FL/Fl_Widget.H>

#include <vector>

class MemoryLayoutWidget : public Fl_Widget {
public:
    MemoryLayoutWidget(int x, int y, int w, int h, const char* label = nullptr);

    void setBlocks(const std::vector<memory_management::MemoryBlock>& blocks, int totalSize);
    void clearBlocks();
    void draw() override;

private:
    std::vector<memory_management::MemoryBlock> blocks_;
    int totalSize_ = 0;
};
