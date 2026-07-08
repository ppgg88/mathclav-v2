#include "widgets/HistoryDialog.h"

#include "FormulaView.h"

#include "mathclav/core/latex/LatexSerializer.h"

#include <QDebug>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QUuid>
#include <QVBoxLayout>
#include <QWidget>

namespace history = mathclav::core::history;
namespace latex = mathclav::core::latex;

HistoryDialog::HistoryDialog(mathclav::core::ast::Container currentDocument, QString historyPath, QWidget* parent)
    : QDialog(parent), currentDocument_(std::move(currentDocument)), historyPath_(std::move(historyPath)) {
    setWindowTitle(QStringLiteral("MathClav - Historique"));
    resize(600, 500);

    entries_ = history::loadHistory(historyPath_);

    auto* outer = new QVBoxLayout(this);

    auto* saveRow = new QHBoxLayout();
    auto* currentPreview = new FormulaView(this);
    currentPreview->setFormula(latex::render(currentDocument_));
    currentPreview->setFixedHeight(60);
    saveRow->addWidget(currentPreview, /*stretch=*/1);

    nameEdit_ = new QLineEdit(this);
    nameEdit_->setPlaceholderText(QStringLiteral("Nom"));
    saveRow->addWidget(nameEdit_);

    auto* saveButton = new QPushButton(QStringLiteral("Sauvegarder"), this);
    connect(saveButton, &QPushButton::clicked, this, &HistoryDialog::onSaveClicked);
    saveRow->addWidget(saveButton);

    outer->addLayout(saveRow);

    auto* scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    auto* listWidget = new QWidget(scrollArea);
    listLayout_ = new QVBoxLayout(listWidget);
    scrollArea->setWidget(listWidget);
    outer->addWidget(scrollArea, /*stretch=*/1);

    rebuildList();
}

void HistoryDialog::rebuildList() {
    // Clears everything, including the trailing stretch added at the end
    // of this function on the previous call -- it's re-added below, after
    // the current entries, so rows always stack at the top of the scroll
    // area instead of centering in it.
    QLayoutItem* item = nullptr;
    while ((item = listLayout_->takeAt(0)) != nullptr) {
        if (QWidget* w = item->widget()) {
            w->deleteLater();
        }
        delete item;
    }

    for (int i = 0; i < static_cast<int>(entries_.size()); ++i) {
        const history::HistoryEntry& entry = entries_[static_cast<std::size_t>(i)];

        auto* row = new QWidget(this);
        auto* rowLayout = new QHBoxLayout(row);

        auto* nameButton = new QPushButton(entry.name.isEmpty() ? QStringLiteral("(sans nom)") : entry.name, row);
        nameButton->setFlat(true);
        connect(nameButton, &QPushButton::clicked, this, [this, i] {
            emit entryInsertRequested(entries_[static_cast<std::size_t>(i)].document);
            close();
        });
        rowLayout->addWidget(nameButton);

        auto* preview = new FormulaView(row);
        preview->setFormula(latex::render(entry.document));
        preview->setFixedHeight(50);
        rowLayout->addWidget(preview, /*stretch=*/1);

        auto* deleteButton = new QPushButton(QStringLiteral("Supprimer"), row);
        connect(deleteButton, &QPushButton::clicked, this, [this, i] { deleteEntryAt(i); });
        rowLayout->addWidget(deleteButton);

        listLayout_->addWidget(row);
    }
    listLayout_->addStretch(1);
}

void HistoryDialog::onSaveClicked() {
    history::HistoryEntry entry;
    entry.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    entry.name = nameEdit_->text();
    entry.document = currentDocument_;
    entries_.push_back(std::move(entry));
    if (!history::saveHistory(historyPath_, entries_)) {
        qWarning("HistoryDialog: failed to write %s", qPrintable(historyPath_));
    }
    rebuildList();
}

void HistoryDialog::deleteEntryAt(int index) {
    entries_.erase(entries_.begin() + index);
    if (!history::saveHistory(historyPath_, entries_)) {
        qWarning("HistoryDialog: failed to write %s", qPrintable(historyPath_));
    }
    rebuildList();
}
