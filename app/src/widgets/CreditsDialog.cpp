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

    const QString text =
        QStringLiteral("MathClav v%1\nPar : Team SchnakyX & apparentés (TS&a)\n\n"
                        "Licence (CC BY-NC-SA 4.0) 2022 - MathClav\n"
                        "This work is licensed under the Creative Commons Attribution-NonCommercial-ShareAlike 4.0\n\n"
                        "Nous contacter : paul.giroux87@gmail.com")
            .arg(QString::fromUtf8(mathclav::core::version().data()));
    auto* label = new QLabel(text, this);
    label->setAlignment(Qt::AlignCenter);
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
