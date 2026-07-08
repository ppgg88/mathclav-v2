#include "widgets/RaptorEasterEgg.h"

#include <QLabel>
#include <QPixmap>
#include <QPushButton>
#include <QVBoxLayout>

RaptorEasterEgg::RaptorEasterEgg(QWidget* parent)
    : QDialog(parent) {
    setWindowTitle(QStringLiteral("MathClav - Credit"));

    auto* outer = new QVBoxLayout(this);

    auto* image = new QLabel(this);
    image->setPixmap(QPixmap(QStringLiteral(":/raptor.jpg")).scaled(300, 300, Qt::KeepAspectRatio,
                                                                      Qt::SmoothTransformation));
    outer->addWidget(image);

    auto* quitButton = new QPushButton(QStringLiteral("Quitter"), this);
    connect(quitButton, &QPushButton::clicked, this, &QDialog::accept);
    outer->addWidget(quitButton);
}
