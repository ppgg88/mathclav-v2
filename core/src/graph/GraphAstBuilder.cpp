#include "mathclav/core/graph/GraphAstBuilder.h"

#include <cwctype>

namespace mathclav::core::graph {

namespace {

using ast::Container;
using ast::Kind;
using ast::Node;

std::wstring trimmed(std::wstring s) {
    while (!s.empty() && s.back() == L' ') s.pop_back();
    while (!s.empty() && s.front() == L' ') s.erase(s.begin());
    return s;
}

GraphNode numberNode(double v) {
    GraphNode n;
    n.kind = GraphKind::Number;
    n.numberValue = v;
    return n;
}

GraphNode varNode(std::wstring name) {
    GraphNode n;
    n.kind = GraphKind::Variable;
    n.variableName = std::move(name);
    return n;
}

GraphNode callNode(Func f, GraphNode arg) {
    GraphNode n;
    n.kind = GraphKind::Call;
    n.func = f;
    n.children.push_back(std::move(arg));
    return n;
}

GraphNode binNode(GraphKind k, GraphNode a, GraphNode b) {
    GraphNode n;
    n.kind = k;
    n.children.push_back(std::move(a));
    n.children.push_back(std::move(b));
    return n;
}

GraphNode negNode(GraphNode a) {
    GraphNode n;
    n.kind = GraphKind::Neg;
    n.children.push_back(std::move(a));
    return n;
}

// One token in the flattened, implicit-multiplication-not-yet-resolved
// stream: either a fully-built operand (a number, variable, or the result
// of recursing into a composite/paren) or an explicit binary operator.
struct Token {
    bool isOperator;
    wchar_t op = L'\0'; // '+','-','*','/','^' (only if isOperator)
    GraphNode operand;   // only if !isOperator
};

[[noreturn]] void throwUngraphable() {
    // Every kind reaching this has no graphStr()/`.math` in legacy either
    // (latex.py): Power, Subscript, Text, SqrtN, the bound-only/contour/
    // multi integral variants, ParenSquare, ParenCurly, Union,
    // Intersection, the Product family, Log, Vector, Limit family, Matrix,
    // System, Binom. There legacy raises an uncaught AttributeError; here
    // it's a typed, catchable exception (see GrapherDialog, app layer).
    throw GraphError("this construct cannot be graphed");
}

// Parses the "from" slot of a Sum (index.py sum.graphStr, latex.py
// 444-455): the bound variable is the *first character* of the slot's
// content, and the start value accumulates from the digits that follow an
// '='. Ported as a walk over the slot's own Symbol children instead of a
// string scan, but kept just as loose: anything after '=' that isn't a
// digit is silently ignored, matching legacy's `try: ... except: pass`.
void parseSumFrom(const Container& fromSlot, std::wstring& varOut, double& startOut) {
    varOut.clear();
    bool seenEquals = false;
    bool anyDigit = false;
    double value = 0;
    for (const Node& n : fromSlot) {
        if (n.kind != Kind::Symbol) continue;
        const std::wstring t = trimmed(n.literal);
        if (t.empty()) continue;
        if (varOut.empty() && !seenEquals) {
            varOut = std::wstring(1, t.front());
        }
        for (wchar_t c : t) {
            if (c == L'=') {
                seenEquals = true;
            } else if (seenEquals && std::iswdigit(static_cast<wint_t>(c))) {
                value = value * 10 + static_cast<double>(c - L'0');
                anyDigit = true;
            }
        }
    }
    startOut = anyDigit ? value : 0;
}

// Parses the "to" slot of a Sum: must be a plain non-negative integer
// (index.py: `try: max = int(n) except: pass`, leaving max at 0 on any
// non-numeric content).
double parseSumTo(const Container& toSlot) {
    double value = 0;
    bool any = false;
    for (const Node& n : toSlot) {
        if (n.kind != Kind::Symbol) continue;
        for (wchar_t c : n.literal) {
            if (std::iswdigit(static_cast<wint_t>(c))) {
                value = value * 10 + static_cast<double>(c - L'0');
                any = true;
            } else if (c != L' ') {
                return 0; // matches legacy's int(n) throwing on anything non-numeric
            }
        }
    }
    return any ? value : 0;
}

// The integral's dummy-variable slot (content[3], index.py) is expected to
// render to a single variable name.
std::wstring parseVariableSlot(const Container& slot) {
    std::wstring name;
    for (const Node& n : slot) {
        if (n.kind == Kind::Symbol) name += n.literal;
    }
    return trimmed(name);
}

GraphNode buildExpr(const Container& container); // used by buildComposite below

// Exhaustive over Kind (-Wswitch-enum, see cmake/Warnings.cmake): adding a
// new Kind without deciding here whether/how it graphs is a build failure,
// not a silent gap.
GraphNode buildComposite(const Node& node) {
    switch (node.kind) {
        case Kind::Symbol:
            throw GraphError("internal: buildComposite given a Symbol"); // tokenize() handles these directly
        case Kind::Sqrt: return callNode(Func::Sqrt, buildExpr(node.args[0]));
        case Kind::SqrtN: throwUngraphable();
        case Kind::Power: throwUngraphable();
        case Kind::Subscript: throwUngraphable();
        case Kind::Fraction: return binNode(GraphKind::Div, buildExpr(node.args[0]), buildExpr(node.args[1]));
        case Kind::Text: throwUngraphable();
        case Kind::Integral: {
            GraphNode n;
            n.kind = GraphKind::Integral;
            n.variableName = parseVariableSlot(node.args[3]);
            n.children.push_back(buildExpr(node.args[0]));
            n.children.push_back(buildExpr(node.args[1]));
            n.children.push_back(buildExpr(node.args[2]));
            return n;
        }
        case Kind::IntegralBoundOnly: throwUngraphable();
        case Kind::ContourIntegral: throwUngraphable();
        case Kind::DoubleIntegral: throwUngraphable();
        case Kind::DoubleContourIntegral: throwUngraphable();
        case Kind::TripleIntegral: throwUngraphable();
        case Kind::TripleContourIntegral: throwUngraphable();
        case Kind::Paren: return buildExpr(node.args[0]);
        case Kind::ParenSquare: throwUngraphable();
        case Kind::ParenCurly: throwUngraphable();
        case Kind::Union: throwUngraphable();
        case Kind::Intersection: throwUngraphable();
        case Kind::Sum: {
            std::wstring var;
            double start = 0;
            parseSumFrom(node.args[0], var, start);
            GraphNode n;
            n.kind = GraphKind::Sum;
            n.variableName = var;
            n.children.push_back(numberNode(start));
            n.children.push_back(numberNode(parseSumTo(node.args[1])));
            n.children.push_back(buildExpr(node.args[2]));
            return n;
        }
        case Kind::SumBoundOnly: throwUngraphable();
        case Kind::Product: throwUngraphable();
        case Kind::ProductBoundOnly: throwUngraphable();
        case Kind::Ln: return callNode(Func::Ln, buildExpr(node.args[0]));
        case Kind::Log: throwUngraphable(); // no graphStr()/`.math` in legacy's log class either
        case Kind::Exp: return callNode(Func::Exp, buildExpr(node.args[0]));
        case Kind::EulerPower: return callNode(Func::Exp, buildExpr(node.args[0])); // e^{a} means exp(a)
        case Kind::Cos: return callNode(Func::Cos, buildExpr(node.args[0]));
        case Kind::Sin: return callNode(Func::Sin, buildExpr(node.args[0]));
        case Kind::Sinc: return callNode(Func::Sinc, buildExpr(node.args[0]));
        case Kind::Tan: return callNode(Func::Tan, buildExpr(node.args[0]));
        case Kind::ArcCos: return callNode(Func::ArcCos, buildExpr(node.args[0]));
        case Kind::ArcSin: return callNode(Func::ArcSin, buildExpr(node.args[0]));
        case Kind::ArcTan: return callNode(Func::ArcTan, buildExpr(node.args[0]));
        case Kind::Vector: throwUngraphable();
        case Kind::Limit: throwUngraphable();
        case Kind::LimitBounded: throwUngraphable();
        case Kind::Matrix: throwUngraphable();
        case Kind::System: throwUngraphable();
        case Kind::Binom: throwUngraphable();
        case Kind::Norm: return callNode(Func::Abs, buildExpr(node.args[0]));
        case Kind::NormDouble: return callNode(Func::Abs, buildExpr(node.args[0]));
    }
    throw GraphError("internal: buildComposite given an out-of-range Kind");
}

// Recognizes the Symbol literals (trimmed) that mean something other than
// "insert this text as a variable name": arithmetic operators, pi, and the
// thin space the space-bar multi-choice trigger inserts (`\: `), which
// legacy's mathSymbol.graphStr() strips to nothing. Appends 0 or 1 tokens
// to `out` and returns whether the literal was recognized at all.
bool tokenizeSymbol(const std::wstring& literalTrimmed, std::vector<Token>& out) {
    // The '*'-family and '+'/'-' families (index.py's multi-choice
    // candidates for those keys) are all treated as the operator they
    // visually represent. Legacy's mathSymbol.graphStr() only special-
    // cases \ast/\times (not \cdot/\odot/\otimes/\wedge) and bare +/-
    // (not \pm/\oplus/\mp/\ominus), so cycling to those other candidates
    // silently fails to graph there -- treating the whole family
    // uniformly is a deliberate, low-risk improvement over that gap, not
    // a faithfulness regression (see the plan doc's Phase 4 notes).
    static const std::vector<std::wstring> kTimes = {L"*",      L"\\times",  L"\\cdot",
                                                       L"\\wedge", L"\\ast", L"\\odot", L"\\otimes"};
    static const std::vector<std::wstring> kPlus = {L"+", L"\\pm", L"\\oplus"};
    static const std::vector<std::wstring> kMinus = {L"-", L"\\mp", L"\\ominus"};

    if (literalTrimmed == L"\\:") {
        return true; // thin space: contributes no token at all
    }
    if (literalTrimmed == L"\\pi") {
        out.push_back(Token{false, L'\0', numberNode(3.14159265)});
        return true;
    }
    if (literalTrimmed == L"/" || literalTrimmed == L"\\div") {
        out.push_back(Token{true, L'/', {}});
        return true;
    }
    if (literalTrimmed == L"^") {
        out.push_back(Token{true, L'^', {}});
        return true;
    }
    for (const auto& s : kTimes) {
        if (literalTrimmed == s) {
            out.push_back(Token{true, L'*', {}});
            return true;
        }
    }
    for (const auto& s : kPlus) {
        if (literalTrimmed == s) {
            out.push_back(Token{true, L'+', {}});
            return true;
        }
    }
    for (const auto& s : kMinus) {
        if (literalTrimmed == s) {
            out.push_back(Token{true, L'-', {}});
            return true;
        }
    }
    return false;
}

std::vector<Token> tokenize(const Container& container) {
    std::vector<Token> tokens;
    std::wstring digitRun; // accumulates consecutive digits/decimal point

    auto flushDigits = [&] {
        if (digitRun.empty()) return;
        try {
            tokens.push_back(Token{false, L'\0', numberNode(std::stod(digitRun))});
        } catch (const std::exception&) {
            throw GraphError("malformed number");
        }
        digitRun.clear();
    };

    for (const Node& node : container) {
        if (node.kind == Kind::Symbol) {
            const std::wstring t = trimmed(node.literal);
            if (t.size() == 1 && (std::iswdigit(static_cast<wint_t>(t[0])) || t[0] == L'.')) {
                digitRun += t;
                continue;
            }
            flushDigits();
            std::vector<Token> produced;
            if (tokenizeSymbol(t, produced)) {
                for (Token& tok : produced) tokens.push_back(std::move(tok));
                continue;
            }
            if (t.empty()) {
                continue;
            }
            tokens.push_back(Token{false, L'\0', varNode(t)});
            continue;
        }
        flushDigits();
        tokens.push_back(Token{false, L'\0', buildComposite(node)});
    }
    flushDigits();
    return tokens;
}

// Inserts an implicit '*' between any two consecutive operand tokens: the
// token-level generalization of graph.py's character-scan heuristic
// (217-222, 270-279), which special-cased digit/æ/')' immediately followed
// by a letter/backslash/'('. Any operand directly adjacent to another
// operand (no explicit operator token between them) means the same thing
// regardless of what kind of operand each side is.
std::vector<Token> insertImplicitMultiplication(std::vector<Token> tokens) {
    std::vector<Token> out;
    for (std::size_t i = 0; i < tokens.size(); ++i) {
        if (i > 0 && !tokens[i].isOperator && !tokens[i - 1].isOperator) {
            out.push_back(Token{true, L'*', {}});
        }
        out.push_back(std::move(tokens[i]));
    }
    return out;
}

// Small recursive-descent expression parser over the resolved token
// stream. Precedence, loosest to tightest: + - (left-assoc), * / (left-
// assoc, includes the implicit-multiplication tokens above), unary +/-,
// ^ (right-assoc), atoms.
class Parser {
public:
    explicit Parser(const std::vector<Token>& tokens) : tokens_(tokens) {}

