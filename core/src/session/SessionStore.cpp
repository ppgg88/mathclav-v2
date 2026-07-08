#include "mathclav/core/session/SessionStore.h"

#include "mathclav/core/persistence/JsonNodeCodec.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>

namespace mathclav::core::session {

namespace {
constexpr int kSchemaVersion = 1;
}

bool loadSession(const QString& path, ast::Container& outDocument) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }
    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (!doc.isObject()) {
        return false;
    }
    const QJsonValue documentValue = doc.object().value(QStringLiteral("document"));
    if (!documentValue.isArray()) {
        return false;
    }
    try {
        outDocument = persistence::containerFromJson(documentValue.toArray());
    } catch (const persistence::PersistenceError&) {
        return false;
    }
    return true;
}

bool saveSession(const QString& path, const ast::Container& document) {
    QDir().mkpath(QFileInfo(path).absolutePath());
    // QSaveFile writes to a temp file and only replaces the real one on a
    // successful commit(), avoiding the truncated-file-on-crash window in
    // legacy's `f.seek(0); f.write(...); f.truncate()` pattern
    // (index.py 480-482).
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }
    QJsonObject obj;
    obj[QStringLiteral("schemaVersion")] = kSchemaVersion;
    obj[QStringLiteral("document")] = persistence::containerToJson(document);
    file.write(QJsonDocument(obj).toJson(QJsonDocument::Compact));
    return file.commit();
}

} // namespace mathclav::core::session
