#include "MemoryLayoutWidget.h"

#include <FL/fl_draw.H>

#include <algorithm>
#include <functional>
#include <string>

namespace {

Fl_Color blockColor(const memory_management::MemoryBlock& block) {
    if (!block.allocated) {
        return fl_rgb_color(225, 225, 225);
    }
    const std::size_t hashValue = std::hash<std::string>{}(block.name);
    const unsigned char r = static_cast<unsigned char>(80 + (hashValue % 120));
    const unsigned char g = static_cast<unsigned char>(80 + ((hashValue / 11) % 120));
    const unsigned char b = static_cast<unsigned char>(80 + ((hashValue / 23) % 120));
    return fl_rgb_color(r, g, b);
}

}  // namespace

MemoryLayoutWidget::MemoryLayoutWidget(int x, int y, int w, int h, const char* label)
    : Fl_Widget(x, y, w, h, label) {}

void MemoryLayoutWidget::setBlocks(const std::vector<memory_management::MemoryBlock>& blocks, int totalSize) {
    blocks_ = blocks;
    totalSize_ = totalSize;
    redraw();
}

void MemoryLayoutWidget::clearBlocks() {
    blocks_.clear();
    totalSize_ = 0;
    redraw();
}

void MemoryLayoutWidget::draw() {
    fl_push_clip(x(), y(), w(), h());
    fl_color(245, 245, 245);
    fl_rectf(x(), y(), w(), h());
    fl_color(200, 200, 200);
    fl_rect(x(), y(), w(), h());

    fl_color(FL_BLACK);
    fl_font(FL_HELVETICA_BOLD, 14);
    fl_draw("内存布局", x() + 12, y() + 22);

    if (blocks_.empty() || totalSize_ <= 0) {
        fl_color(FL_DARK3);
        fl_font(FL_HELVETICA, 14);
        fl_draw("初始化或分配后在这里展示内存布局", x() + 12, y() + h() / 2);
        fl_pop_clip();
        return;
    }

    const int leftMargin = 20;
    const int rightMargin = 20;
    const int topMargin = 36;
    const int blockHeight = 58;
    const int usableWidth = std::max(1, w() - leftMargin - rightMargin);

    fl_font(FL_HELVETICA, 12);
    for (std::size_t i = 0; i < blocks_.size(); ++i) {
        const auto& block = blocks_[i];
        const double startRatio = static_cast<double>(block.start) / totalSize_;
        const double endRatio = static_cast<double>(block.start + block.size) / totalSize_;
        const int startX = x() + leftMargin + static_cast<int>(usableWidth * startRatio);
        int endX = x() + leftMargin + static_cast<int>(usableWidth * endRatio);
        if (i + 1 == blocks_.size()) {
            endX = x() + leftMargin + usableWidth;
        }
        const int blockWidth = std::max(40, endX - startX);
        const int blockY = y() + topMargin;

        fl_color(blockColor(block));
        fl_rectf(startX, blockY, blockWidth, blockHeight);
        fl_color(FL_BLACK);
        fl_rect(startX, blockY, blockWidth, blockHeight);

        const std::string label = block.name + "(" + std::to_string(block.size) + ")";
        fl_draw(label.c_str(), startX + 6, blockY + 22);
        fl_draw(std::to_string(block.start).c_str(), startX, blockY + blockHeight + 18);
    }

    const std::string endText = std::to_string(totalSize_);
    fl_draw(endText.c_str(), x() + leftMargin + usableWidth - 10, y() + topMargin + blockHeight + 18);
    fl_pop_clip();
}
