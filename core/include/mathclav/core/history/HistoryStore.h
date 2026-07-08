#pragma once

#include "mathclav/core/ast/Node.h"

#include <QString>

#include <vector>

namespace mathclav::core::history {

// A single saved expression, replacing legacy's one-pickle-file-per-entry
// plus a separate counter file plus `os.rename`-based renumbering on
// delete (historique.py 41-45, 151-169) -- both a pickle-deserialization
// risk and a renumbering-bug risk. Here the whole list is one JSON array;
// `id` is a stable identifier so deleting entry N doesn't require shifting
// every later entry's identity.
struct HistoryEntry {
    QString id;
    QString name;
    ast::Container document;
};

// Returns an empty list on any read failure (missing file, malformed
// JSON) -- matches legacy's tolerance of a missing history directory
// (historique.py doesn't special-case it, relying on file-not-found
// exceptions per entry) without the renumbering fragility.
[[nodiscard]] std::vector<HistoryEntry> loadHistory(const QString& path);

[[nodiscard]] bool saveHistory(const QString& path, const std::vector<HistoryEntry>& entries);

} // namespace mathclav::core::history
