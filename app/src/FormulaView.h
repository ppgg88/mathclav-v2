#pragma once

#include "microtex/FormulaRenderer.h"

#include <QWidget>

#include <string>

// Pure rendering widget: given a LaTeX source string (already produced by
// core::latex::CaretSplice::renderWithCaret upstream in MainWindow -- this
// widget has no notion of Document/CursorPath, only of a string to
// reparse), draws it via MicroTeX. Reparses on every setFormula() call,
// matching MicroTeX's own qt_texwidget.cpp sample: cheap enough per
// keystroke, no debouncing needed.
class FormulaView : public QWidget {
    Q_OBJECT

public:
    explicit FormulaView(QWidget* parent = nullptr);

    void setFormula(const std::wstring& latexSource);

    // Used by MultiChoicePopup to mark the currently-active candidate
    // among the ones it displays (multitouche.py's per-candidate
    // highlight).
    void setHighlighted(bool highlighted);

    // Lets MainWindow keep the main canvas's glyph color legible against
    // its own theme-dependent background (MainWindow::applyTheme); left
    // at its default (near-black) for every other FormulaView instance
    // (MultiChoicePopup's candidate previews, which always sit on a fixed
    // light background regardless of app theme).
    void setForegroundColor(unsigned int argb);

    [[nodiscard]] QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    void reparse();

    mathclav::app::microtex::FormulaRenderer renderer_;
    std::wstring latexSource_;
    unsigned int foregroundArgb_ = 0xff202020;
};
