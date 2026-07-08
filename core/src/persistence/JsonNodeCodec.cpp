#include "mathclav/core/persistence/JsonNodeCodec.h"

#include <QHash>

#include <algorithm>

namespace mathclav::core::persistence {

namespace {

using ast::Kind;

// Exhaustive over Kind (-Wswitch-enum, see cmake/Warnings.cmake): a new
// Kind that isn't given a tag here fails the build instead of silently
// round-tripping as garbage.
QString kindToString(Kind kind) {
    switch (kind) {
        case Kind::Symbol: return QStringLiteral("Symbol");
        case Kind::Sqrt: return QStringLiteral("Sqrt");
        case Kind::SqrtN: return QStringLiteral("SqrtN");
        case Kind::Power: return QStringLiteral("Power");
        case Kind::Subscript: return QStringLiteral("Subscript");
        case Kind::Fraction: return QStringLiteral("Fraction");
        case Kind::Text: return QStringLiteral("Text");
        case Kind::Integral: return QStringLiteral("Integral");
        case Kind::IntegralBoundOnly: return QStringLiteral("IntegralBoundOnly");
        case Kind::ContourIntegral: return QStringLiteral("ContourIntegral");
        case Kind::DoubleIntegral: return QStringLiteral("DoubleIntegral");
        case Kind::DoubleContourIntegral: return QStringLiteral("DoubleContourIntegral");
        case Kind::TripleIntegral: return QStringLiteral("TripleIntegral");
        case Kind::TripleContourIntegral: return QStringLiteral("TripleContourIntegral");
        case Kind::Paren: return QStringLiteral("Paren");
        case Kind::ParenSquare: return QStringLiteral("ParenSquare");
        case Kind::ParenCurly: return QStringLiteral("ParenCurly");
        case Kind::Union: return QStringLiteral("Union");
        case Kind::Intersection: return QStringLiteral("Intersection");
        case Kind::Sum: return QStringLiteral("Sum");
        case Kind::SumBoundOnly: return QStringLiteral("SumBoundOnly");
        case Kind::Product: return QStringLiteral("Product");
        case Kind::ProductBoundOnly: return QStringLiteral("ProductBoundOnly");
        case Kind::Ln: return QStringLiteral("Ln");
        case Kind::Log: return QStringLiteral("Log");
        case Kind::Exp: return QStringLiteral("Exp");
        case Kind::EulerPower: return QStringLiteral("EulerPower");
        case Kind::Cos: return QStringLiteral("Cos");
        case Kind::Sin: return QStringLiteral("Sin");
        case Kind::Sinc: return QStringLiteral("Sinc");
        case Kind::Tan: return QStringLiteral("Tan");
        case Kind::ArcCos: return QStringLiteral("ArcCos");
        case Kind::ArcSin: return QStringLiteral("ArcSin");
        case Kind::ArcTan: return QStringLiteral("ArcTan");
        case Kind::Vector: return QStringLiteral("Vector");
        case Kind::Limit: return QStringLiteral("Limit");
        case Kind::LimitBounded: return QStringLiteral("LimitBounded");
        case Kind::Matrix: return QStringLiteral("Matrix");
        case Kind::System: return QStringLiteral("System");
        case Kind::Binom: return QStringLiteral("Binom");
        case Kind::Norm: return QStringLiteral("Norm");
        case Kind::NormDouble: return QStringLiteral("NormDouble");
    }
    throw PersistenceError("internal: kindToString given an out-of-range Kind");
}

Kind kindFromString(const QString& s) {
    static const QHash<QString, Kind> kByName = [] {
        QHash<QString, Kind> h;
        // Populated by round-tripping every enumerator through
        // kindToString, so this table can never drift out of sync with it.
        for (int i = 0; i < ast::kNodeKindCount; ++i) {
            const auto kind = static_cast<Kind>(i);
            h.insert(kindToString(kind), kind);
        }
        return h;
    }();
    const auto it = kByName.find(s);
    if (it == kByName.end()) {
        throw PersistenceError("unknown node kind in JSON: " + s.toStdString());
    }
    return it.value();
}

} // namespace

QJsonObject nodeToJson(const ast::Node& node) {
    QJsonObject obj;
    obj[QStringLiteral("k")] = kindToString(node.kind);

    if (node.kind == Kind::Symbol) {
        obj[QStringLiteral("t")] = QString::fromStdWString(node.literal);
        return obj;
    }

    if (node.grid.has_value()) {
        obj[QStringLiteral("rows")] = node.grid->rows;
        obj[QStringLiteral("cols")] = node.grid->cols;
    }

    QJsonArray argsArray;
    for (const ast::Container& slot : node.args) {
        argsArray.append(containerToJson(slot));
    }
    obj[QStringLiteral("args")] = argsArray;
    return obj;
}

ast::Node nodeFromJson(const QJsonObject& obj) {
    if (!obj.contains(QStringLiteral("k")) || !obj.value(QStringLiteral("k")).isString()) {
        throw PersistenceError("node JSON missing string \"k\"");
    }
    const Kind kind = kindFromString(obj.value(QStringLiteral("k")).toString());

    if (kind == Kind::Symbol) {
        return ast::Node::symbol(obj.value(QStringLiteral("t")).toString().toStdWString());
    }

    ast::Node node = (kind == Kind::Matrix || kind == Kind::System)
                          ? [&] {
                                const int rows = obj.value(QStringLiteral("rows")).toInt(-1);
                                const int cols = obj.value(QStringLiteral("cols")).toInt(-1);
                                if (rows < 0 || cols < 0) {
                                    throw PersistenceError("Matrix/System JSON missing rows/cols");
                                }
                                return kind == Kind::Matrix ? ast::Node::makeMatrix(rows, cols)
                                                             : ast::Node::makeSystem(rows, cols);
                            }()
                          : ast::Node::composite(kind);

    const QJsonArray argsArray = obj.value(QStringLiteral("args")).toArray();
    const std::size_t count = std::min(static_cast<std::size_t>(argsArray.size()), node.args.size());
    for (std::size_t i = 0; i < count; ++i) {
        node.args[i] = containerFromJson(argsArray[static_cast<int>(i)].toArray());
    }
    return node;
}

QJsonArray containerToJson(const ast::Container& container) {
    QJsonArray array;
    for (const ast::Node& node : container) {
        array.append(nodeToJson(node));
    }
    return array;
}

ast::Container containerFromJson(const QJsonArray& array) {
    ast::Container container;
    container.reserve(static_cast<std::size_t>(array.size()));
    for (const QJsonValue& value : array) {
        if (!value.isObject()) {
            throw PersistenceError("container JSON element is not an object");
        }
        container.push_back(nodeFromJson(value.toObject()));
    }
    return container;
}

} // namespace mathclav::core::persistence
