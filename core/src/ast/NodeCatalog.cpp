#include "mathclav/core/ast/NodeCatalog.h"

#include <cassert>
#include <string>

namespace mathclav::core::ast {

namespace {

// clang-format off
constexpr wchar_t P0 = placeholderFor(0);
constexpr wchar_t P1 = placeholderFor(1);
constexpr wchar_t P2 = placeholderFor(2);
constexpr wchar_t P3 = placeholderFor(3);
// clang-format on

} // namespace

// -Wswitch-enum (enabled for this target, see cmake/Warnings.cmake) turns a
// missed Kind here into a build failure, which is the whole point of using
// an exhaustive switch instead of an array indexed by static_cast<int>(kind):
// adding a new Kind cannot silently ship with a garbage/default spec.
const NodeSpec& spec(Kind kind) noexcept {
    switch (kind) {
        case Kind::Symbol: {
            // Symbols carry no template/arity of their own -- their text
            // comes from Node::literal directly.
            static const NodeSpec s{0, L"", false, GraphFunction::None};
            return s;
        }
        case Kind::Sqrt: {
            static const std::wstring t = std::wstring(L"\\sqrt{") + P0 + L"}";
            static const NodeSpec s{1, t, false, GraphFunction::Sqrt};
            return s;
        }
        case Kind::SqrtN: {
            // Slot 0 = root index, slot 1 = radicand (latex.py sqrt_n,
            // lines 128-146). LatexSerializer special-cases slot 0's empty
            // hint to the literal "n" instead of the usual ░ glyph, matching
            // legacy's `\sqrt[n]{░}` when both args are empty.
            static const std::wstring t = std::wstring(L"\\sqrt[") + P0 + L"]{" + P1 + L"}";
            static const NodeSpec s{2, t, false, GraphFunction::None};
            return s;
        }
        case Kind::Power: {
            static const std::wstring t = std::wstring(L"^{") + P0 + L"}";
            static const NodeSpec s{1, t, false, GraphFunction::None};
            return s;
        }
        case Kind::Subscript: {
            static const std::wstring t = std::wstring(L"_{") + P0 + L"}";
            static const NodeSpec s{1, t, false, GraphFunction::None};
            return s;
        }
        case Kind::Fraction: {
            static const std::wstring t = std::wstring(L"\\frac{") + P0 + L"}{" + P1 + L"}";
            static const NodeSpec s{2, t, false, GraphFunction::None};
            return s;
        }
        case Kind::Text: {
            // LatexSerializer special-cases Text entirely (no ░ when empty,
            // plus the space-normalization from latex.py's texte.__str__,
            // lines 115-126) -- this template is unused but documents the
            // shape for reference.
            static const std::wstring t = std::wstring(L"\\text{") + P0 + L"}";
            static const NodeSpec s{1, t, false, GraphFunction::None};
            return s;
        }
        case Kind::Integral: {
            static const std::wstring t = std::wstring(L"\\int_{") + P0 + L"}^{" + P1 +
                                           L"}{ " + P2 + L" \\: d " + P3 + L" \\:}";
            static const NodeSpec s{4, t, true, GraphFunction::None};
            return s;
        }
        case Kind::IntegralBoundOnly: {
            static const std::wstring t = std::wstring(L"\\int_{") + P0 + L"}";
            static const NodeSpec s{1, t, false, GraphFunction::None};
            return s;
        }
        case Kind::ContourIntegral: {
            static const std::wstring t = std::wstring(L"\\oint_{") + P0 + L"}";
            static const NodeSpec s{1, t, false, GraphFunction::None};
            return s;
        }
        case Kind::DoubleIntegral: {
            static const std::wstring t = std::wstring(L"\\iint_{") + P0 + L"}";
            static const NodeSpec s{1, t, false, GraphFunction::None};
            return s;
        }
        case Kind::DoubleContourIntegral: {
            static const std::wstring t = std::wstring(L"\\oiint_{") + P0 + L"}";
            static const NodeSpec s{1, t, false, GraphFunction::None};
            return s;
        }
        case Kind::TripleIntegral: {
            static const std::wstring t = std::wstring(L"\\iiint_{") + P0 + L"}";
            static const NodeSpec s{1, t, false, GraphFunction::None};
            return s;
        }
        case Kind::TripleContourIntegral: {
            static const std::wstring t = std::wstring(L"\\oiiint_{") + P0 + L"}";
            static const NodeSpec s{1, t, false, GraphFunction::None};
            return s;
        }
        case Kind::Paren: {
            static const std::wstring t = std::wstring(L" \\left(") + P0 + L" \\right) ";
            static const NodeSpec s{1, t, false, GraphFunction::None};
            return s;
        }
        case Kind::ParenSquare: {
            static const std::wstring t = std::wstring(L" \\left[") + P0 + L" \\right] ";
            static const NodeSpec s{1, t, false, GraphFunction::None};
            return s;
        }
        case Kind::ParenCurly: {
            static const std::wstring t = std::wstring(L" \\left\\{") + P0 + L" \\right\\} ";
            static const NodeSpec s{1, t, false, GraphFunction::None};
            return s;
        }
        case Kind::Union: {
            static const std::wstring t = std::wstring(L"\\bigcup_{") + P0 + L"}^{" + P1 + L"}";
            static const NodeSpec s{2, t, false, GraphFunction::None};
            return s;
        }
        case Kind::Intersection: {
            static const std::wstring t = std::wstring(L"\\bigcap_{") + P0 + L"}^{" + P1 + L"}";
            static const NodeSpec s{2, t, false, GraphFunction::None};
            return s;
        }
        case Kind::Sum: {
            static const std::wstring t = std::wstring(L"\\sum_{") + P0 + L"}^{" + P1 + L"}{" + P2 + L"}";
            static const NodeSpec s{3, t, true, GraphFunction::None};
            return s;
        }
        case Kind::SumBoundOnly: {
            static const std::wstring t = std::wstring(L"\\sum_{") + P0 + L"}";
            static const NodeSpec s{1, t, false, GraphFunction::None};
            return s;
        }
        case Kind::Product: {
            static const std::wstring t = std::wstring(L"\\prod_{") + P0 + L"}^{" + P1 + L"}";
            static const NodeSpec s{2, t, false, GraphFunction::None};
            return s;
        }
        case Kind::ProductBoundOnly: {
            static const std::wstring t = std::wstring(L"\\prod_{") + P0 + L"}";
            static const NodeSpec s{1, t, false, GraphFunction::None};
            return s;
        }
        case Kind::Ln: {
            static const std::wstring t = std::wstring(L"\\ln{\\left(") + P0 + L"\\right)} ";
            static const NodeSpec s{1, t, false, GraphFunction::Ln};
            return s;
        }
        case Kind::Log: {
            // No .math template in legacy's `log` class (latex.py 543-561)
            // -- ungraphable there too, so GraphFunction::None is not a
            // regression, just made explicit/typed instead of an uncaught
            // AttributeError.
            static const std::wstring t = std::wstring(L"\\log_{") + P0 + L"}{\\left(" + P1 + L"\\right)}";
            static const NodeSpec s{2, t, false, GraphFunction::None};
            return s;
        }
        case Kind::Exp: {
            static const std::wstring t = std::wstring(L"\\exp{\\left(") + P0 + L"\\right)} ";
            static const NodeSpec s{1, t, false, GraphFunction::Exp};
            return s;
        }
        case Kind::EulerPower: {
            static const std::wstring t = std::wstring(L"e^{") + P0 + L"} ";
            static const NodeSpec s{1, t, false, GraphFunction::Exp};
            return s;
        }
        case Kind::Cos: {
            static const std::wstring t = std::wstring(L"\\cos{\\left(") + P0 + L"\\right)} ";
            static const NodeSpec s{1, t, false, GraphFunction::Cos};
            return s;
        }
        case Kind::Sin: {
            static const std::wstring t = std::wstring(L"\\sin{\\left(") + P0 + L"\\right)} ";
            static const NodeSpec s{1, t, false, GraphFunction::Sin};
            return s;
        }
        case Kind::Sinc: {
            static const std::wstring t = std::wstring(L"\\sin c{\\left(") + P0 + L"\\right)} ";
            static const NodeSpec s{1, t, false, GraphFunction::Sinc};
            return s;
        }
        case Kind::Tan: {
            static const std::wstring t = std::wstring(L"\\tan{\\left(") + P0 + L"\\right)} ";
            static const NodeSpec s{1, t, false, GraphFunction::Tan};
            return s;
        }
        case Kind::ArcCos: {
            static const std::wstring t = std::wstring(L"\\arccos{\\left(") + P0 + L"\\right)} ";
            static const NodeSpec s{1, t, false, GraphFunction::ArcCos};
            return s;
        }
        case Kind::ArcSin: {
            static const std::wstring t = std::wstring(L"\\arcsin{\\left(") + P0 + L"\\right)} ";
            static const NodeSpec s{1, t, false, GraphFunction::ArcSin};
            return s;
        }
        case Kind::ArcTan: {
            static const std::wstring t = std::wstring(L"\\arctan{\\left(") + P0 + L"\\right)} ";
            static const NodeSpec s{1, t, false, GraphFunction::ArcTan};
            return s;
        }
        case Kind::Vector: {
            static const std::wstring t = std::wstring(L"\\overrightarrow{") + P0 + L"} ";
            static const NodeSpec s{1, t, false, GraphFunction::None};
            return s;
        }
        case Kind::Limit: {
            static const std::wstring t = std::wstring(L"\\lim{\\left(") + P0 + L"\\right)} ";
            static const NodeSpec s{1, t, false, GraphFunction::None};
            return s;
        }
        case Kind::LimitBounded: {
            static const std::wstring t = std::wstring(L"\\lim_{") + P0 + L"}{\\left(" + P1 + L"\\right)}";
            static const NodeSpec s{2, t, false, GraphFunction::None};
            return s;
        }
        case Kind::Matrix: {
            // arity -1: slot count is rows*cols, only known per-instance.
            // See Node::makeMatrix, which also builds the per-instance
            // template (this legacy class was actually dead/broken code --
            // see Node.cpp).
            static const NodeSpec s{-1, L"", false, GraphFunction::None};
            return s;
        }
        case Kind::System: {
            static const NodeSpec s{-1, L"", false, GraphFunction::None};
            return s;
        }
        case Kind::Binom: {
            static const std::wstring t = std::wstring(L"\\binom{") + P0 + L"}{" + P1 + L"}";
            static const NodeSpec s{2, t, false, GraphFunction::None};
            return s;
        }
        case Kind::Norm: {
            static const std::wstring t = std::wstring(L"\\left\\| ") + P0 + L" \\right\\|";
            static const NodeSpec s{1, t, false, GraphFunction::Abs};
            return s;
        }
        case Kind::NormDouble: {
            static const std::wstring t = std::wstring(L"\\left\\Vert ") + P0 + L" \\right\\Vert";
            static const NodeSpec s{1, t, false, GraphFunction::Abs};
            return s;
        }
    }
    assert(false && "unreachable: NodeCatalog::spec given an out-of-range Kind");
    static const NodeSpec fallback{0, L"", false, GraphFunction::None};
    return fallback;
}

} // namespace mathclav::core::ast
