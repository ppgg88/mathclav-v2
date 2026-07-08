#include <catch2/catch_test_macros.hpp>

#include "mathclav/core/history/HistoryStore.h"

#include <QTemporaryDir>

using namespace mathclav::core::ast;
using namespace mathclav::core::history;

TEST_CASE("saveHistory then loadHistory round-trips entries", "[persistence][history]") {
    QTemporaryDir dir;
    const QString path = dir.filePath("history.json");

    HistoryEntry a;
    a.id = QStringLiteral("id-a");
    a.name = QStringLiteral("Pythagore");
    a.document.push_back(Node::symbol(L"x"));

    HistoryEntry b;
    b.id = QStringLiteral("id-b");
    b.name = QStringLiteral("Euler");
    b.document.push_back(Node::composite(Kind::EulerPower));

    REQUIRE(saveHistory(path, {a, b}));

    const std::vector<HistoryEntry> restored = loadHistory(path);
    REQUIRE(restored.size() == 2);
    REQUIRE(restored[0].id == a.id);
    REQUIRE(restored[0].name == a.name);
    REQUIRE(restored[0].document == a.document);
    REQUIRE(restored[1].id == b.id);
    REQUIRE(restored[1].document == b.document);
}

TEST_CASE("loadHistory returns an empty list for a missing file", "[persistence][history]") {
    QTemporaryDir dir;
    REQUIRE(loadHistory(dir.filePath("nope.json")).empty());
}

TEST_CASE("deleting an entry doesn't renumber the survivors' identity", "[persistence][history]") {
    // The exact legacy failure mode this design avoids (historique.py's
    // os.rename-based renumbering on delete, 151-169): saving a list with
    // the first entry removed must leave the remaining entries' own ids
    // intact, not shifted.
    QTemporaryDir dir;
    const QString path = dir.filePath("history.json");

    HistoryEntry a;
    a.id = QStringLiteral("id-a");
    a.document.push_back(Node::symbol(L"a"));
    HistoryEntry b;
    b.id = QStringLiteral("id-b");
    b.document.push_back(Node::symbol(L"b"));
    REQUIRE(saveHistory(path, {a, b}));

    std::vector<HistoryEntry> loaded = loadHistory(path);
    loaded.erase(loaded.begin()); // remove "a"
    REQUIRE(saveHistory(path, loaded));

    const std::vector<HistoryEntry> restored = loadHistory(path);
    REQUIRE(restored.size() == 1);
    REQUIRE(restored[0].id == QStringLiteral("id-b"));
}
