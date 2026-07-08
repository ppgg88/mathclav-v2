#include <catch2/catch_test_macros.hpp>

#include "mathclav/core/Version.h"

// Placeholder test proving the CMake + Catch2 + CTest wiring works end to
// end on both CI platforms. Replaced in scope by the Phase 1 suite
// (test_cursor_ops.cpp, test_latex_serializer.cpp, ...).
TEST_CASE("core reports a non-empty semantic version", "[version]") {
    REQUIRE_FALSE(mathclav::core::version().empty());
}
