#pragma once

#include <QDialog>

// F9, ported from credit.py's `credit` class: static text + two links
// (email, documentation). Kept nearly as-is per the plan's "core +
// targeted modernization" scope -- low cost, no legacy bugs to fix here.
class CreditsDialog : public QDialog {
    Q_OBJECT

public:
    explicit CreditsDialog(QWidget* parent = nullptr);
};
