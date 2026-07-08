#include "MainWindow.h"
#include "microtex/MicroTexContext.h"

#include <QApplication>

int main(int argc, char** argv) {
    QApplication app(argc, argv);
    QApplication::setApplicationName(QStringLiteral("MathClav"));
    QApplication::setOrganizationName(QStringLiteral("MathClav"));

    // Must outlive every FormulaRenderer::setFormula() call (tex::LaTeX's
    // resource tables are process-wide), so it's constructed before and
    // destroyed after the window.
    mathclav::app::microtex::MicroTexContext texContext;

    MainWindow window;
    window.show();

    return QApplication::exec();
}
