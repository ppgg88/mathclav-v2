# Provides the `LaTeX` target (MicroTeX, native C++ LaTeX rendering) plus
# MATHCLAV_MICROTEX_RES_DIR pointing at its res/ (fonts + symbol tables),
# which must be deployed next to the app binary at runtime -- see
# app/CMakeLists.txt's post-build copy step and
# app/src/microtex/MicroTexContext.cpp's resource-path resolution.
#
# Pinned to a specific commit (not a branch) so CI builds are reproducible.
# QT must be forced ON *before* FetchContent_MakeAvailable: MicroTeX's own
# CMakeLists.txt reads it via a plain `option(QT ...)` declared near the
# bottom of the file but checked with `if (QT)` near the top, so the cache
# value has to already be set when add_subdirectory() runs.

include(FetchContent)

set(QT ON CACHE BOOL "" FORCE)
set(BUILD_EXAMPLE OFF CACHE BOOL "" FORCE)
set(GRAPHICS_DEBUG OFF CACHE BOOL "" FORCE)
set(HAVE_LOG OFF CACHE BOOL "" FORCE)

# MicroTeX detects Qt5-vs-Qt6 itself via find_package(QT NAMES Qt6 Qt5 ...),
# which is unreliable on systems with both installed side by side: it can
# silently resolve to Qt5 even when Qt6 is found first and satisfies every
# requested component (observed on Ubuntu 24.04 with qt6-base-dev +
# qtbase5-dev both present -- CMake's NAMES search doesn't reliably respect
# list order here). Compiling MicroTeX's Qt backend against Qt5 headers
# while linking it into a Qt6 app is an ABI mismatch that doesn't fail at
# link time (symbol names coincide) but segfaults at runtime inside Qt
# calls (e.g. QFont::setBold), so this can't be left to chance. app's own
# `find_package(Qt6 REQUIRED COMPONENTS Widgets)` (called before this file
# is included) already resolved the real Qt6_DIR; excluding the sibling Qt5
# config directory from the search forces MicroTeX's NAMES lookup to land
# on Qt6 too.
if(Qt6_DIR)
  string(REPLACE "Qt6" "Qt5" MATHCLAV_QT5_DIR_GUESS "${Qt6_DIR}")
  list(APPEND CMAKE_IGNORE_PATH "${MATHCLAV_QT5_DIR_GUESS}")
endif()

FetchContent_Declare(
  microtex
  GIT_REPOSITORY https://github.com/NanoMichael/MicroTeX.git
  GIT_TAG 0e3707f6dafebb121d98b53c64364d16fefe481d
)
FetchContent_MakeAvailable(microtex)

set(MATHCLAV_MICROTEX_RES_DIR "${microtex_SOURCE_DIR}/res" CACHE PATH
    "MicroTeX resource directory (fonts, symbol XML tables)" FORCE)
