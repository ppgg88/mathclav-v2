#include "widgets/UpdateCheckDialog.h"

#include "mathclav/core/Version.h"

#include <QAbstractButton>
#include <QDesktopServices>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPushButton>
#include <QStringList>
#include <QUrl>

#include <array>

namespace {
// GitHub's own "latest release" API: the release job in
// .github/workflows/ci.yml already publishes a GitHub Release on every
// `v*` tag push, so this needs no separately-maintained version file the
// way legacy's verssion.json (and this class's own first draft, which
// pointed at the same file on the legacy repo) did.
const QUrl kLatestReleaseUrl(
    QStringLiteral("https://api.github.com/repos/ppgg88/mathclav-v2/releases/latest"));
const QUrl kReleasesUrl(QStringLiteral("https://github.com/ppgg88/mathclav-v2/releases"));

// Parses a "vX.Y.Z" or "X.Y.Z" tag into up to 3 numeric components,
// missing/non-numeric trailing parts treated as 0 (so "2.1" reads as
// "2.1.0"). Returns an all-zero version for anything unparseable, which
// compares as "not newer" against any real release -- fails closed rather
// than misfiring on garbage input.
std::array<int, 3> parseVersion(QString text) {
    if (text.startsWith(QLatin1Char('v'), Qt::CaseInsensitive)) {
        text.remove(0, 1);
    }
    const QStringList parts = text.split(QLatin1Char('.'));
    std::array<int, 3> out{0, 0, 0};
    for (int i = 0; i < 3 && i < parts.size(); ++i) {
        bool ok = false;
        const int value = parts[i].toInt(&ok);
        out[static_cast<std::size_t>(i)] = ok ? value : 0;
    }
    return out;
}

bool isNewer(const std::array<int, 3>& remote, const std::array<int, 3>& local) {
    return remote > local;
}

} // namespace

UpdateCheckDialog::UpdateCheckDialog(QWidget* parentForDialogs, QObject* parent)
    : QObject(parent), parentForDialogs_(parentForDialogs) {
    network_ = new QNetworkAccessManager(this);
}

void UpdateCheckDialog::checkNow() {
    QNetworkRequest request(kLatestReleaseUrl);
    // Required by the GitHub API for all requests; an empty/missing
    // User-Agent is rejected outright rather than just being deprioritized.
    request.setRawHeader("User-Agent", "MathClav-UpdateCheck");
    QNetworkReply* reply = network_->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply] {
        handleReply(reply);
        reply->deleteLater();
    });
}

void UpdateCheckDialog::handleReply(QNetworkReply* reply) {
    if (reply->error() != QNetworkReply::NoError) {
        return; // no repo/release yet, offline, rate-limited, ... : silently no-op
    }
    const QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
    if (!doc.isObject()) {
        return;
    }
    const QString remoteTag = doc.object().value(QStringLiteral("tag_name")).toString();
    const QString localVersion = QString::fromUtf8(mathclav::core::version().data());
    if (remoteTag.isEmpty()) {
        return;
    }
    if (!isNewer(parseVersion(remoteTag), parseVersion(localVersion))) {
        return;
    }

    auto* box = new QMessageBox(parentForDialogs_);
    box->setAttribute(Qt::WA_DeleteOnClose);
    box->setWindowTitle(QStringLiteral("Mise à jour"));
    box->setText(
        QStringLiteral("Une nouvelle version est disponible (%1, vous avez %2).").arg(remoteTag, localVersion));
    box->setStandardButtons(QMessageBox::Close);
    QAbstractButton* openReleasesButton = box->addButton(QStringLiteral("Voir les releases"), QMessageBox::AcceptRole);
    connect(box, &QMessageBox::buttonClicked, this, [openReleasesButton](QAbstractButton* clicked) {
        if (clicked == openReleasesButton) {
            QDesktopServices::openUrl(kReleasesUrl);
        }
    });
    box->show();
}
