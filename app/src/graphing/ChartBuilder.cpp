#include "graphing/ChartBuilder.h"

#include "graphing/AsymptoteOverlay.h"

#include "mathclav/core/graph/Evaluator.h"
#include "mathclav/core/graph/GraphAstBuilder.h"

#include <QChart>
#include <QLegend>
#include <QLegendMarker>
#include <QLineSeries>
#include <QPainter>
#include <QPen>
#include <QStringList>
#include <QValueAxis>

#include <cmath>

namespace mathclav::app::graphing {

namespace ast = mathclav::core::ast;
namespace graph = mathclav::core::graph;

GraphChartView::GraphChartView(QWidget* parent)
    : QChartView(parent) {
    setRenderHint(QPainter::Antialiasing);
}

void GraphChartView::setAsymptotes(std::vector<double> verticalAsymptotes, std::vector<double> horizontalAsymptotes) {
    verticalAsymptotes_ = std::move(verticalAsymptotes);
    horizontalAsymptotes_ = std::move(horizontalAsymptotes);
    update();
}

void GraphChartView::paintEvent(QPaintEvent* event) {
    QChartView::paintEvent(event);
    // QChartView is a QGraphicsView: painting has to target the viewport
    // widget, not the view widget itself (constructing a QPainter on
    // `this` here silently fails -- "Paint device returned engine == 0").
    QPainter painter(viewport());
    painter.setRenderHint(QPainter::Antialiasing);
    paintAsymptoteTicks(*this, painter, verticalAsymptotes_, horizontalAsymptotes_);
}

namespace {

// Splits on a literal "\newline" Symbol (inserted by the Enter-key
// multi-choice trigger, KeyDispatcher.cpp), matching graph.py's stacked-
// function split (166-169) without a text round-trip.
std::vector<ast::Container> splitOnNewline(const ast::Container& document) {
    std::vector<ast::Container> parts;
    ast::Container current;
    for (const ast::Node& node : document) {
        if (node.kind == ast::Kind::Symbol && node.literal == L"\\newline") {
            parts.push_back(std::move(current));
            current = ast::Container{};
            continue;
        }
        current.push_back(node);
    }
    parts.push_back(std::move(current));
    return parts;
}

QLineSeries* makeAsymptoteLine(double x0, double y0, double x1, double y1) {
    auto* line = new QLineSeries();
    line->append(x0, y0);
    line->append(x1, y1);
    QPen pen(Qt::red);
    pen.setStyle(Qt::DashLine);
    pen.setWidthF(1.0);
    line->setPen(pen);
    return line;
}

} // namespace

std::vector<QString> buildChart(GraphChartView& view, const ast::Container& document, const PlotSpec& spec) {
    std::vector<QString> errors;

    auto* chart = new QChart();
    chart->setTitle(spec.title);

    auto* xAxis = new QValueAxis();
    xAxis->setRange(spec.xMin, spec.xMax);
    xAxis->setTitleText(spec.xLabel);
    xAxis->setGridLineVisible(spec.grid);

    auto* yAxis = new QValueAxis();
    yAxis->setRange(spec.yMin, spec.yMax);
    yAxis->setTitleText(spec.yLabel);
    yAxis->setGridLineVisible(spec.grid);

    chart->addAxis(xAxis, Qt::AlignBottom);
    chart->addAxis(yAxis, Qt::AlignLeft);

    static const QStringList kLabels = {QStringLiteral("f"), QStringLiteral("g"), QStringLiteral("h"),
                                         QStringLiteral("i"), QStringLiteral("j"), QStringLiteral("k"),
                                         QStringLiteral("l"), QStringLiteral("m"), QStringLiteral("n"),
                                         QStringLiteral("o"), QStringLiteral("p"), QStringLiteral("q"),
                                         QStringLiteral("r"), QStringLiteral("s"), QStringLiteral("t"),
                                         QStringLiteral("u"), QStringLiteral("v"), QStringLiteral("w"),
                                         QStringLiteral("x"), QStringLiteral("y"), QStringLiteral("z")};

    const std::wstring varName = spec.variable.toStdWString();
    const int sampleCount = spec.step > 0 ? static_cast<int>((spec.xMax - spec.xMin) / spec.step) : 0;

    int index = 0;
    for (const ast::Container& fn : splitOnNewline(document)) {
        const QString label = index < kLabels.size() ? kLabels[index] : QStringLiteral("f%1").arg(index);
        graph::GraphNode expr;
        try {
            expr = graph::build(fn);
        } catch (const graph::GraphError& e) {
            errors.push_back(QStringLiteral("%1(x) : %2").arg(label, QString(e.what())));
            ++index;
            continue;
        }

        auto* series = new QLineSeries();
        series->setName(label + QStringLiteral("(x)"));
        bool anyPoint = false;
        for (int i = 0; i <= sampleCount; ++i) {
            const double x = spec.xMin + static_cast<double>(i) * spec.step;
            const graph::Env env{{varName, x}};
            double y = 0;
            try {
                y = graph::eval(expr, env);
            } catch (const graph::GraphError&) {
                continue;
            }
            if (!std::isfinite(y)) {
                continue; // matches legacy's per-point eval() failure skipping that point
            }
            series->append(x, y);
            anyPoint = true;
        }
        if (!anyPoint) {
            errors.push_back(QStringLiteral("%1(x) : aucun point valide sur cet intervalle").arg(label));
        }

        chart->addSeries(series);
        series->attachAxis(xAxis);
        series->attachAxis(yAxis);
        ++index;
    }

    for (const double t : spec.verticalAsymptotes) {
        auto* line = makeAsymptoteLine(t, spec.yMin, t, spec.yMax);
        chart->addSeries(line);
        line->attachAxis(xAxis);
        line->attachAxis(yAxis);
        for (auto* marker : chart->legend()->markers(line)) {
            marker->setVisible(false);
        }
    }
    for (const double t : spec.horizontalAsymptotes) {
        auto* line = makeAsymptoteLine(spec.xMin, t, spec.xMax, t);
        chart->addSeries(line);
        line->attachAxis(xAxis);
        line->attachAxis(yAxis);
        for (auto* marker : chart->legend()->markers(line)) {
            marker->setVisible(false);
        }
    }

    chart->legend()->setVisible(true);
    view.setChart(chart);
    view.setAsymptotes(spec.verticalAsymptotes, spec.horizontalAsymptotes);
    return errors;
}

} // namespace mathclav::app::graphing
