#include "graphing/AsymptoteOverlay.h"

#include <QChart>
#include <QChartView>
#include <QPainter>
#include <QValueAxis>

namespace mathclav::app::graphing {

namespace {
constexpr qreal kTickHalfLength = 5.0;
}

void paintAsymptoteTicks(QChartView& view, QPainter& painter, const std::vector<double>& verticalAsymptotes,
                          const std::vector<double>& horizontalAsymptotes) {
    if (verticalAsymptotes.empty() && horizontalAsymptotes.empty()) {
        return;
    }
    QChart* chart = view.chart();
    if (chart == nullptr) {
        return;
    }
    const auto verticalAxes = chart->axes(Qt::Vertical);
    const auto horizontalAxes = chart->axes(Qt::Horizontal);
    if (verticalAxes.isEmpty() || horizontalAxes.isEmpty()) {
        return;
    }
    auto* yAxis = qobject_cast<QValueAxis*>(verticalAxes.first());
    auto* xAxis = qobject_cast<QValueAxis*>(horizontalAxes.first());
    if (yAxis == nullptr || xAxis == nullptr) {
        return;
    }

    painter.save();
    QPen pen(Qt::red);
    pen.setWidth(2);
    painter.setPen(pen);
    QFont font = painter.font();
    font.setPointSize(8);
    painter.setFont(font);

    for (const double t : verticalAsymptotes) {
        const QPointF p = chart->mapToPosition(QPointF(t, yAxis->min()));
        painter.drawLine(QPointF(p.x(), p.y() - kTickHalfLength), QPointF(p.x(), p.y() + kTickHalfLength));
        painter.drawText(QPointF(p.x() - 12, p.y() + 18), QString::number(t, 'g', 4));
    }
    for (const double t : horizontalAsymptotes) {
        const QPointF p = chart->mapToPosition(QPointF(xAxis->min(), t));
        painter.drawLine(QPointF(p.x() - kTickHalfLength, p.y()), QPointF(p.x() + kTickHalfLength, p.y()));
        painter.drawText(QPointF(p.x() - 36, p.y() + 4), QString::number(t, 'g', 4));
    }
    painter.restore();
}

} // namespace mathclav::app::graphing
