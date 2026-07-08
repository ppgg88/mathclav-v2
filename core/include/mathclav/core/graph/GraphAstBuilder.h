#pragma once

#include "mathclav/core/ast/Node.h"
#include "mathclav/core/graph/GraphAst.h"

namespace mathclav::core::graph {

// Builds a GraphNode expression tree directly from a container's children
// -- no text round-trip through LaTeX/eval() like legacy's graphStr()
// (latex.py, pervasive) plus graph.py's character-scan implicit-
// multiplication hack (graph.py 217-222, 270-279). Implicit multiplication
// is resolved as a token-adjacency rule instead (see GraphAstBuilder.cpp):
// any two operand tokens with no explicit operator between them imply a
// '*', which is what legacy's character-pattern matching was approximating
// case by case.
//
// Throws GraphError for any node kind with no numeric meaning (every kind
// legacy's classes never define a graphStr()/`.math` for: Power,
// Subscript, Text, SqrtN, the bound-only/contour/multi integrals, Union,
// Intersection, Product family, Log, Vector, Limit family, Matrix, System,
// Binom, ParenSquare, ParenCurly) instead of legacy's uncaught
// AttributeError.
[[nodiscard]] GraphNode build(const ast::Container& container);

} // namespace mathclav::core::graph
