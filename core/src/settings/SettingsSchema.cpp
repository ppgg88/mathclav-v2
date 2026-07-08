#include "mathclav/core/settings/SettingsSchema.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>

namespace mathclav::core::settings {

namespace {
constexpr int kSchemaVersion = 1;

QJsonObject graphDefaultsToJson(const GraphDefaults& g) {
    QJsonObject obj;
    obj[QStringLiteral("xMin")] = g.xMin;
    obj[QStringLiteral("xMax")] = g.xMax;
    obj[QStringLiteral("yMin")] = g.yMin;
    obj[QStringLiteral("yMax")] = g.yMax;
    obj[QStringLiteral("step")] = g.step;
    obj[QStringLiteral("variable")] = g.variable;
    return obj;
}

GraphDefaults graphDefaultsFromJson(const QJsonObject& obj, const GraphDefaults& fallback) {
    GraphDefaults g = fallback;
    if (obj.value(QStringLiteral("xMin")).isDouble()) g.xMin = obj.value(QStringLiteral("xMin")).toDouble();
    if (obj.value(QStringLiteral("xMax")).isDouble()) g.xMax = obj.value(QStringLiteral("xMax")).toDouble();
    if (obj.value(QStringLiteral("yMin")).isDouble()) g.yMin = obj.value(QStringLiteral("yMin")).toDouble();
    if (obj.value(QStringLiteral("yMax")).isDouble()) g.yMax = obj.value(QStringLiteral("yMax")).toDouble();
    if (obj.value(QStringLiteral("step")).isDouble()) g.step = obj.value(QStringLiteral("step")).toDouble();
    if (obj.value(QStringLiteral("variable")).isString()) g.variable = obj.value(QStringLiteral("variable")).toString();
    return g;
}

} // namespace

Settings loadSettings(const QString& path) {
    const Settings defaults;
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        return defaults;
    }
    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isObject()) {
        return defaults;
    }
    const QJsonObject obj = doc.object();
    Settings settings = defaults;
    if (obj.value(QStringLiteral("theme")).isString()) settings.theme = obj.value(QStringLiteral("theme")).toString();
    if (obj.value(QStringLiteral("fontSize")).isDouble()) settings.fontSize = obj.value(QStringLiteral("fontSize")).toInt();
    if (obj.value(QStringLiteral("helpViewMode")).isString())
        settings.helpViewMode = obj.value(QStringLiteral("helpViewMode")).toString();
    if (obj.value(QStringLiteral("physicalKeyboardOverride")).isString())
        settings.physicalKeyboardOverride = obj.value(QStringLiteral("physicalKeyboardOverride")).toString();
    if (obj.value(QStringLiteral("graphDefaults")).isObject())
        settings.graphDefaults = graphDefaultsFromJson(obj.value(QStringLiteral("graphDefaults")).toObject(),
                                                         defaults.graphDefaults);
    return settings;
}

bool saveSettings(const QString& path, const Settings& settings) {
    QDir().mkpath(QFileInfo(path).absolutePath());
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }
    QJsonObject obj;
    obj[QStringLiteral("schemaVersion")] = kSchemaVersion;
    obj[QStringLiteral("theme")] = settings.theme;
    obj[QStringLiteral("fontSize")] = settings.fontSize;
    obj[QStringLiteral("helpViewMode")] = settings.helpViewMode;
    obj[QStringLiteral("physicalKeyboardOverride")] = settings.physicalKeyboardOverride;
    obj[QStringLiteral("graphDefaults")] = graphDefaultsToJson(settings.graphDefaults);
    file.write(QJsonDocument(obj).toJson(QJsonDocument::Compact));
    return file.commit();
}

} // namespace mathclav::core::settings
