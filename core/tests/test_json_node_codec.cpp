#include <catch2/catch_test_macros.hpp>

#include "mathclav/core/ast/Node.h"
#include "mathclav/core/persistence/JsonNodeCodec.h"

using namespace mathclav::core::ast;
using namespace mathclav::core::persistence;

TEST_CASE("a Symbol round-trips through JSON", "[persistence][json]") {
    const Node original = Node::symbol(L"\\alpha ");
    const Node restored = nodeFromJson(nodeToJson(original));
    REQUIRE(restored == original);
}

TEST_CASE("a composite with filled args round-trips through JSON", "[persistence][json]") {
    Node frac = Node::composite(Kind::Fraction);
    frac.args[0].push_back(Node::symbol(L"1"));
    frac.args[1].push_back(Node::symbol(L"2"));
    const Node restored = nodeFromJson(nodeToJson(frac));
    REQUIRE(restored == frac);
}

TEST_CASE("a nested tree round-trips through JSON", "[persistence][json]") {
    Node sqrt = Node::composite(Kind::Sqrt);
    Node inner = Node::composite(Kind::Fraction);
    inner.args[0].push_back(Node::symbol(L"1"));
    inner.args[1].push_back(Node::symbol(L"2"));
    sqrt.args[0].push_back(std::move(inner));
    sqrt.args[0].push_back(Node::symbol(L"x"));

    Container original;
    original.push_back(std::move(sqrt));
    original.push_back(Node::symbol(L"+"));
    original.push_back(Node::symbol(L"1"));

    const Container restored = containerFromJson(containerToJson(original));
    REQUIRE(restored == original);
}

TEST_CASE("Matrix round-trips its rows/cols and per-cell content", "[persistence][json]") {
    Node m = Node::makeMatrix(2, 2);
    m.args[0].push_back(Node::symbol(L"1"));
    m.args[3].push_back(Node::symbol(L"4"));
    const Node restored = nodeFromJson(nodeToJson(m));
    REQUIRE(restored.kind == Kind::Matrix);
    REQUIRE(restored.grid.has_value());
    REQUIRE(restored.grid->rows == 2);
    REQUIRE(restored.grid->cols == 2);
    REQUIRE(restored.args[0] == m.args[0]);
    REQUIRE(restored.args[3] == m.args[3]);
}

TEST_CASE("an unknown kind tag throws PersistenceError", "[persistence][json]") {
    QJsonObject obj;
    obj[QStringLiteral("k")] = QStringLiteral("NotARealKind");
    REQUIRE_THROWS_AS(nodeFromJson(obj), PersistenceError);
}

TEST_CASE("a missing \"k\" field throws PersistenceError", "[persistence][json]") {
    QJsonObject obj;
    REQUIRE_THROWS_AS(nodeFromJson(obj), PersistenceError);
}
