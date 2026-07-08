#pragma once

#include <string>

namespace tex {
class TeXRender;
}
class QPainter;

namespace mathclav::app::microtex {

// RAII owner of a single tex::TeXRender*, matching the pattern confirmed in
// MicroTeX's own qt_texwidget.cpp sample: reparse on every formula change
// (cheap enough for per-keystroke use, no debouncing needed), delete the
// previous render before building the next one.
class FormulaRenderer {
public:
    FormulaRenderer() = default;
    ~FormulaRenderer();

    FormulaRenderer(const FormulaRenderer&) = delete;
    FormulaRenderer& operator=(const FormulaRenderer&) = delete;

    // width <= 0 means unconstrained (single line, no wrapping) -- what
    // FormulaView always wants, since the editor never wraps formulas.
    void setFormula(const std::wstring& latex, int width, float textSizePt, unsigned int foregroundArgb);

    [[nodiscard]] bool isValid() const { return render_ != nullptr; }
    [[nodiscard]] int width() const;
    [[nodiscard]] int height() const;
    [[nodiscard]] int depth() const;
    [[nodiscard]] float baseline() const;

    void draw(QPainter& painter, int x, int y) const;

private:
    tex::TeXRender* render_ = nullptr;
};

} // namespace mathclav::app::microtex
