#pragma once

#include "mathclav/core/graph/GraphAst.h"

#include <string>
#include <unordered_map>

namespace mathclav::core::graph {

// Variable bindings available during evaluation: normally just the plot
// variable (e.g. "x"), plus a Sum's or Integral's own bound variable while
// evaluating inside its body.
using Env = std::unordered_map<std::wstring, double>;

// Evaluates a GraphNode. Throws GraphError for an unbound Variable.
// Division by zero and out-of-domain calls (sqrt of a negative number,
// asin outside [-1,1], ...) are *not* thrown: they produce inf/nan via
// normal IEEE-754 semantics, same as legacy's per-point eval() failures
// effectively skip that point -- ChartBuilder (app layer) filters
// non-finite y values the same way graph.py's per-point try/except did
// (graph.py 280-289), which is what actually draws the gap at an
// asymptote instead of a spurious error.
[[nodiscard]] double eval(const GraphNode& node, const Env& env);

} // namespace mathclav::core::graph
