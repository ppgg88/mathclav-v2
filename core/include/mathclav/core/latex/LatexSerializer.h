#pragma once

#include "mathclav/core/ast/Node.h"

#include <string>

namespace mathclav::core::latex {

// U+2591 LIGHT SHADE -- the "empty slot" glyph shown when a composite's
// argument hasn't been filled in yet, matching legacy's use of '░'
// (latex.py, pervasive).
inline constexpr wchar_t kEmptySlotGlyph = 0x2591;

// Renders a single node or a whole container to a LaTeX source string ready
// to hand to MicroTeX. Pure, no Qt/MicroTeX dependency: replaces the ~30
// hand-written `self.latex.replace('æ1', ...)` chains spread across
// latex.py's classes with one routine driven by NodeCatalog, plus two
// documented special cases (Kind::Text, Kind::SqrtN) where legacy's
// per-class behavior genuinely isn't the generic "empty slot -> ░" pattern.
[[nodiscard]] std::wstring render(const ast::Node& node);
[[nodiscard]] std::wstring render(const ast::Container& container);

} // namespace mathclav::core::latex
