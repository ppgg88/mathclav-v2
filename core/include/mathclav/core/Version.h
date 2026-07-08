#pragma once

#include <string_view>

namespace mathclav::core {

// Semantic version of mathclav_core, kept in lockstep with the top-level
// CMakeLists.txt `project(MathClav VERSION ...)` call.
[[nodiscard]] std::string_view version() noexcept;

} // namespace mathclav::core
