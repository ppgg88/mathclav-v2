#include "widgets/GrapherDialog.h"

#include "graphing/ChartBuilder.h"

#include "mathclav/core/latex/LatexSerializer.h"

#include <QCheckBox>
#include <QClipboard>
#include <QGridLayout>
#include <QGuiApplication>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QStringList>
#include <QVBoxLayout>

namespace graphing = mathclav::app::graphing;
namespace latex = mathclav::core::latex;

GrapherDialog::GrapherDialog(mathclav::core::ast::Container document, QWidget* parent)
    : QDialog(parent), document_(std::move(document)) {
    setWindowTitle(QStringLiteral("MathClav - Graph"));
    resize(900, 700);

    auto* outer = new QVBoxLayout(this);
    auto* form = new QGridLayout();

    auto addField = [&](const QString& label, int row, int col, const QString& defaultValue = {}) {
        form->addWidget(new QLabel(label, this), row, col);
        auto* edit = new QLineEdit(defaultValue, this);
        form->addWidget(edit, row + 1, col);
        return edit;
    };

    form->addWidget(new QLabel(QStringLiteral("Titre du graphique :"), this), 0, 0);
    titleEdit_ = new QLineEdit(QString::fromStdWString(latex::render(document_)), this);
    form->addWidget(titleEdit_, 1, 0, 1, 2); // spans both columns, unlike the other fields

    xMinEdit_ = addField(QStringLiteral("x minimum :"), 2, 0, QStringLiteral("0"));
    xMaxEdit_ = addField(QStringLiteral("x maximum :"), 2, 1, QStringLiteral("10"));
    yMinEdit_ = addField(QStringLiteral("y minimum :"), 4, 0, QStringLiteral("0"));
    yMaxEdit_ = addField(QStringLiteral("y maximum :"), 4, 1, QStringLiteral("10"));
    xLabelEdit_ = addField(QStringLiteral("Label sur l'axe x :"), 6, 0);
    yLabelEdit_ = addField(QStringLiteral("Label sur l'axe y :"), 6, 1);
    horizontalAsymEdit_ = addField(QStringLiteral("Asymptote horizontale :"), 8, 0);
    verticalAsymEdit_ = addField(QStringLiteral("Asymptote verticale :"), 8, 1);
    stepEdit_ = addField(QStringLiteral("Pas de la trace :"), 10, 0, QStringLiteral("0.01"));
    variableEdit_ = addField(QStringLiteral("Variable du tracé :"), 10, 1, QStringLiteral("x"));

    gridCheck_ = new QCheckBox(QStringLiteral("Grille"), this);
    form->addWidget(gridCheck_, 12, 0);

    auto* graphButton = new QPushButton(QStringLiteral("Graph"), this);
    connect(graphButton, &QPushButton::clicked, this, &GrapherDialog::onGraphClicked);
    form->addWidget(graphButton, 12, 1);

    outer->addLayout(form);

    statusLabel_ = new QLabel(this);
    statusLabel_->setWordWrap(true);
    outer->addWidget(statusLabel_);

    chartView_ = new graphing::GraphChartView(this);
    outer->addWidget(chartView_, /*stretch=*/1);

    auto* copyButton = new QPushButton(QStringLiteral("Copier l'image"), this);
    connect(copyButton, &QPushButton::clicked, this, &GrapherDialog::onCopyImageClicked);
    outer->addWidget(copyButton);

    onGraphClicked();
}

std::vector<double> GrapherDialog::parseAsymptoteList(const QString& text) {
    // Simpler than graph.py's digit-by-digit scanner (199-262), which
    // existed to support decimal commas: this app's fields expect '.' as
    // the decimal separator, split on ';'.
    std::vector<double> values;
    for (const QString& part : text.split(QLatin1Char(';'), Qt::SkipEmptyParts)) {
        bool ok = false;
        const double v = part.trimmed().toDouble(&ok);
        if (ok) {
            values.push_back(v);
        }
    }
    return values;
}

void GrapherDialog::onGraphClicked() {
    graphing::PlotSpec spec;
    spec.xMin = xMinEdit_->text().toDouble();
    spec.xMax = xMaxEdit_->text().toDouble();
    spec.yMin = yMinEdit_->text().toDouble();
    spec.yMax = yMaxEdit_->text().toDouble();
    spec.step = stepEdit_->text().toDouble();
    spec.variable = variableEdit_->text().isEmpty() ? QStringLiteral("x") : variableEdit_->text();
    spec.title = titleEdit_->text();
    spec.xLabel = xLabelEdit_->text();
    spec.yLabel = yLabelEdit_->text();
    spec.horizontalAsymptotes = parseAsymptoteList(horizontalAsymEdit_->text());
    spec.verticalAsymptotes = parseAsymptoteList(verticalAsymEdit_->text());
    spec.grid = gridCheck_->isChecked();

    if (spec.step <= 0 || spec.xMax <= spec.xMin) {
        statusLabel_->setText(QStringLiteral("Paramètres invalides (x maximum > x minimum, pas > 0)."));
        return;
    }

    const std::vector<QString> errors = graphing::buildChart(*chartView_, document_, spec);
    statusLabel_->setText(QStringList(QList<QString>(errors.begin(), errors.end())).join(QStringLiteral(" / ")));
}

void GrapherDialog::onCopyImageClicked() {
    // A transient status message rather than legacy's blocking modal
    // (tk.messagebox.showinfo, graph.py line 160): copying an image is a
    // low-stakes action that doesn't need to interrupt the user.
    QGuiApplication::clipboard()->setPixmap(chartView_->grab());
    statusLabel_->setText(QStringLiteral("Image copiée dans le presse-papier."));
}
