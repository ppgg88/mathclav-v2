#pragma once

#include "mathclav/core/ast/Node.h"

#include <QString>

namespace mathclav::core::session {

// Autosaves/restores the current document across app restarts. Replaces
// legacy's `pickle.load(open(data_path + r"\historique\last.pkl", "rb"))`
// (index.py 102) -- a deserialization-of-untrusted-data risk -- with plain
// JSON. The path is passed in explicitly rather than resolved internally
// (QStandardPaths::AppDataLocation is the app layer's job, MainWindow.cpp)
// so this stays testable against a QTemporaryDir with no real app-data
// directory involved.

// True if `path` was written and parsed successfully; on any failure
// (missing file, malformed JSON, schema mismatch) returns false and
// leaves `outDocument` untouched, matching legacy's blanket
// `try: ... except: self.result = [mathObject()]` fallback (index.py
// 101-107) -- but as a checked return instead of a silently-empty
// document indistinguishable from "user cleared everything".
[[nodiscard]] bool loadSession(const QString& path, ast::Container& outDocument);

// Returns false if the file couldn't be written (e.g. the directory
// doesn't exist and couldn't be created).
[[nodiscard]] bool saveSession(const QString& path, const ast::Container& document);

} // namespace mathclav::core::session
