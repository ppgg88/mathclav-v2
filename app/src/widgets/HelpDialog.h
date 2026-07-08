#pragma once

#include <QDialog>
#include <QString>

class QStackedWidget;
class QPushButton;

// Ported from help.py's `help`/`help_2` classes: a keyboard-image view and
// a scrollable list view, toggled by a button, persisted as
// settings.helpViewMode (Phase 5 infra, `settingsPath`). Unlike legacy,
// the list view renders every key's Math/Greek output live via
// FormulaView instead of static matplotlib-generated screenshots -- see
// buildListView() in the .cpp, which walks the same
// mathclav::app::input::mathModeCandidates/greekUppercase/greekLowercase
// tables KeyDispatcher itself dispatches through, so this view can never
// drift out of sync with actual behavior.
class HelpDialog : public QDialog {
    Q_OBJECT

public:
    HelpDialog(bool darkTheme, QString settingsPath, QWidget* parent = nullptr);

private:
    QWidget* buildKeyboardView();
    QWidget* buildListView();
    void toggleView();

    bool darkTheme_;
    QString settingsPath_;
    QStackedWidget* stack_ = nullptr;
    QPushButton* toggleButton_ = nullptr;
};
