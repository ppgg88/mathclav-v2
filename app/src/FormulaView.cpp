#include "FormulaView.h"

#include <QPainter>
#include <QPaintEvent>
#include <QResizeEvent>

namespace {
constexpr int kPadding = 16;
constexpr float kTextSizePt = 24.f;
} // namespace

FormulaView::FormulaView(QWidget* parent)
    : QWidget(parent) {
    setAutoFillBackground(true);
    // Needed for an externally-applied QSS border/border-radius (see
    // MainWindow::applyTheme's "#formulaCanvas" rule) to actually paint --
    // without it, Qt's stylesheet engine still patches the palette's
    // Window role for plain background-color (so autoFillBackground alone
    // picks that much up), but border/radius are silently ignored on a
    // widget with a custom paintEvent unless this is set.
    setAttribute(Qt::WA_StyledBackground, true);
    QPalette pal = palette();
    pal.setColor(QPalette::Window, Qt::white);
    setPalette(pal);
}

void FormulaView::setFormula(const std::wstring& latexSource) {
    latexSource_ = latexSource;
    reparse();
}

void FormulaView::setHighlighted(bool highlighted) {
    QPalette pal = palette();
    pal.setColor(QPalette::Window, highlighted ? QColor(0xcf, 0xe8, 0xff) : Qt::white);
    setPalette(pal);
    update();
}

void FormulaView::setForegroundColor(unsigned int argb) {
    if (foregroundArgb_ == argb) return;
    foregroundArgb_ = argb;
    if (!latexSource_.empty()) reparse();
}

void FormulaView::reparse() {
    const int width = qMax(this->width() - kPadding * 2, 0);
    renderer_.setFormula(latexSource_, width, kTextSizePt, foregroundArgb_);
    updateGeometry();
    update();
}

QSize FormulaView::sizeHint() const {
    if (!renderer_.isValid()) {
        return {2 * kPadding, static_cast<int>(kTextSizePt) + 2 * kPadding};
    }
    return {renderer_.width() + 2 * kPadding, renderer_.height() + renderer_.depth() + 2 * kPadding};
}

void FormulaView::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    renderer_.draw(painter, kPadding, kPadding);
}

void FormulaView::resizeEvent(QResizeEvent* event) {
    QWidget::resizeEvent(event);
    if (!latexSource_.empty()) {
        reparse();
    }
}
