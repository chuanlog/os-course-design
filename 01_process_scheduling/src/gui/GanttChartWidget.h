#pragma once

#include "../scheduling/SchedulingTypes.h"

#include <FL/Fl_Widget.H>

#include <vector>

class GanttChartWidget : public Fl_Widget {
public:
    GanttChartWidget(int x, int y, int w, int h, const char* label = nullptr);

    void setTimeline(const std::vector<scheduling::TimelineSegment>& timeline);
    void clearTimeline();
    void draw() override;

private:
    std::vector<scheduling::TimelineSegment> timeline_;
};
