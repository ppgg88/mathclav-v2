#pragma once

#include "input/KeyDispatcher.h"

#include <QMainWindow>
#include <QString>

class FormulaView;
class MultiChoicePopup;
class QLabel;
class QCloseEvent;
class QPushButton;

// The editor shell. Owns a KeyDispatcher (document/cursor/mode/undo state
// and the full keyboard dispatch logic, app/src/input/KeyDispatcher.h) and
// wires its callbacks to the two pieces of UI that react to it: the
// FormulaView (re-render on change) and MultiChoicePopup (show on a
// multi-candidate cycle). keyPressEvent itself is now a one-line
// delegation -- see KeyDispatcher for the actual dispatch order.
//
// Also owns session/history/settings persistence (Phase 5): the document
// is restored from session-autosave.json on startup and saved back on
// close (replacing legacy's pickle-based last.pkl, index.py
// 101-107/914-917); history.json's and settings.json's paths are resolved
// here and handed to HistoryDialog/HelpDialog.
//
// The toolbar (Copier/Effacer/Thème/mode selector/Graph/Aide) ports
// index.py's createWidgets() button row (362-434), minus the LaTeX-engine
// and font-size comboboxes: those existed to pick between MathClav's own
// matplotlib mathtext renderer and a "true" LaTeX install, a distinction
// that doesn't apply to MicroTeX. The three-button mode selector (rather
// than legacy's single cycling "Mode" button) is a deliberate ergonomics
// improvement requested directly by the user: which mode is active, and
// how to reach any other one directly, are both visible at a glance
// instead of requiring blind repeated clicks.
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

protected:
    void keyPressEvent(QKeyEvent* event) override;
    void closeEvent(QCloseEvent* event) override;

private:
    void updateRender();
    void updateModeLabel(mathclav::app::input::Mode mode);
    [[nodiscard]] QString dataFilePath(const QString& fileName) const;
    void openHistoryDialog();
    void openHelpDialog();
    void clearDocument();
    void toggleTheme();
    void applyTheme();

    FormulaView* formulaView_ = nullptr;
    QLabel* modeLabel_ = nullptr;
    QPushButton* normalModeButton_ = nullptr;
    QPushButton* mathModeButton_ = nullptr;
    QPushButton* greekModeButton_ = nullptr;
    MultiChoicePopup* multiChoicePopup_ = nullptr;
    mathclav::app::input::KeyDispatcher dispatcher_;
    bool darkTheme_ = true;
};
