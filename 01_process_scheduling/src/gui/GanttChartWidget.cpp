#include "GanttChartWidget.h"

#include <FL/fl_draw.H>

#include <algorithm>
#include <functional>
#include <string>

namespace {

Fl_Color colorForName(const std::string& name) {
    if (name == "IDLE") {
        return fl_rgb_color(220, 220, 220);
    }
    const std::size_t hashValue = std::hash<std::string>{}(name);
    const unsigned char r = static_cast<unsigned char>(80 + (hashValue % 120));
    const unsigned char g = static_cast<unsigned char>(80 + ((hashValue / 13) % 120));
    const unsigned char b = static_cast<unsigned char>(80 + ((hashValue / 29) % 120));
    return fl_rgb_color(r, g, b);
}

}  // namespace

GanttChartWidget::GanttChartWidget(int x, int y, int w, int h, const char* label)
    : Fl_Widget(x, y, w, h, label) {}

void GanttChartWidget::setTimeline(const std::vector<scheduling::TimelineSegment>& timeline) {
    timeline_ = timeline;
    redraw();
}

void GanttChartWidget::clearTimeline() {
    timeline_.clear();
    redraw();
}

void GanttChartWidget::draw() {
    fl_push_clip(x(), y(), w(), h());
    fl_color(245, 245, 245);
    fl_rectf(x(), y(), w(), h());
    fl_color(200, 200, 200);
    fl_rect(x(), y(), w(), h());

    fl_color(FL_BLACK);
    fl_font(FL_HELVETICA_BOLD, 14);
    fl_draw("Gantt 图", x() + 12, y() + 22);

    if (timeline_.empty()) {
        fl_color(FL_DARK3);
        fl_font(FL_HELVETICA, 14);
        fl_draw("运行后在这里显示调度时间线", x() + 12, y() + h() / 2);
        fl_pop_clip();
        return;
    }

    const int leftMargin = 20;
    const int rightMargin = 20;
    const int topMargin = 36;
    const int blockHeight = 56;
    const int axisY = y() + topMargin + blockHeight + 22;
    const int usableWidth = std::max(1, w() - leftMargin - rightMargin);
    const int totalTime = std::max(1, timeline_.back().end);

    fl_font(FL_HELVETICA, 12);
    for (std::size_t i = 0; i < timeline_.size(); ++i) {
        const auto& segment = timeline_[i];
        const double startRatio = static_cast<double>(segment.start) / totalTime;
        const double endRatio = static_cast<double>(segment.end) / totalTime;

        const int startX = x() + leftMargin + static_cast<int>(usableWidth * startRatio);
        int endX = x() + leftMargin + static_cast<int>(usableWidth * endRatio);
        if (i + 1 == timeline_.size()) {
            endX = x() + leftMargin + usableWidth;
        }
        const int blockWidth = std::max(30, endX - startX);
        const int blockY = y() + topMargin;

        fl_color(colorForName(segment.name));
        fl_rectf(startX, blockY, blockWidth, blockHeight);
        fl_color(FL_BLACK);
        fl_rect(startX, blockY, blockWidth, blockHeight);

        fl_draw(segment.name.c_str(), startX + 6, blockY + 22);

        const std::string startText = std::to_string(segment.start);
        fl_draw(startText.c_str(), startX, axisY);
    }

    const std::string endText = std::to_string(timeline_.back().end);
    fl_draw(endText.c_str(), x() + leftMargin + usableWidth - 10, axisY);
    fl_pop_clip();
}
