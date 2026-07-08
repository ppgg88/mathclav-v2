#pragma once

#include "mathclav/core/ast/Node.h"

#include <QChartView>
#include <QString>

#include <vector>

QT_BEGIN_NAMESPACE
class QChart;
class QValueAxis;
QT_END_NAMESPACE

namespace mathclav::app::graphing {

// Plot parameters, ported from graphScreen's fields (graph.py 91-148).
// verticalAsymptotes/horizontalAsymptotes are already-parsed values (the
// dialog owns turning the raw ';'-separated text fields into these lists;
// see graph.py 199-262 for the character-scan parser this replaces with
// plain QString::split + toDouble).
struct PlotSpec {
    double xMin = 0, xMax = 10, yMin = 0, yMax = 10;
    double step = 0.01;
    QString variable = QStringLiteral("x");
    QString title;
    QString xLabel, yLabel;
    std::vector<double> verticalAsymptotes;   // "Asymptote verticale" field -- vertical lines at x=t
    std::vector<double> horizontalAsymptotes; // "Asymptote horizontale" field -- horizontal lines at y=t
    bool grid = false;
};

// A QChartView that additionally draws small tick marks + numeric labels
// on its axes at each asymptote position (graph.py 323-337's
// `axes.xaxis.set_ticks(...)` equivalent) -- QValueAxis has no API for
// ad-hoc extra ticks, so this is a thin QPainter overlay on top of an
// otherwise fully native QtCharts view, drawn in paintEvent after the base
// class's own painting.
class GraphChartView : public QChartView {
    Q_OBJECT

public:
    explicit GraphChartView(QWidget* parent = nullptr);

    void setAsymptotes(std::vector<double> verticalAsymptotes, std::vector<double> horizontalAsymptotes);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    std::vector<double> verticalAsymptotes_;
    std::vector<double> horizontalAsymptotes_;
};

// Builds one QLineSeries per `\newline`-separated function in `document`
// (graph.py 166-169's stacked-function split), samples it via
// core::graph::build + core::graph::eval over [xMin,xMax] in steps of
// `step`, and populates `view`'s chart. A function that fails to build
// (GraphError, e.g. it contains a Matrix) or evaluate is skipped and its
// message appended to the returned list, instead of legacy's all-or-
// nothing `graph()` returning False for the very first bad function
// (graph.py 280-289).
[[nodiscard]] std::vector<QString> buildChart(GraphChartView& view, const mathclav::core::ast::Container& document,
                                               const PlotSpec& spec);

} // namespace mathclav::app::graphing
