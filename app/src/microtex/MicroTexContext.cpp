#include "microtex/MicroTexContext.h"

#include "latex.h"

#include <QCoreApplication>
#include <QDir>

#include <stdexcept>

namespace mathclav::app::microtex {

namespace {

std::string resolveResPath() {
    // Deployed next to the binary by app/CMakeLists.txt's post-build copy
    // step (both dev and packaged builds).
    const QDir besideBinary(QCoreApplication::applicationDirPath() + "/res");
    if (besideBinary.exists()) {
        return besideBinary.absolutePath().toStdString();
    }
    throw std::runtime_error(
        "MicroTeX res/ directory not found next to the executable ("
        + besideBinary.absolutePath().toStdString()
        + "). Rebuild so the post-build copy step in app/CMakeLists.txt runs.");
}

} // namespace

MicroTexContext::MicroTexContext() {
    tex::LaTeX::init(resolveResPath());
}

MicroTexContext::~MicroTexContext() {
    tex::LaTeX::release();
}

} // namespace mathclav::app::microtex
