#include <catch2/catch_test_macros.hpp>

#include "mathclav/core/ast/Node.h"
#include "mathclav/core/ast/NodeCatalog.h"

using namespace mathclav::core::ast;

TEST_CASE("placeholder helpers round-trip slot indices", "[ast][catalog]") {
    for (int i = 0; i < 4; ++i) {
        const wchar_t c = placeholderFor(i);
        REQUIRE(isPlaceholder(c));
        REQUIRE(placeholderSlot(c) == i);
    }
    REQUIRE_FALSE(isPlaceholder(L'a'));
    REQUIRE_FALSE(isPlaceholder(L'\\'));
}

TEST_CASE("NodeCatalog reports the arity ported from latex.py", "[ast][catalog]") {
    REQUIRE(spec(Kind::Sqrt).arity == 1);
    REQUIRE(spec(Kind::SqrtN).arity == 2);
    REQUIRE(spec(Kind::Fraction).arity == 2);
    REQUIRE(spec(Kind::Integral).arity == 4); // latex.py integral, 4 args
    REQUIRE(spec(Kind::Sum).arity == 3);
    REQUIRE(spec(Kind::Binom).arity == 2);
    REQUIRE(spec(Kind::Vector).arity == 1);
}

TEST_CASE("Integral and Sum are the only autoDescendOnDelete kinds", "[ast][catalog]") {
    // Ported from latex.py's `supr_opt = True` attribute, present on only
    // these two of the ~39 classes (lines 182, 426).
    REQUIRE(spec(Kind::Integral).autoDescendOnDelete);
    REQUIRE(spec(Kind::Sum).autoDescendOnDelete);
    REQUIRE_FALSE(spec(Kind::Fraction).autoDescendOnDelete);
    REQUIRE_FALSE(spec(Kind::Sqrt).autoDescendOnDelete);
    REQUIRE_FALSE(spec(Kind::IntegralBoundOnly).autoDescendOnDelete);
}

TEST_CASE("Matrix and System report -1 arity (grid-determined, not catalog-fixed)", "[ast][catalog]") {
    REQUIRE(spec(Kind::Matrix).arity == -1);
    REQUIRE(spec(Kind::System).arity == -1);
}

TEST_CASE("graph-function assignment matches which legacy classes had a .math template",
          "[ast][catalog]") {
    // latex.py: sqrt/cos/sin/tan/sinc/arc*/ln/exp/e/norme/norme2 all define
    // `.math`; log/lim/lim1/vect/binom/matrice/system do not.
    REQUIRE(spec(Kind::Sqrt).graphFn == GraphFunction::Sqrt);
    REQUIRE(spec(Kind::Cos).graphFn == GraphFunction::Cos);
    REQUIRE(spec(Kind::Sin).graphFn == GraphFunction::Sin);
    REQUIRE(spec(Kind::EulerPower).graphFn == GraphFunction::Exp);
    REQUIRE(spec(Kind::Exp).graphFn == GraphFunction::Exp);
    REQUIRE(spec(Kind::Norm).graphFn == GraphFunction::Abs);

    REQUIRE(spec(Kind::Log).graphFn == GraphFunction::None);
    REQUIRE(spec(Kind::Limit).graphFn == GraphFunction::None);
    REQUIRE(spec(Kind::LimitBounded).graphFn == GraphFunction::None);
    REQUIRE(spec(Kind::Vector).graphFn == GraphFunction::None);
    REQUIRE(spec(Kind::Binom).graphFn == GraphFunction::None);
    REQUIRE(spec(Kind::Matrix).graphFn == GraphFunction::None);
    REQUIRE(spec(Kind::System).graphFn == GraphFunction::None);
}

TEST_CASE("Node::composite sizes its args from the catalog arity", "[ast][node]") {
    const Node f = Node::composite(Kind::Fraction);
    REQUIRE(f.args.size() == 2);
    REQUIRE(f.args[0].empty());
    REQUIRE(f.args[1].empty());

    const Node integral = Node::composite(Kind::Integral);
    REQUIRE(integral.args.size() == 4);
}

TEST_CASE("Node::makeMatrix/makeSystem size args from rows*cols", "[ast][node]") {
    const Node m = Node::makeMatrix(2, 3);
    REQUIRE(m.args.size() == 6);
    REQUIRE(m.grid.has_value());
    REQUIRE(m.grid->rows == 2);
    REQUIRE(m.grid->cols == 3);
    REQUIRE(m.dynamicTemplate.has_value());

    const Node s = Node::makeSystem(2, 2);
    REQUIRE(s.args.size() == 4);
}
