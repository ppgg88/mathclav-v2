#pragma once

#include "mathclav/core/ast/NodeKind.h"

#include <optional>
#include <string>
#include <vector>

namespace mathclav::core::ast {

class Node;

// The equivalent of legacy's mathObject.content: an ordered list of sibling
// nodes. The root document is itself just a Container.
using Container = std::vector<Node>;

struct GridDims {
    int rows = 0;
    int cols = 0;

    friend bool operator==(const GridDims&, const GridDims&) = default;
};

// A single generic, tagged node, used for every one of the ~39 kinds in
// NodeKind.h instead of a 39-class hierarchy or a 39-way variant: reading
// mathclav/latex.py shows every kind's payload is homogeneous (a literal
// for Symbol, or 0-4 homogeneous child Containers for everything else, plus
// a row/col count for Matrix/System) -- see NodeCatalog for the per-kind
// shape (arity, LaTeX template, graph function, auto-descend-on-delete).
//
// A Node never renders itself: LatexSerializer turns it into a
// std::wstring for MicroTeX, and GraphAstBuilder (Phase 4) turns it into an
// optional GraphAst -- both pure, both testable without Qt or MicroTeX.
class Node {
public:
    Kind kind = Kind::Symbol;

    // Only meaningful when kind == Kind::Symbol, e.g. L"\\alpha ", L"=", L"A".
    std::wstring literal;

    // Child argument args. Size == NodeCatalog::spec(kind).arity for every
    // fixed-arity kind; size == rows*cols for Kind::Matrix/Kind::System.
    std::vector<Container> args;

    // Only set for Kind::Matrix / Kind::System.
    std::optional<GridDims> grid;
    std::optional<std::wstring> dynamicTemplate;

    Node() = default;

    [[nodiscard]] static Node symbol(std::wstring literal);
    [[nodiscard]] static Node composite(Kind kind);
    [[nodiscard]] static Node makeMatrix(int rows, int cols);
    [[nodiscard]] static Node makeSystem(int rows, int cols);

    friend bool operator==(const Node&, const Node&) = default;
};

} // namespace mathclav::core::ast
