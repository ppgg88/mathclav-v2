#pragma once

#include <QString>

namespace mathclav::core::settings {

// Default plot parameters for a freshly-opened GrapherDialog (app/src/
// widgets/GrapherDialog.cpp), persisted across restarts like legacy's
// settings['settings']['graph'] block (graph.py 57-89).
struct GraphDefaults {
    double xMin = 0, xMax = 10, yMin = 0, yMax = 10;
    double step = 0.01;
    QString variable = QStringLiteral("x");
};

// Ported from index.py/graph.py's settings.json (written ad hoc in half a
// dozen places in legacy, e.g. index.py 96-98, 972-974, 1013-1014, with no
// single schema owner). `helpViewMode`/`physicalKeyboardOverride` are
// forward-declared fields for Phase 6/3 features that don't have UI to set
// them yet; they round-trip with their defaults until then.
struct Settings {
    QString theme = QStringLiteral("dark");
    int fontSize = 11;
    QString helpViewMode = QStringLiteral("keyboard");
    QString physicalKeyboardOverride = QStringLiteral("auto");
    GraphDefaults graphDefaults;
};

// Returns default-constructed Settings on any read failure (missing file,
// malformed JSON), matching legacy's `try: ... except: <write defaults>`
// fallback (index.py 91-98) minus the code-execution risk of a corrupt
// file being blindly trusted.
[[nodiscard]] Settings loadSettings(const QString& path);

[[nodiscard]] bool saveSettings(const QString& path, const Settings& settings);

} // namespace mathclav::core::settings
