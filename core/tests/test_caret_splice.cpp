#include <catch2/catch_test_macros.hpp>

#include "mathclav/core/ast/Node.h"
#include "mathclav/core/latex/CaretSplice.h"
#include "mathclav/core/latex/LatexSerializer.h"

using namespace mathclav::core::ast;
using namespace mathclav::core::cursor;
using namespace mathclav::core::latex;

TEST_CASE("renderWithCaret splices the caret glyph at the cursor position", "[caret]") {
    Container root;
    root.push_back(Node::symbol(L"A"));
    root.push_back(Node::symbol(L"B"));
    CursorPath path{{}, 1}; // between A and B

    const std::wstring rendered = renderWithCaret(root, path);

    REQUIRE(rendered == std::wstring(L"A") + kCaretGlyph + L"B");
}

TEST_CASE("renderWithCaret never mutates the real document", "[caret]") {
    Container root;
    root.push_back(Node::symbol(L"A"));
    CursorPath path{{}, 1};

    const std::wstring rendered = renderWithCaret(root, path);
    (void)rendered;

    REQUIRE(root.size() == 1); // unlike legacy's mutate-then-pop, no trace left behind
    REQUIRE(root[0].literal == L"A");
}

TEST_CASE("renderWithCaret adds a slot-boundary marker in the parent when nested", "[caret]") {
    Container root;
    Node frac = Node::composite(Kind::Fraction);
    frac.args[0].push_back(Node::symbol(L"1"));
    root.push_back(std::move(frac));
    root.push_back(Node::symbol(L"B"));

    // Positioned inside the fraction's numerator, right after "1".
    CursorPath path{{PathStep{0, 0}}, 1};

    const std::wstring rendered = renderWithCaret(root, path);

    // The fraction's numerator shows "1" + caret; the marker is spliced
    // into the root container right after the fraction (index 1).
    const std::wstring expectedFrac = L"\\frac{1" + std::wstring(1, kCaretGlyph) + L"}{" +
                                       std::wstring(1, kEmptySlotGlyph) + L"}";
    REQUIRE(rendered == expectedFrac + kSlotBoundaryMarker + L"B");
}