    GraphNode parseExpression() {
        if (tokens_.empty()) {
            throw GraphError("empty expression");
        }
        GraphNode result = parseAdditive();
        if (pos_ != tokens_.size()) {
            throw GraphError("unexpected token in expression");
        }
        return result;
    }

private:
    const std::vector<Token>& tokens_;
    std::size_t pos_ = 0;

    [[nodiscard]] bool atEnd() const { return pos_ >= tokens_.size(); }
    [[nodiscard]] bool isOp(wchar_t c) const {
        return !atEnd() && tokens_[pos_].isOperator && tokens_[pos_].op == c;
    }

    GraphNode parseAdditive() {
        GraphNode lhs = parseMultiplicative();
        while (isOp(L'+') || isOp(L'-')) {
            const wchar_t op = tokens_[pos_].op;
            ++pos_;
            GraphNode rhs = parseMultiplicative();
            lhs = binNode(op == L'+' ? GraphKind::Add : GraphKind::Sub, std::move(lhs), std::move(rhs));
        }
        return lhs;
    }

    GraphNode parseMultiplicative() {
        GraphNode lhs = parseUnary();
        while (isOp(L'*') || isOp(L'/')) {
            const wchar_t op = tokens_[pos_].op;
            ++pos_;
            GraphNode rhs = parseUnary();
            lhs = binNode(op == L'*' ? GraphKind::Mul : GraphKind::Div, std::move(lhs), std::move(rhs));
        }
        return lhs;
    }

    GraphNode parseUnary() {
        if (isOp(L'-')) {
            ++pos_;
            return negNode(parseUnary());
        }
        if (isOp(L'+')) {
            ++pos_;
            return parseUnary();
        }
        return parsePower();
    }

    GraphNode parsePower() {
        GraphNode base = parseAtom();
        if (isOp(L'^')) {
            ++pos_;
            GraphNode exponent = parseUnary(); // right-assoc, allows e.g. x^-1
            return binNode(GraphKind::Pow, std::move(base), std::move(exponent));
        }
        return base;
    }

    GraphNode parseAtom() {
        if (atEnd() || tokens_[pos_].isOperator) {
            throw GraphError("expected a value");
        }
        return tokens_[pos_++].operand;
    }
};

GraphNode buildExpr(const Container& container) {
    std::vector<Token> tokens = insertImplicitMultiplication(tokenize(container));
    Parser parser(tokens);
    return parser.parseExpression();
}

} // namespace

GraphNode build(const ast::Container& container) {
    return buildExpr(container);
}

} // namespace mathclav::core::graph
