#pragma once

#include "mathclav/core/ast/Node.h"

#include <QJsonArray>
#include <QJsonObject>

#include <stdexcept>

namespace mathclav::core::persistence {

// Thrown when decoding a Node/Container from JSON that doesn't match the
// schema (unknown "k" kind tag, missing required field). Legacy has no
// equivalent check at all: `pickle.load` on a corrupt/foreign file is a
// straight deserialization-time code-execution risk (index.py 102,
// historique.py) rather than a validated, catchable parse error -- this
// exception is what replaces that entire class of risk. Callers (Session/
// History/Settings stores) catch it and fall back to an empty/default
// value, matching legacy's `try: ... except: <default>` pattern around a
// missing settings.json (index.py 91-98) but now also covering a
// corrupted one.
class PersistenceError : public std::runtime_error {
public:
    explicit PersistenceError(const std::string& message) : std::runtime_error(message) {}
};

// New JSON schema (not a port of any legacy format -- legacy used pickle
// per-object, historique.py/index.py): a node is `{"k": "<Kind>"}` for a
// composite, plus `"t"` (the literal) for Kind::Symbol, plus `"rows"`/
// `"cols"` for Kind::Matrix/Kind::System, plus `"args"` (an array of
// Containers, i.e. arrays of nodes) for every composite kind. The JSON key
// is "args" -- matching Node::args, not the "slots" name the architecture
// doc's very first sketch used before that field was renamed to dodge
// Qt's `slots` macro (see MainWindow.cpp's git history / the plan doc's
// Phase 2 notes).
[[nodiscard]] QJsonObject nodeToJson(const ast::Node& node);
[[nodiscard]] ast::Node nodeFromJson(const QJsonObject& obj);

[[nodiscard]] QJsonArray containerToJson(const ast::Container& container);
[[nodiscard]] ast::Container containerFromJson(const QJsonArray& array);

} // namespace mathclav::core::persistence
