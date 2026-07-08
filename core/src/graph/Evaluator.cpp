#include "mathclav/core/graph/Evaluator.h"

#include <cmath>

namespace mathclav::core::graph {

namespace {

double callFunc(Func f, double x) {
    switch (f) {
        case Func::Sin: return std::sin(x);
        case Func::Cos: return std::cos(x);
        case Func::Tan: return std::tan(x);
        case Func::Sinc: return std::sin(x) / x; // matches legacy's unguarded mt.sin(a)/a
        case Func::ArcSin: return std::asin(x);
        case Func::ArcCos: return std::acos(x);
        case Func::ArcTan: return std::atan(x);
        case Func::Ln: return std::log(x);
        case Func::Exp: return std::exp(x);
        case Func::Sqrt: return std::sqrt(x);
        case Func::Abs: return std::fabs(x);
    }
    return std::nan("");
}

double evalSum(const GraphNode& node, const Env& env) {
    const double start = node.children[0].numberValue;
    const double end = node.children[1].numberValue;
    const GraphNode& body = node.children[2];
    Env local = env;
    double total = 0;
    for (double i = start; i <= end; i += 1.0) {
        local[node.variableName] = i;
        total += eval(body, local);
    }
    return total;
}

double evalIntegral(const GraphNode& node, const Env& env) {
    const double lo = eval(node.children[0], env);
    const double hi = eval(node.children[1], env);
    const GraphNode& body = node.children[2];
    constexpr double kStep = 0.001; // matches integral.graphStr's hardcoded e=0.001
    const int n = static_cast<int>((hi - lo) / kStep);
    Env local = env;
    double total = 0;
    for (int j = 0; j <= n; ++j) {
        const double x = lo + static_cast<double>(j) * kStep;
        local[node.variableName] = x;
        total += eval(body, local);
    }
    return total * kStep;
}

} // namespace

double eval(const GraphNode& node, const Env& env) {
    switch (node.kind) {
        case GraphKind::Number:
            return node.numberValue;
        case GraphKind::Variable: {
            const auto it = env.find(node.variableName);
            if (it == env.end()) {
                throw GraphError("undefined variable in expression");
            }
            return it->second;
        }
        case GraphKind::Add: return eval(node.children[0], env) + eval(node.children[1], env);
        case GraphKind::Sub: return eval(node.children[0], env) - eval(node.children[1], env);
        case GraphKind::Mul: return eval(node.children[0], env) * eval(node.children[1], env);
        case GraphKind::Div: return eval(node.children[0], env) / eval(node.children[1], env);
        case GraphKind::Pow: return std::pow(eval(node.children[0], env), eval(node.children[1], env));
        case GraphKind::Neg: return -eval(node.children[0], env);
        case GraphKind::Call: return callFunc(node.func, eval(node.children[0], env));
        case GraphKind::Sum: return evalSum(node, env);
        case GraphKind::Integral: return evalIntegral(node, env);
    }
    throw GraphError("internal: eval given an unknown GraphKind");
}

} // namespace mathclav::core::graph
