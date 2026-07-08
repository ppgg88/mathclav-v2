#pragma once

#include "mathclav/core/ast/NodeKind.h"

#include <string_view>

namespace mathclav::core::ast {

// Slot placeholders inside a NodeSpec::latexTemplate are single wchar_t
// values drawn from the Unicode Private Use Area (U+E000..U+E007), so a
// template can embed them directly as literals (e.g. L"\\sqrt{}")
// with no risk of colliding with real LaTeX content and no escaping/parsing
// ambiguity (unlike legacy's approach of reusing a visible glyph like '░'
// or a literal letter like 'n' as both the "empty" placeholder and the
// in-template marker -- see latex.py's `.replace('░', ...)` /
// `.replace('n', ...)` calls, which is exactly the kind of stringly-typed
// substitution LatexSerializer::render replaces with one generic routine).
inline constexpr wchar_t kPlaceholderBase = 0xE000;
inline constexpr int kMaxSlots = 8;

[[nodiscard]] constexpr wchar_t placeholderFor(int slot) noexcept {
    return static_cast<wchar_t>(kPlaceholderBase + slot);
}

[[nodiscard]] constexpr bool isPlaceholder(wchar_t c) noexcept {
    return c >= kPlaceholderBase && c < kPlaceholderBase + kMaxSlots;
}

[[nodiscard]] constexpr int placeholderSlot(wchar_t c) noexcept {
    return static_cast<int>(c - kPlaceholderBase);
}

// Which graph function (if any) a kind maps to when building a GraphAst
// (Phase 4). None means the construction has no numeric meaning to plot --
// ported from which legacy classes do/don't define a `.math`
// template / `graphStr()` override (e.g. Matrix, System, Vector, Limit,
// LimitBounded, Binom and Log have neither in latex.py, so graphing them
// there would raise an uncaught AttributeError; here it's a typed,
// catchable GraphError instead -- see GraphAstBuilder, Phase 4).
enum class GraphFunction : std::uint8_t {
    None,
    Sin,
    Cos,
    Tan,
    Sinc,
    ArcSin,
    ArcCos,
    ArcTan,
    Ln,
    Exp,
    Sqrt,
    Abs,
};

struct NodeSpec {
    // Number of child Container args. -1 for Kind::Matrix/Kind::System,
    // whose slot count is rows*cols and only known per-instance (see
    // Node::grid) -- Node::composite() must not be used for those two
    // kinds, only Node::makeMatrix/makeSystem.
    int arity;

    // Ignored for Matrix/System, which build their own template per
    // instance in Node::makeMatrix/makeSystem (row/col count dependent).
    std::wstring_view latexTemplate;

    // True only for Integral and Sum, matching legacy's `supr_opt = True`
    // attribute (latex.py lines 182, 426): CursorOps::deleteBackward
    // tunnels into the *last* slot of these kinds before deleting, instead
    // of deleting the whole node outright (see CursorOps.cpp).
    bool autoDescendOnDelete;

    GraphFunction graphFn;
};

// Single source of truth for every node kind's shape. Never throws; passing
// an out-of-range Kind is a programming error (asserted, not a runtime
// error path).
[[nodiscard]] const NodeSpec& spec(Kind kind) noexcept;

} // namespace mathclav::core::ast
