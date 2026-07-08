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
#include <QUrl>

namespace {
// Placeholder pointing at the same repo legacy used (update.py line 6);
// there's no v2-specific release feed yet. Update this once mathclav-v2
// has its own tagged-release process (see plan doc, Phase 7).
const QUrl kVersionUrl(QStringLiteral("https://raw.githubusercontent.com/ppgg88/mathclav/master/verssion.json"));
const QUrl kReleasesUrl(QStringLiteral("https://github.com/ppgg88/mathclav/releases"));
} // namespace

UpdateCheckDialog::UpdateCheckDialog(QWidget* parentForDialogs, QObject* parent)
    : QObject(parent), parentForDialogs_(parentForDialogs) {
    network_ = new QNetworkAccessManager(this);
}

void UpdateCheckDialog::checkNow() {
    QNetworkReply* reply = network_->get(QNetworkRequest(kVersionUrl));
    connect(reply, &QNetworkReply::finished, this, [this, reply] {
        handleReply(reply);
        reply->deleteLater();
    });
}

void UpdateCheckDialog::handleReply(QNetworkReply* reply) {
    if (reply->error() != QNetworkReply::NoError) {
        return; // matches legacy's verssion() `except: return "0"` -- fail silently
    }
    const QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
    if (!doc.isObject()) {
        return;
    }
    const QString remoteVersion = doc.object().value(QStringLiteral("verssion")).toString();
    const QString localVersion = QString::fromUtf8(mathclav::core::version().data());
    if (remoteVersion.isEmpty() || remoteVersion == localVersion) {
        return;
    }

    auto* box = new QMessageBox(parentForDialogs_);
    box->setAttribute(Qt::WA_DeleteOnClose);
    box->setWindowTitle(QStringLiteral("Mise à jour"));
    box->setText(QStringLiteral("Une nouvelle version est disponible (%1, vous avez %2).").arg(remoteVersion, localVersion));
    box->setStandardButtons(QMessageBox::Close);
    QAbstractButton* openReleasesButton = box->addButton(QStringLiteral("Voir les releases"), QMessageBox::AcceptRole);
    connect(box, &QMessageBox::buttonClicked, this, [openReleasesButton](QAbstractButton* clicked) {
        if (clicked == openReleasesButton) {
            QDesktopServices::openUrl(kReleasesUrl);
        }
    });
    box->show();
}
