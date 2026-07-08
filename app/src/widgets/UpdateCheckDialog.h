#pragma once

#include <QObject>

class QWidget;
class QNetworkAccessManager;
class QNetworkReply;

// Ported from update.py, with the unsafe half removed: legacy's
// update_download() (update.py 18-29) downloads an installer .exe over
// plain requests.get and runs it via `os.system(...)` with no signature
// or checksum verification -- a silent-arbitrary-code-execution path
// triggered automatically on every startup (update_q(), called from
// index.py's own __init__). This class only ever fetches GitHub's own
// "latest release" API for this repo, compares its tag against the local
// version numerically (major.minor.patch, not string inequality -- see
// UpdateCheckDialog.cpp), and -- if the remote one is actually newer --
// shows a dialog whose only action is opening the releases page in the
// user's browser (QDesktopServices::openUrl). Nothing is ever downloaded
// or executed automatically.
class UpdateCheckDialog : public QObject {
    Q_OBJECT

public:
    explicit UpdateCheckDialog(QWidget* parentForDialogs, QObject* parent = nullptr);

    // Fetches the version file asynchronously; shows a dialog only if a
    // newer version is found (or the fetch/parse fails silently, matching
    // legacy's `except: return "0"` -- a failed check just means "no
    // update available", not an error dialog).
    void checkNow();

private:
    void handleReply(QNetworkReply* reply);

    QNetworkAccessManager* network_;
    QWidget* parentForDialogs_;
};
