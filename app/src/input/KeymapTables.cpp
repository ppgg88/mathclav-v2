#include "input/KeymapTables.h"

namespace mathclav::app::input {

using mathclav::core::ast::Kind;

namespace {

Node sym(const wchar_t* s) {
    return Node::symbol(s);
}

Node comp(Kind k) {
    return Node::composite(k);
}

} // namespace

// Ported verbatim from index.py's self.corespondance[2] (lines 313-349),
// which is what action() actually dispatches through -- not
// self.view_corespondance (lines 198-234), used only by the legacy help
// screen and inconsistent with it in at least one spot ('K': the help
// table shows a thin space, but the real dispatch table cycles 'k'/'n').
std::vector<Node> mathModeCandidates(QChar letterUpper) {
    switch (letterUpper.toLatin1()) {
        case 'A': return {sym(L"\\Rightarrow "), sym(L"\\Leftarrow ")};
        case 'B': return {comp(Kind::Binom)};
        case 'C': return {sym(L"\\in "), sym(L"\\supset "), sym(L"\\subset "), sym(L"\\supseteq "), sym(L"\\subseteq ")};
        case 'D': return {comp(Kind::EulerPower), comp(Kind::Exp), comp(Kind::Ln), comp(Kind::Log)};
        case 'E': return {sym(L"\\Longleftrightarrow "), sym(L"\\Leftrightarrow ")};
        case 'F': return {sym(L"f"), sym(L"g"), sym(L"h"), sym(L"u")};
        case 'G': return {sym(L"\\rightarrow "), sym(L"\\leftarrow "), sym(L"\\leftrightarrow ")};
        // 'H' intentionally absent: KeyDispatcher intercepts it before consulting this table.
        case 'I':
            return {comp(Kind::Integral),        comp(Kind::IntegralBoundOnly), comp(Kind::ContourIntegral),
                    comp(Kind::DoubleIntegral),   comp(Kind::DoubleContourIntegral),
                    comp(Kind::TripleIntegral),   comp(Kind::TripleContourIntegral)};
        case 'J': return {sym(L"\\imath "), sym(L"\\jmath "), sym(L"\\Re "), sym(L"\\Im ")};
        case 'K': return {sym(L"k"), sym(L"n")};
        case 'L': return {comp(Kind::Ln), comp(Kind::Log), comp(Kind::EulerPower), comp(Kind::Exp)};
        case 'M': return {comp(Kind::LimitBounded), comp(Kind::Limit)};
        case 'N': return {sym(L"n"), sym(L"k"), sym(L"l")};
        case 'O': return {comp(Kind::Sum), comp(Kind::SumBoundOnly), sym(L"\\sum ")};
        case 'P': return {comp(Kind::Product), comp(Kind::ProductBoundOnly), sym(L"\\prod ")};
        case 'Q': return {comp(Kind::Fraction)};
        case 'R':
            return {sym(L"\\mathbb{R} "), sym(L"\\mathbb{C} "), sym(L"\\mathbb{N} "), sym(L"\\mathbb{Z} "),
                    sym(L"\\mathbb{Q} ")};
        case 'S': return {comp(Kind::Sqrt), comp(Kind::SqrtN)};
        case 'T': return {comp(Kind::Cos), comp(Kind::Sin), comp(Kind::Tan), comp(Kind::Sinc)};
        case 'U': return {sym(L"\\cup "), sym(L"\\cap "), comp(Kind::Union), comp(Kind::Intersection)};
        case 'V': return {comp(Kind::Vector)};
        case 'W': return {sym(L"\\forall "), sym(L"\\exists ")};
        case 'X': return {sym(L"x"), sym(L"y"), sym(L"z")};
        case 'Y': return {comp(Kind::ArcCos), comp(Kind::ArcSin), comp(Kind::ArcTan)};
        case 'Z': return {sym(L"\\infty "), sym(L"+\\infty "), sym(L"-\\infty "), sym(L"\\pm\\infty ")};
        default: return {};
    }
}

// index.py self.corespondance[0], lines 239-264.
Node greekUppercase(QChar letterUpper) {
    switch (letterUpper.toLatin1()) {
        case 'A': return sym(L"A");
        case 'B': return sym(L"B");
        case 'C': return sym(L"\\Theta ");
        case 'D': return sym(L"\\Delta ");
        case 'E': return sym(L"E");
        case 'F': return sym(L"\\Phi ");
        case 'G': return sym(L"\\Gamma ");
        case 'H': return sym(L"H");
        case 'I': return sym(L"I");
        case 'J': return sym(L"\\Omega ");
        case 'K': return sym(L"K");
        case 'L': return sym(L"\\Lambda ");
        case 'M': return sym(L"M");
        case 'N': return sym(L"N");
        case 'O': return sym(L"O");
        case 'P': return sym(L"\\Pi ");
        case 'Q': return sym(L"Q");
        case 'R': return sym(L"P");
        case 'S': return sym(L"\\Sigma ");
        case 'T': return sym(L"T");
        case 'U': return sym(L"U");
        case 'V': return sym(L"X");
        case 'W': return sym(L"\\Psi ");
        case 'X': return sym(L"\\Xi ");
        case 'Y': return sym(L"Y");
        case 'Z': return sym(L"Z");
        default: return sym(L"");
    }
}

// index.py self.corespondance[1], lines 276-301.
Node greekLowercase(QChar letterUpper) {
    switch (letterUpper.toLatin1()) {
        case 'A': return sym(L"\\alpha ");
        case 'B': return sym(L"\\beta ");
        case 'C': return sym(L"\\theta ");
        case 'D': return sym(L"\\delta ");
        case 'E': return sym(L"\\epsilon ");
        case 'F': return sym(L"\\varphi ");
        case 'G': return sym(L"\\gamma ");
        case 'H': return sym(L"\\eta ");
        case 'I': return sym(L"\\iota ");
        case 'J': return sym(L"\\omega ");
        case 'K': return sym(L"\\kappa ");
        case 'L': return sym(L"\\lambda ");
        case 'M': return sym(L"\\mu ");
        case 'N': return sym(L"\\nu ");
        case 'O': return sym(L"o ");
        case 'P': return sym(L"\\pi ");
        case 'Q': return sym(L"q");
        case 'R': return sym(L"\\rho ");
        case 'S': return sym(L"\\sigma ");
        case 'T': return sym(L"\\tau ");
        case 'U': return sym(L"u");
        case 'V': return sym(L"\\chi ");
        case 'W': return sym(L"\\psi ");
        case 'X': return sym(L"\\xi ");
        case 'Y': return sym(L"\\upsilon ");
        case 'Z': return sym(L"\\zeta ");
        default: return sym(L"");
    }
}

} // namespace mathclav::app::input
