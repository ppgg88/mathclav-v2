# Qt6::Core only (QJsonDocument/QJsonObject/QSaveFile for persistence,
# Phase 5) -- no Gui/Widgets, so this library stays headless-testable in CI
# with no display server.
find_package(Qt6 REQUIRED COMPONENTS Core)

# Generated from the top-level project(MathClav VERSION ...) call so the
# version string has exactly one source of truth -- previously
# core/src/Version.cpp hardcoded its own literal copy, independent of both
# CMakeLists.txt's version and packaging/windows/mathclav.nsi's, with no
# way to notice the three drifting apart.
configure_file(
    src/Version.cpp.in
    ${CMAKE_CURRENT_BINARY_DIR}/generated/Version.cpp
    @ONLY
)

add_library(mathclav_core STATIC
    ${CMAKE_CURRENT_BINARY_DIR}/generated/Version.cpp
    src/ast/Node.cpp
    src/ast/NodeCatalog.cpp
    src/cursor/CursorPath.cpp
    src/cursor/CursorOps.cpp
    src/cursor/MultiChoiceCycle.cpp
    src/latex/LatexSerializer.cpp
    src/latex/CaretSplice.cpp
    src/graph/GraphAstBuilder.cpp
    src/graph/Evaluator.cpp
    src/persistence/JsonNodeCodec.cpp
    src/session/SessionStore.cpp
    src/history/HistoryStore.cpp
    src/settings/SettingsSchema.cpp
)

target_include_directories(mathclav_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(mathclav_core PUBLIC Qt6::Core)

target_compile_features(mathclav_core PUBLIC cxx_std_20)
mathclav_enable_strict_warnings(mathclav_core)

if(MATHCLAV_BUILD_TESTS)
    add_subdirectory(tests)
endif()
