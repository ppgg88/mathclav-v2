#include "microtex/FormulaRenderer.h"

#include "latex.h"
#include "platform/qt/graphic_qt.h"

#include <QPainter>

namespace mathclav::app::microtex {

FormulaRenderer::~FormulaRenderer() {
    delete render_;
}

void FormulaRenderer::setFormula(const std::wstring& latex, int width, float textSizePt,
                                  unsigned int foregroundArgb) {
    tex::TeXRender* next = tex::LaTeX::parse(latex, width, textSizePt, textSizePt / 3.f, foregroundArgb);
    delete render_;
    render_ = next;
}

int FormulaRenderer::width() const {
    return render_ != nullptr ? render_->getWidth() : 0;
}

int FormulaRenderer::height() const {
    return render_ != nullptr ? render_->getHeight() : 0;
}

int FormulaRenderer::depth() const {
    return render_ != nullptr ? render_->getDepth() : 0;
}

float FormulaRenderer::baseline() const {
    return render_ != nullptr ? render_->getBaseline() : 0.f;
}

void FormulaRenderer::draw(QPainter& painter, int x, int y) const {
    if (render_ == nullptr) {
        return;
    }
    tex::Graphics2D_qt g2(&painter);
    render_->draw(g2, x, y);
}

} // namespace mathclav::app::microtex
