#include "mathclav/core/history/HistoryStore.h"

#include "mathclav/core/persistence/JsonNodeCodec.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>

namespace mathclav::core::history {

namespace {
constexpr int kSchemaVersion = 1;
}

std::vector<HistoryEntry> loadHistory(const QString& path) {
    std::vector<HistoryEntry> entries;
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        return entries;
    }
    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isObject()) {
        return entries;
    }
    const QJsonValue entriesValue = doc.object().value(QStringLiteral("entries"));
    if (!entriesValue.isArray()) {
        return entries;
    }
    for (const QJsonValue& v : entriesValue.toArray()) {
        if (!v.isObject()) continue;
        const QJsonObject obj = v.toObject();
        const QJsonValue documentValue = obj.value(QStringLiteral("document"));
        if (!obj.value(QStringLiteral("id")).isString() || !documentValue.isArray()) {
            continue; // skip a malformed entry rather than discarding the whole file
        }
        try {
            HistoryEntry entry;
            entry.id = obj.value(QStringLiteral("id")).toString();
            entry.name = obj.value(QStringLiteral("name")).toString();
            entry.document = persistence::containerFromJson(documentValue.toArray());
            entries.push_back(std::move(entry));
        } catch (const persistence::PersistenceError&) {
            continue;
        }
    }
    return entries;
}

bool saveHistory(const QString& path, const std::vector<HistoryEntry>& entries) {
    QDir().mkpath(QFileInfo(path).absolutePath());
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }
    QJsonArray entriesArray;
    for (const HistoryEntry& entry : entries) {
        QJsonObject obj;
        obj[QStringLiteral("id")] = entry.id;
        obj[QStringLiteral("name")] = entry.name;
        obj[QStringLiteral("document")] = persistence::containerToJson(entry.document);
        entriesArray.append(obj);
    }
    QJsonObject root;
    root[QStringLiteral("schemaVersion")] = kSchemaVersion;
    root[QStringLiteral("entries")] = entriesArray;
    file.write(QJsonDocument(root).toJson(QJsonDocument::Compact));
    return file.commit();
}

} // namespace mathclav::core::history
