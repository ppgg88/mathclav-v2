#include "mathclav/core/latex/LatexSerializer.h"

#include "mathclav/core/ast/NodeCatalog.h"

namespace mathclav::core::latex {

using ast::Container;
using ast::Kind;
using ast::Node;

namespace {

std::wstring substituteGeneric(std::wstring_view tmpl, const std::vector<Container>& args) {
    std::wstring out;
    out.reserve(tmpl.size());
    for (wchar_t c : tmpl) {
        if (ast::isPlaceholder(c)) {
            const auto idx = static_cast<std::size_t>(ast::placeholderSlot(c));
            const Container& slot = args.at(idx);
            out += slot.empty() ? std::wstring(1, kEmptySlotGlyph) : render(slot);
        } else {
            out += c;
        }
    }
    return out;
}

// Ported from texte.__str__ (latex.py 115-126). The space bar is
// intercepted app-wide to insert the thin-space macro "\:" rather than a
// literal ' ' (index.py line 745-747), so no user-typed space ever reaches
// here as a literal ' ' char -- any literal spaces present are incidental
// trailing separators baked into symbol templates (e.g. "\\alpha ").
// Stripping them and re-inserting exactly one space after each "\:" keeps
// multi-symbol text readable without those stray separators colliding.
// Unlike every other kind, an empty Text renders as "" (not the ░ glyph) --
// legacy's texte.__str__ substitutes unconditionally, with no empty check.
std::wstring renderText(const Node& node) {
    const std::wstring inner = node.args.empty() ? std::wstring{} : render(node.args[0]);

    std::wstring stripped;
    stripped.reserve(inner.size());
    for (wchar_t c : inner) {
        if (c != L' ') {
            stripped += c;
        }
    }

    std::wstring result;
    result.reserve(stripped.size() + 8);
    for (std::size_t i = 0; i < stripped.size(); ++i) {
        result += stripped[i];
        if (stripped[i] == L':' && i > 0 && stripped[i - 1] == L'\\') {
            result += L' ';
        }
    }
    return L"\\text{" + result + L"}";
}

// Ported from sqrt_n.__str__ (latex.py 128-146): unlike every other
// two-slot kind, the root-index slot's empty hint is the literal letter
// "n" (a visual cue distinguishing it from the radicand's ░), not the
// generic empty glyph.
std::wstring renderSqrtN(const Node& node) {
    const Container& indexSlot = node.args[0];
    const Container& radicandSlot = node.args[1];
    const std::wstring indexPart = indexSlot.empty() ? std::wstring(1, L'n') : render(indexSlot);
    const std::wstring radicandPart =
        radicandSlot.empty() ? std::wstring(1, kEmptySlotGlyph) : render(radicandSlot);
    return L"\\sqrt[" + indexPart + L"]{" + radicandPart + L"}";
}

} // namespace

std::wstring render(const Node& node) {
    if (node.kind == Kind::Symbol) {
        return node.literal;
    }
    if (node.kind == Kind::Text) {
        return renderText(node);
    }
    if (node.kind == Kind::SqrtN) {
        return renderSqrtN(node);
    }
    if (node.kind == Kind::Matrix || node.kind == Kind::System) {
        return substituteGeneric(*node.dynamicTemplate, node.args);
    }
    return substituteGeneric(ast::spec(node.kind).latexTemplate, node.args);
}

std::wstring render(const Container& container) {
    std::wstring out;
    for (const auto& node : container) {
        out += render(node);
    }
    return out;
}

} // namespace mathclav::core::latex
