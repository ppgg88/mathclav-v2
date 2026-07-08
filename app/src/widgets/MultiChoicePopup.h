#pragma once

#include "mathclav/core/ast/Node.h"

#include <QWidget>

#include <vector>

class FormulaView;
class QTimer;

// Floating preview for MathClav's signature "press the same key again to
// cycle through related symbols" feature (index.py multiple_choice(),
// index.py 806-861). Ported from multitouche.py's `multi` class, but as a
// single persistent widget whose child FormulaViews are rebuilt in place
// on each cycle keystroke, instead of destroying and recreating a new
// Toplevel every time (multitouche.py's __init__ does this per keystroke).
//
// A *child* of the anchor widget, not a separate top-level window: an
// earlier version used Qt::Tool + WA_ShowWithoutActivating to try to keep
// it from stealing keyboard input, but that's only a hint -- a real
// window manager reportedly still gave it activation/input focus on a
// user's machine, which made every multi-choice trigger freeze typing for
// up to 1.5s until the popup's own auto-dismiss timer released it. A
// plain child widget can never be window-manager-activated at all (only
// top-level windows can be), which removes the whole risk category
// instead of trying to out-hint an uncooperative WM.
class MultiChoicePopup : public QWidget {
    Q_OBJECT

public:
    explicit MultiChoicePopup(QWidget* anchor);

    // Shows (or updates, if already visible) the candidate row with
    // `activeIndex` highlighted, positioned near the bottom of the
    // anchor's own client area, and (re)starts the 1.5s auto-dismiss
    // timer.
    void showCandidates(const std::vector<mathclav::core::ast::Node>& candidates, int activeIndex);

private:
    void rebuild(const std::vector<mathclav::core::ast::Node>& candidates, int activeIndex);
    void repositionBelowAnchor();

    QWidget* anchor_;
    QTimer* dismissTimer_;
    std::vector<FormulaView*> candidateViews_;
};
