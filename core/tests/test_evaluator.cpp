#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <cmath>

#include "mathclav/core/ast/Node.h"
#include "mathclav/core/graph/Evaluator.h"
#include "mathclav/core/graph/GraphAstBuilder.h"

using namespace mathclav::core::ast;
using namespace mathclav::core::graph;

namespace {
GraphNode callOf(Kind kind, const std::wstring& argLiteral) {
    Node n = Node::composite(kind);
    n.args[0].push_back(Node::symbol(argLiteral));
    Container c;
    c.push_back(std::move(n));
    return build(c);
}
} // namespace

TEST_CASE("arithmetic precedence evaluates correctly", "[graph][eval]") {
    Container c;
    c.push_back(Node::symbol(L"2"));
    c.push_back(Node::symbol(L"+"));
    c.push_back(Node::symbol(L"3"));
    c.push_back(Node::symbol(L"*"));
    c.push_back(Node::symbol(L"4"));
    REQUIRE(eval(build(c), {}) == Catch::Approx(14.0));
}

TEST_CASE("implicit multiplication evaluates with the bound variable", "[graph][eval]") {
    Container c;
    c.push_back(Node::symbol(L"2"));
    c.push_back(Node::symbol(L"x"));
    REQUIRE(eval(build(c), {{L"x", 5.0}}) == Catch::Approx(10.0));
}

TEST_CASE("the 1-argument function set matches legacy's graphStr math", "[graph][eval]") {
    REQUIRE(eval(callOf(Kind::Sqrt, L"4"), {}) == Catch::Approx(2.0));
    REQUIRE(eval(callOf(Kind::Ln, L"1"), {}) == Catch::Approx(0.0));
    REQUIRE(eval(callOf(Kind::Exp, L"0"), {}) == Catch::Approx(1.0));
    REQUIRE(eval(callOf(Kind::Cos, L"0"), {}) == Catch::Approx(1.0));
    REQUIRE(eval(callOf(Kind::Sin, L"0"), {}) == Catch::Approx(0.0));
    REQUIRE(eval(callOf(Kind::ArcTan, L"0"), {}) == Catch::Approx(0.0));
}

TEST_CASE("Norm graphs as abs, including of a negative argument", "[graph][eval]") {
    Node norm = Node::composite(Kind::Norm);
    norm.args[0].push_back(Node::symbol(L"-"));
    norm.args[0].push_back(Node::symbol(L"3"));
    Container c;
    c.push_back(std::move(norm));
    REQUIRE(eval(build(c), {}) == Catch::Approx(3.0));
}

TEST_CASE("EulerPower graphs as exp of its argument", "[graph][eval]") {
    // e^{0} = 1 (latex.py e.math = "mt.exp(a)").
    REQUIRE(eval(callOf(Kind::EulerPower, L"0"), {}) == Catch::Approx(1.0));
}

TEST_CASE("division by zero yields a non-finite value instead of throwing", "[graph][eval]") {
    // Matches the practical effect of legacy's per-point eval() failure
    // (graph.py 280-289) skipping that point -- ChartBuilder filters
    // non-finite y values the same way.
    Container c;
    c.push_back(Node::symbol(L"1"));
    c.push_back(Node::symbol(L"/"));
    c.push_back(Node::symbol(L"0"));
    REQUIRE(std::isinf(eval(build(c), {})));
}

TEST_CASE("Sum evaluates as a bounded loop over integer values", "[graph][eval]") {
    // sum_{i=1}^{5} i == 15 (index.py sum.graphStr, latex.py 444-468).
    Node sum = Node::composite(Kind::Sum);
    sum.args[0].push_back(Node::symbol(L"i"));
    sum.args[0].push_back(Node::symbol(L"="));
    sum.args[0].push_back(Node::symbol(L"1"));
    sum.args[1].push_back(Node::symbol(L"5"));
    sum.args[2].push_back(Node::symbol(L"i"));
    Container c;
    c.push_back(std::move(sum));
    REQUIRE(eval(build(c), {}) == Catch::Approx(15.0));
}

TEST_CASE("Integral approximates a known analytic result via the rectangle rule", "[graph][eval]") {
    // integral of x dx from 0 to 1 == 0.5 (latex.py integral.graphStr, 208-230).
    Node integral = Node::composite(Kind::Integral);
    integral.args[0].push_back(Node::symbol(L"0"));
    integral.args[1].push_back(Node::symbol(L"1"));
    integral.args[2].push_back(Node::symbol(L"x"));
    integral.args[3].push_back(Node::symbol(L"x"));
    Container c;
    c.push_back(std::move(integral));
    REQUIRE(eval(build(c), {}) == Catch::Approx(0.5).margin(0.01));
}

TEST_CASE("an unbound variable throws GraphError", "[graph][eval]") {
    Container c;
    c.push_back(Node::symbol(L"y"));
    REQUIRE_THROWS_AS(eval(build(c), {{L"x", 1.0}}), GraphError);
}
