#pragma once

namespace mathclav::app::microtex {

// Owns MicroTeX's process-wide init/release (tex::LaTeX::init/release are
// static, not per-instance -- this wrapper just makes the lifetime explicit
// and RAII-scoped to main() instead of a bare global call).
//
// Resource path resolution: MicroTeX needs its res/ directory (fonts +
// symbol XML tables) at runtime. app/CMakeLists.txt copies MicroTeX's res/
// next to the built binary as a post-build step, in both dev and packaged
// builds (windeployqt/AppImage both preserve the executable's own
// directory), so resolveResPath() only ever needs to look relative to
// QCoreApplication::applicationDirPath().
class MicroTexContext {
public:
    MicroTexContext();
    ~MicroTexContext();

    MicroTexContext(const MicroTexContext&) = delete;
    MicroTexContext& operator=(const MicroTexContext&) = delete;
};

} // namespace mathclav::app::microtex
