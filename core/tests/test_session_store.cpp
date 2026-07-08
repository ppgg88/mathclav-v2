#include <catch2/catch_test_macros.hpp>

#include "mathclav/core/ast/Node.h"
#include "mathclav/core/session/SessionStore.h"

#include <QFile>
#include <QTemporaryDir>

using namespace mathclav::core::ast;
using namespace mathclav::core::session;

TEST_CASE("saveSession then loadSession round-trips the document", "[persistence][session]") {
    QTemporaryDir dir;
    REQUIRE(dir.isValid());
    const QString path = dir.filePath("session-autosave.json");

    Container original;
    original.push_back(Node::symbol(L"x"));
    original.push_back(Node::composite(Kind::Sqrt));

    REQUIRE(saveSession(path, original));

    Container restored;
    REQUIRE(loadSession(path, restored));
    REQUIRE(restored == original);
}

TEST_CASE("loadSession returns false for a missing file", "[persistence][session]") {
    QTemporaryDir dir;
    Container out;
    REQUIRE_FALSE(loadSession(dir.filePath("does-not-exist.json"), out));
}

TEST_CASE("loadSession returns false for malformed JSON, not a crash", "[persistence][session]") {
    QTemporaryDir dir;
    const QString path = dir.filePath("corrupt.json");
    QFile f(path);
    REQUIRE(f.open(QIODevice::WriteOnly));
    f.write("{ this is not valid json");
    f.close();

    Container out;
    REQUIRE_FALSE(loadSession(path, out));
}

TEST_CASE("saveSession creates missing parent directories", "[persistence][session]") {
    QTemporaryDir dir;
    const QString path = dir.filePath("nested/subdir/session.json");
    Container doc;
    doc.push_back(Node::symbol(L"y"));
    REQUIRE(saveSession(path, doc));
    REQUIRE(QFile::exists(path));
}
