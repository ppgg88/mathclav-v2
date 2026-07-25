#include "widgets/CreditsDialog.h"

#include "mathclav/core/Version.h"

#include <QDesktopServices>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QUrl>
#include <QVBoxLayout>

CreditsDialog::CreditsDialog(QWidget* parent)
    : QDialog(parent) {
    setWindowTitle(QStringLiteral("MathClav - Credit"));

    auto* outer = new QVBoxLayout(this);

    // Rich text rather than the plain "\n" block this started as, so the
    // contributor handles are actual clickable links instead of URLs the
    // reader has to retype. Note the escaped "&amp;": under Qt::RichText a
    // bare "&" in "SchnakyX & apparentés" would be parsed as the start of
    // an entity and silently swallow what follows.
    const QString text =
        QStringLiteral("MathClav v%1<br>Par : Team SchnakyX &amp; apparentés (TS&amp;a)<br><br>"
                        "Contributeurs :<br>"
                        "<a href=\"https://github.com/ppgg88\">ppgg88</a> &middot; "
                        "<a href=\"https://github.com/zatomos\">zatomos</a><br><br>"
                        "Licence : <a href=\"https://www.gnu.org/licenses/gpl-3.0.html\">GNU GPL v3</a> - 2022 MathClav<br>"
                        "This program is free software: you can redistribute it and/or modify it under the "
                        "terms of the GNU General Public License, version 3.<br>"
                        "It comes with ABSOLUTELY NO WARRANTY.<br><br>"
                        "Nous contacter : paul.giroux87@gmail.com")
            .arg(QString::fromUtf8(mathclav::core::version().data()));
    auto* label = new QLabel(text, this);
    label->setTextFormat(Qt::RichText);
    label->setAlignment(Qt::AlignCenter);
    // Hands the href to QDesktopServices, matching what the buttons below do.
    label->setOpenExternalLinks(true);
    outer->addWidget(label);

    auto* buttons = new QHBoxLayout();

    auto* quitButton = new QPushButton(QStringLiteral("Quitter"), this);
    connect(quitButton, &QPushButton::clicked, this, &QDialog::accept);
    buttons->addWidget(quitButton);

    auto* contactButton = new QPushButton(QStringLiteral("Nous contacter"), this);
    connect(contactButton, &QPushButton::clicked, this,
            [] { QDesktopServices::openUrl(QUrl(QStringLiteral("mailto:paul.giroux87@gmail.com"))); });
    buttons->addWidget(contactButton);

    auto* docButton = new QPushButton(QStringLiteral("Documentation"), this);
    connect(docButton, &QPushButton::clicked, this,
            [] { QDesktopServices::openUrl(QUrl(QStringLiteral("https://github.com/ppgg88/mathclav"))); });
    buttons->addWidget(docButton);

    outer->addLayout(buttons);
}
