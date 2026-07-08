#pragma once

#include "mathclav/core/ast/Node.h"

#include <QDialog>

#include <vector>

class QLineEdit;
class QCheckBox;
class QLabel;

namespace mathclav::app::graphing {
class GraphChartView;
}

// Ported from graphScreen (graph.py 42-155): the plot-parameter form, plus
// the chart itself embedded in the same dialog (legacy opens a second,
// separate matplotlib window for the plot -- consolidating both into one
// dialog is a deliberate simplification; the two-window split was an
// artifact of matplotlib's own display model, not a feature).
class GrapherDialog : public QDialog {
    Q_OBJECT

public:
    explicit GrapherDialog(mathclav::core::ast::Container document, QWidget* parent = nullptr);

private slots:
    void onGraphClicked();
    void onCopyImageClicked();

private:
    [[nodiscard]] static std::vector<double> parseAsymptoteList(const QString& text);

    mathclav::core::ast::Container document_;

    QLineEdit* titleEdit_ = nullptr;
    QLineEdit* xMinEdit_ = nullptr;
    QLineEdit* xMaxEdit_ = nullptr;
    QLineEdit* yMinEdit_ = nullptr;
    QLineEdit* yMaxEdit_ = nullptr;
    QLineEdit* xLabelEdit_ = nullptr;
    QLineEdit* yLabelEdit_ = nullptr;
    QLineEdit* horizontalAsymEdit_ = nullptr; // "Asymptote horizontale" -> horizontal lines at y=t
    QLineEdit* verticalAsymEdit_ = nullptr;   // "Asymptote verticale" -> vertical lines at x=t
    QLineEdit* stepEdit_ = nullptr;
    QLineEdit* variableEdit_ = nullptr;
    QCheckBox* gridCheck_ = nullptr;
    QLabel* statusLabel_ = nullptr;

    mathclav::app::graphing::GraphChartView* chartView_ = nullptr;
};
