#include "widgets/MultiChoicePopup.h"

#include "FormulaView.h"

#include "mathclav/core/latex/LatexSerializer.h"

#include <QHBoxLayout>
#include <QPalette>
#include <QTimer>

namespace {
constexpr int kDismissMs = 1500;
constexpr int kBottomMargin = 20;
} // namespace

MultiChoicePopup::MultiChoicePopup(QWidget* anchor)
    : QWidget(anchor), anchor_(anchor) {
    setFocusPolicy(Qt::NoFocus);
    setAutoFillBackground(true);
    QPalette pal = palette();
    pal.setColor(QPalette::Window, QColor(0xff, 0xf6, 0xd8));
    setPalette(pal);
    setStyleSheet(QStringLiteral("MultiChoicePopup { border: 1px solid gray; }"));

    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(6, 6, 6, 6);
    layout->setSpacing(4);

    dismissTimer_ = new QTimer(this);
    dismissTimer_->setSingleShot(true);
    connect(dismissTimer_, &QTimer::timeout, this, &QWidget::hide);
}

void MultiChoicePopup::showCandidates(const std::vector<mathclav::core::ast::Node>& candidates,
                                       int activeIndex) {
    rebuild(candidates, activeIndex);
    repositionBelowAnchor();
    show();
    raise();
    dismissTimer_->start(kDismissMs);
}

void MultiChoicePopup::rebuild(const std::vector<mathclav::core::ast::Node>& candidates, int activeIndex) {
    // Immediate delete, not deleteLater(): this widget fully owns
    // candidateViews_ and isn't called from within one of their own event
    // handlers, so there's no reentrancy hazard deleteLater() guards
    // against -- and deferring risked two rapid-fire triggers (e.g. '+'
    // immediately followed by '=', both mode-independent) each adding a
    // fresh batch to the layout before the previous batch's deferred
    // deletion actually ran, leaving stale candidates visibly stacked
    // alongside the new ones.
    for (FormulaView* view : candidateViews_) {
        delete view;
    }
    candidateViews_.clear();

    auto* hbox = qobject_cast<QHBoxLayout*>(layout());
    for (int i = 0; i < static_cast<int>(candidates.size()); ++i) {
        auto* view = new FormulaView(this);
        view->setFormula(mathclav::core::latex::render(candidates[static_cast<std::size_t>(i)]));
        view->setHighlighted(i == activeIndex);
        hbox->addWidget(view);
        candidateViews_.push_back(view);
    }
    adjustSize();
}

void MultiChoicePopup::repositionBelowAnchor() {
    // Relative to the anchor's own client area now that this is a child
    // widget, not screen coordinates -- bottom-center of the anchor,
    // matching multitouche.py's original "near the bottom of the screen"
    // placement closely enough while staying confined to (and therefore
    // never able to be mistaken for a window-manager-level window
    // outside) the anchor.
    const int x = qMax(0, (anchor_->width() - width()) / 2);
    const int y = qMax(0, anchor_->height() - height() - kBottomMargin);
    move(x, y);
}
