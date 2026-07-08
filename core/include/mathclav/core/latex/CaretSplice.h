#pragma once

#include "mathclav/core/cursor/CursorPath.h"

#include <string>

namespace mathclav::core::latex {

// U+00A6 BROKEN BAR -- the visual caret glyph, matching legacy's
// mathSymbol(chr(166)) (index.py line 865).
inline constexpr wchar_t kCaretGlyph = 0x00A6;

// The dimmed marker shown in the *parent* container at the position of the
// composite currently being edited into, matching legacy's
// mathSymbol(r'\:\:░') (index.py line 868).
inline constexpr const wchar_t* kSlotBoundaryMarker = L"\\:\\:░";

// Renders `document` with a visual caret spliced in at `path` -- and, when
// `path` is nested inside a composite, a dimmed slot-boundary marker in the
// parent container showing which argument is being edited. Ported from
// graph() (index.py 862-878), but operates on a copy rather than mutating
// the real document and popping the markers back off afterward: legacy's
// approach is fragile if any early return skipped the pop (an entire class
// of bug this eliminates by construction). Pure and fully testable without
// Qt or MicroTeX.
[[nodiscard]] std::wstring renderWithCaret(const ast::Container& document, const cursor::CursorPath& path);

} // namespace mathclav::core::latex
