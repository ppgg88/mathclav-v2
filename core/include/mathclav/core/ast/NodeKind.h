#pragma once

#include <cstdint>

namespace mathclav::core::ast {

// The ~39 constructions ported from mathclav/latex.py's classes (mathObject
// and mathSymbol themselves are represented by Container and Kind::Symbol,
// not listed here). See NodeCatalog.cpp for each kind's arity, LaTeX
// template and graph function.
enum class Kind : std::uint8_t {
    Symbol,

    Sqrt,
    SqrtN,
    Power,
    Subscript,
    Fraction,
    Text,

    Integral,
    IntegralBoundOnly,
    ContourIntegral,
    DoubleIntegral,
    DoubleContourIntegral,
    TripleIntegral,
    TripleContourIntegral,

    Paren,
    ParenSquare,
    ParenCurly,

    Union,
    Intersection,

    Sum,
    SumBoundOnly,
    Product,
    ProductBoundOnly,

    Ln,
    Log,
    Exp,
    EulerPower,
    Cos,
    Sin,
    Sinc,
    Tan,
    ArcCos,
    ArcSin,
    ArcTan,

    Vector,
    Limit,
    LimitBounded,

    Matrix,
    System,
    Binom,

    Norm,
    NormDouble,
};

inline constexpr int kNodeKindCount = static_cast<int>(Kind::NormDouble) + 1;

} // namespace mathclav::core::ast
