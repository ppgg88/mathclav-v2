#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "mathclav/core/ast/Node.h"
#include "mathclav/core/graph/GraphAstBuilder.h"

using namespace mathclav::core::ast;
using namespace mathclav::core::graph;

TEST_CASE("a digit run merges into one multi-digit number", "[graph][builder]") {
    Container c;
    c.push_back(Node::symbol(L"1"));
    c.push_back(Node::symbol(L"2"));
    c.push_back(Node::symbol(L"3"));
    const GraphNode n = build(c);
    REQUIRE(n.kind == GraphKind::Number);
    REQUIRE(n.numberValue == Catch::Approx(123.0));
}

TEST_CASE("implicit multiplication between a number and a variable", "[graph][builder]") {
    // Ported behavior from graph.py's char-scan heuristic (217-222): "2x" means 2*x.
    Container c;
    c.push_back(Node::symbol(L"2"));
    c.push_back(Node::symbol(L"x"));
    const GraphNode n = build(c);
    REQUIRE(n.kind == GraphKind::Mul);
    REQUIRE(n.children[0].kind == GraphKind::Number);
    REQUIRE(n.children[0].numberValue == Catch::Approx(2.0));
    REQUIRE(n.children[1].kind == GraphKind::Variable);
    REQUIRE(n.children[1].variableName == L"x");
}

TEST_CASE("implicit multiplication between a closing paren and a variable", "[graph][builder]") {
    // "(x)y" means (x)*y -- the token-adjacency rule generalizes graph.py's
    // ')' followed by a letter/backslash/'(' special case (276-278).
    Container c;
    Node paren = Node::composite(Kind::Paren);
    paren.args[0].push_back(Node::symbol(L"x"));
    c.push_back(std::move(paren));
    c.push_back(Node::symbol(L"y"));
    const GraphNode n = build(c);
    REQUIRE(n.kind == GraphKind::Mul);
    REQUIRE(n.children[0].kind == GraphKind::Variable);
    REQUIRE(n.children[0].variableName == L"x");
    REQUIRE(n.children[1].variableName == L"y");
}

TEST_CASE("operator precedence: a+b*c groups the multiplication first", "[graph][builder]") {
    Container c;
    c.push_back(Node::symbol(L"a"));
    c.push_back(Node::symbol(L"+"));
    c.push_back(Node::symbol(L"b"));
    c.push_back(Node::symbol(L"*"));
    c.push_back(Node::symbol(L"c"));
    const GraphNode n = build(c);
    REQUIRE(n.kind == GraphKind::Add);
    REQUIRE(n.children[0].variableName == L"a");
    REQUIRE(n.children[1].kind == GraphKind::Mul);
    REQUIRE(n.children[1].children[0].variableName == L"b");
    REQUIRE(n.children[1].children[1].variableName == L"c");
}

TEST_CASE("exponentiation is right-associative: a^b^c is a^(b^c)", "[graph][builder]") {
    Container c;
    c.push_back(Node::symbol(L"a"));
    c.push_back(Node::symbol(L"^"));
    c.push_back(Node::symbol(L"b"));
    c.push_back(Node::symbol(L"^"));
    c.push_back(Node::symbol(L"c"));
    const GraphNode n = build(c);
    REQUIRE(n.kind == GraphKind::Pow);
    REQUIRE(n.children[0].variableName == L"a");
    REQUIRE(n.children[1].kind == GraphKind::Pow);
    REQUIRE(n.children[1].children[0].variableName == L"b");
    REQUIRE(n.children[1].children[1].variableName == L"c");
}

TEST_CASE("Fraction builds as division", "[graph][builder]") {
    Node frac = Node::composite(Kind::Fraction);
    frac.args[0].push_back(Node::symbol(L"1"));
    frac.args[1].push_back(Node::symbol(L"2"));
    Container c;
    c.push_back(std::move(frac));
    const GraphNode n = build(c);
    REQUIRE(n.kind == GraphKind::Div);
    REQUIRE(n.children[0].numberValue == Catch::Approx(1.0));
    REQUIRE(n.children[1].numberValue == Catch::Approx(2.0));
}

TEST_CASE("pi becomes its hardcoded decimal value, matching legacy exactly", "[graph][builder]") {
    // Ported from mathSymbol.graphStr() (latex.py 52): '\pi' -> '3.14159265'.
    Container c;
    c.push_back(Node::symbol(L"\\pi "));
    const GraphNode n = build(c);
    REQUIRE(n.kind == GraphKind::Number);
    REQUIRE(n.numberValue == Catch::Approx(3.14159265));
}

TEST_CASE("the thin-space symbol contributes no token", "[graph][builder]") {
    Container c;
    c.push_back(Node::symbol(L"x"));
    c.push_back(Node::symbol(L"\\: "));
    const GraphNode n = build(c);
    REQUIRE(n.kind == GraphKind::Variable);
    REQUIRE(n.variableName == L"x");
}

TEST_CASE("a construct with no numeric meaning throws GraphError", "[graph][builder]") {
    // Matrix has no graphStr()/`.math` in legacy (latex.py) -- ungraphable there too.
    Container c;
    c.push_back(Node::makeMatrix(2, 2));
    REQUIRE_THROWS_AS(build(c), GraphError);
}
