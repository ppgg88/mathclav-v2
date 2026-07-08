#pragma once

#include <QDialog>

// Ported from credit.py's `raptor` class, triggered when '=' is pressed
// while the whole document reads exactly "raptor" (KeyDispatcher.cpp,
// index.py line 700). Kept as-is per the plan's scope -- it's an easter
// egg, not a feature with legacy bugs to fix.
class RaptorEasterEgg : public QDialog {
    Q_OBJECT

public:
    explicit RaptorEasterEgg(QWidget* parent = nullptr);
};
