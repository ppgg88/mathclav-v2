#pragma once

#include <vector>

class QChartView;
class QPainter;

namespace mathclav::app::graphing {

// Draws a small tick mark + numeric label on a QChartView's X/Y axes at
// each asymptote position -- graph.py's `axes.xaxis.set_ticks(...)`
// equivalent (graph.py 323-337), needed because QValueAxis has no API for
// ad-hoc extra tick positions. Call from the view's paintEvent, after the
// base QChartView::paintEvent has drawn the chart itself.
void paintAsymptoteTicks(QChartView& view, QPainter& painter, const std::vector<double>& verticalAsymptotes,
                          const std::vector<double>& horizontalAsymptotes);

} // namespace mathclav::app::graphing
