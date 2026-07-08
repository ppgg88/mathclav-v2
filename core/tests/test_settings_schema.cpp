#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "mathclav/core/settings/SettingsSchema.h"

#include <QFile>
#include <QTemporaryDir>

using namespace mathclav::core::settings;

TEST_CASE("loadSettings returns defaults for a missing file", "[persistence][settings]") {
    QTemporaryDir dir;
    const Settings s = loadSettings(dir.filePath("nope.json"));
    REQUIRE(s.theme == QStringLiteral("dark"));
    REQUIRE(s.fontSize == 11);
}

TEST_CASE("saveSettings then loadSettings round-trips every field", "[persistence][settings]") {
    QTemporaryDir dir;
    const QString path = dir.filePath("settings.json");

    Settings s;
    s.theme = QStringLiteral("light");
    s.fontSize = 16;
    s.helpViewMode = QStringLiteral("list");
    s.physicalKeyboardOverride = QStringLiteral("azerty");
    s.graphDefaults.xMin = -5;
    s.graphDefaults.xMax = 5;
    s.graphDefaults.step = 0.02;
    s.graphDefaults.variable = QStringLiteral("t");

    REQUIRE(saveSettings(path, s));
    const Settings restored = loadSettings(path);

    REQUIRE(restored.theme == s.theme);
    REQUIRE(restored.fontSize == s.fontSize);
    REQUIRE(restored.helpViewMode == s.helpViewMode);
    REQUIRE(restored.physicalKeyboardOverride == s.physicalKeyboardOverride);
    REQUIRE(restored.graphDefaults.xMin == Catch::Approx(-5.0));
    REQUIRE(restored.graphDefaults.xMax == Catch::Approx(5.0));
    REQUIRE(restored.graphDefaults.step == Catch::Approx(0.02));
    REQUIRE(restored.graphDefaults.variable == QStringLiteral("t"));
}

TEST_CASE("loadSettings falls back to defaults on malformed JSON", "[persistence][settings]") {
    QTemporaryDir dir;
    const QString path = dir.filePath("corrupt.json");
    QFile f(path);
    REQUIRE(f.open(QIODevice::WriteOnly));
    f.write("not json at all");
    f.close();

    const Settings s = loadSettings(path);
    REQUIRE(s.theme == QStringLiteral("dark"));
}
