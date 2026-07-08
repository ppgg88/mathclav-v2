#pragma once

#include <stdexcept>
#include <string>
#include <vector>

namespace mathclav::core::graph {

// Thrown by GraphAstBuilder for a construct with no numeric meaning (e.g.
// Matrix, System, Vector, Binom -- kinds legacy's classes simply never
// define a graphStr()/`.math` for, latex.py, pervasive) and by Evaluator
// for a runtime failure (division by zero, domain error). Legacy lets the
// first case surface as an uncaught AttributeError and the second as a
// bare "error" string from integral.graphStr() (latex.py 208-230) that
// propagates into a Python eval() SyntaxError -- both become a single
// typed, catchable exception here (see GrapherDialog, app layer).
class GraphError : public std::runtime_error {
public:
    explicit GraphError(const std::string& message) : std::runtime_error(message) {}
};

enum class GraphKind {
    Number,   // numberValue
    Variable, // variableName, looked up in Evaluator::Env at eval time
    Add,      // children[0] + children[1]
    Sub,      // children[0] - children[1]
    Mul,      // children[0] * children[1]
    Div,      // children[0] / children[1]
    Pow,      // children[0] ^ children[1]
    Neg,      // -children[0]
    Call,     // func(children[0])
    // Bounded sum (index.py sum.graphStr, latex.py 444-468): sums
    // children[2] (the body) over integer values of `variableName` from
    // children[0] to children[1] inclusive.
    Sum,
    // Rectangle-method numeric integral (index.py integral.graphStr,
    // latex.py 208-230): children[0]=lower, children[1]=upper,
    // children[2]=body, integrated over `variableName`.
    Integral,
};

enum class Func {
    Sin,
    Cos,
    Tan,
    Sinc,
    ArcSin,
    ArcCos,
    ArcTan,
    Ln,
    Exp,
    Sqrt,
    Abs,
};

// A single generic recursive node, mirroring ast::Node's design: one tagged
// type instead of a class hierarchy, since every GraphKind's payload is
// homogeneous (0-3 child GraphNodes plus one optional scalar/name/func).
struct GraphNode {
    GraphKind kind = GraphKind::Number;
    double numberValue = 0.0;
    std::wstring variableName; // Variable / Sum's and Integral's bound variable
    Func func = Func::Sin;     // only meaningful for Call
    std::vector<GraphNode> children;
};

} // namespace mathclav::core::graph
