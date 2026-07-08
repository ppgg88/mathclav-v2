#include "widgets/HelpDialog.h"

#include "FormulaView.h"
#include "input/KeymapTables.h"

#include "mathclav/core/latex/LatexSerializer.h"
#include "mathclav/core/settings/SettingsSchema.h"

#include <QDebug>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QPushButton>
#include <QScrollArea>
#include <QStackedWidget>
#include <QVBoxLayout>

namespace latex = mathclav::core::latex;
namespace settings = mathclav::core::settings;
using mathclav::app::input::greekLowercase;
using mathclav::app::input::greekUppercase;
using mathclav::app::input::mathModeCandidates;
using mathclav::core::ast::Node;

namespace {

// Renders a candidate list as one expression separated by thin spaces
// (matching help.py's `tmptext += x.__str__(); tmptext += '\:\:'`, 240-243).
std::wstring joinCandidatesLatex(const std::vector<Node>& candidates) {
    std::wstring result;
    for (const Node& candidate : candidates) {
        result += latex::render(candidate);
        result += L"\\,\\,";
    }
    return result;
}

FormulaView* previewOf(const std::wstring& latexSource, QWidget* parent) {
    auto* view = new FormulaView(parent);
    view->setFormula(latexSource);
    view->setFixedHeight(45);
    return view;
}

} // namespace

HelpDialog::HelpDialog(bool darkTheme, QString settingsPath, QWidget* parent)
    : QDialog(parent), darkTheme_(darkTheme), settingsPath_(std::move(settingsPath)) {
    setWindowTitle(QStringLiteral("MathClav - Aide"));
    resize(900, 600);

    auto* outer = new QVBoxLayout(this);

    auto* buttonRow = new QHBoxLayout();
    toggleButton_ = new QPushButton(this);
    connect(toggleButton_, &QPushButton::clicked, this, &HelpDialog::toggleView);
    buttonRow->addWidget(toggleButton_);
    auto* closeButton = new QPushButton(QStringLiteral("Quitter"), this);
    connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);
    buttonRow->addWidget(closeButton);
    outer->addLayout(buttonRow);

    stack_ = new QStackedWidget(this);
    stack_->addWidget(buildKeyboardView()); // index 0
    stack_->addWidget(buildListView());     // index 1
    outer->addWidget(stack_, /*stretch=*/1);

    const settings::Settings loaded = settings::loadSettings(settingsPath_);
    const bool startInListMode = loaded.helpViewMode == QStringLiteral("list");
    stack_->setCurrentIndex(startInListMode ? 1 : 0);
    toggleButton_->setText(startInListMode ? QStringLiteral("Vue Clavier") : QStringLiteral("Vue Liste"));
}

QWidget* HelpDialog::buildKeyboardView() {
    auto* label = new QLabel(this);
    const QString resource = darkTheme_ ? QStringLiteral(":/clavier_dark.png") : QStringLiteral(":/clavier.png");
    label->setPixmap(QPixmap(resource).scaled(1000, 400, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    label->setAlignment(Qt::AlignCenter);
    return label;
}

QWidget* HelpDialog::buildListView() {
    auto* scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);

    auto* content = new QWidget(scrollArea);
    auto* grid = new QGridLayout(content);
    int row = 0;

    grid->addWidget(new QLabel(QStringLiteral("²  →"), content), row, 0);
    grid->addWidget(new QLabel(QStringLiteral("Mode Math"), content), row, 1);
    ++row;

    grid->addWidget(new QLabel(QStringLiteral("CTRL  →"), content), row, 0);
    grid->addWidget(new QLabel(QStringLiteral("Mode Grec"), content), row, 1);
    ++row;

    for (char letter = 'A'; letter <= 'Z'; ++letter) {
        grid->addWidget(new QLabel(QString(QChar(letter)) + QStringLiteral("  →"), content), row, 0);

        if (letter == 'H') {
            grid->addWidget(new QLabel(QStringLiteral("Historique"), content), row, 1);
        } else {
            const std::vector<Node> candidates = mathModeCandidates(QChar(letter));
            grid->addWidget(previewOf(joinCandidatesLatex(candidates), content), row, 1);
        }

        const std::wstring greekPair =
            latex::render(greekLowercase(QChar(letter))) + L"  -  " + latex::render(greekUppercase(QChar(letter)));
        grid->addWidget(previewOf(greekPair, content), row, 2);
        ++row;
    }

    scrollArea->setWidget(content);
    return scrollArea;
}

void HelpDialog::toggleView() {
    const bool goingToList = stack_->currentIndex() == 0;
    stack_->setCurrentIndex(goingToList ? 1 : 0);
    toggleButton_->setText(goingToList ? QStringLiteral("Vue Clavier") : QStringLiteral("Vue Liste"));

    settings::Settings current = settings::loadSettings(settingsPath_);
    current.helpViewMode = goingToList ? QStringLiteral("list") : QStringLiteral("keyboard");
    if (!settings::saveSettings(settingsPath_, current)) {
        qWarning("HelpDialog: failed to write %s", qPrintable(settingsPath_));
    }
}
