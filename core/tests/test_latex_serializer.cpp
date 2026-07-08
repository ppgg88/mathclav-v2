#include <catch2/catch_test_macros.hpp>

#include "mathclav/core/ast/Node.h"
#include "mathclav/core/latex/LatexSerializer.h"

using namespace mathclav::core::ast;
using namespace mathclav::core::latex;

namespace {
std::wstring glyph() {
    return std::wstring(1, kEmptySlotGlyph);
}
} // namespace

TEST_CASE("a symbol renders as its literal", "[latex]") {
    REQUIRE(render(Node::symbol(L"\\alpha ")) == L"\\alpha ");
}

TEST_CASE("an empty composite slot renders as the empty glyph", "[latex]") {
    const Node sqrt = Node::composite(Kind::Sqrt);
    REQUIRE(render(sqrt) == L"\\sqrt{" + glyph() + L"}");
}

TEST_CASE("a filled single-slot composite substitutes its child", "[latex]") {
    Node sqrt = Node::composite(Kind::Sqrt);
    sqrt.args[0].push_back(Node::symbol(L"2"));
    REQUIRE(render(sqrt) == L"\\sqrt{2}");
}

TEST_CASE("fraction substitutes each slot independently", "[latex]") {
    Node frac = Node::composite(Kind::Fraction);
    SECTION("both empty") {
        REQUIRE(render(frac) == L"\\frac{" + glyph() + L"}{" + glyph() + L"}");
    }
    SECTION("numerator only") {
        frac.args[0].push_back(Node::symbol(L"1"));
        REQUIRE(render(frac) == L"\\frac{1}{" + glyph() + L"}");
    }
    SECTION("both filled") {
        frac.args[0].push_back(Node::symbol(L"1"));
        frac.args[1].push_back(Node::symbol(L"2"));
        REQUIRE(render(frac) == L"\\frac{1}{2}");
    }
}

TEST_CASE("integral substitutes all four args independently", "[latex]") {
    // Ported from latex.py integral.__str__ (177-204).
    Node integral = Node::composite(Kind::Integral);
    integral.args[0].push_back(Node::symbol(L"0"));
    integral.args[2].push_back(Node::symbol(L"x"));
    const std::wstring rendered = render(integral);
    REQUIRE(rendered == L"\\int_{0}^{" + glyph() + L"}{ x \\: d " + glyph() + L" \\:}");
}

TEST_CASE("SqrtN hints the index slot with a literal 'n' when empty, unlike other kinds",
          "[latex]") {
    // Ported from latex.py sqrt_n.__str__ (128-146): this is the one kind
    // whose "empty" hint isn't the generic ░ glyph.
    Node sqrtN = Node::composite(Kind::SqrtN);
    REQUIRE(render(sqrtN) == L"\\sqrt[n]{" + glyph() + L"}");

    sqrtN.args[0].push_back(Node::symbol(L"3"));
    REQUIRE(render(sqrtN) == L"\\sqrt[3]{" + glyph() + L"}");

    sqrtN.args[1].push_back(Node::symbol(L"8"));
    REQUIRE(render(sqrtN) == L"\\sqrt[3]{8}");
}

TEST_CASE("Text renders empty as an empty \\text{}, not the ░ glyph", "[latex]") {
    // Ported from latex.py texte.__str__ (115-126): unlike every other
    // kind, an empty Text has no ░ placeholder at all.
    const Node text = Node::composite(Kind::Text);
    REQUIRE(render(text) == L"\\text{}");
}

TEST_CASE("Text strips incidental spaces and re-spaces after thin-space macros", "[latex]") {
    Node text = Node::composite(Kind::Text);
    text.args[0].push_back(Node::symbol(L"Hello"));
    text.args[0].push_back(Node::symbol(L"\\:"));
    text.args[0].push_back(Node::symbol(L"World"));
    REQUIRE(render(text) == L"\\text{Hello\\: World}");
}

TEST_CASE("Matrix and System substitute per-cell in row-major order", "[latex]") {
    Node system = Node::makeSystem(2, 1);
    system.args[0].push_back(Node::symbol(L"x=1"));
    system.args[1].push_back(Node::symbol(L"y=2"));
    const std::wstring rendered = render(system);
    REQUIRE(rendered == L"\\left\\{\\begin{matrix}x=1\\\\y=2\\end{matrix}\\right.");
}

TEST_CASE("a container concatenates the rendering of its children", "[latex]") {
    Container c;
    c.push_back(Node::symbol(L"A"));
    c.push_back(Node::composite(Kind::Sqrt));
    c.push_back(Node::symbol(L"B"));
    REQUIRE(render(c) == L"A\\sqrt{" + glyph() + L"}B");
}
