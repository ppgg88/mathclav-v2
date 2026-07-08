#pragma once

#include "mathclav/core/ast/Node.h"
#include "mathclav/core/history/HistoryStore.h"

#include <QDialog>

class QLineEdit;
class QVBoxLayout;

// Ported from historique.py: save the current expression under a name,
// browse saved entries with a live rendered preview, click a name to
// insert it into the current document, delete an entry. Replaces the
// per-entry-pickle-file + separate counter file + os.rename-on-delete
// scheme (historique.py 41-45, 151-169) with the single JSON array
// core::history::HistoryStore already provides -- this dialog only
// handles the UI, all persistence goes through loadHistory/saveHistory.
class HistoryDialog : public QDialog {
    Q_OBJECT

public:
    // `historyPath` is the already-resolved path to history.json
    // (QStandardPaths::AppDataLocation, computed by MainWindow).
    HistoryDialog(mathclav::core::ast::Container currentDocument, QString historyPath, QWidget* parent = nullptr);

signals:
    // Emitted when the user clicks a saved entry's name (historique.py's
    // ajouter(), 171-174). The dialog closes itself right after emitting.
    void entryInsertRequested(const mathclav::core::ast::Container& document);

private:
    void rebuildList();
    void onSaveClicked();
    void deleteEntryAt(int index);

    mathclav::core::ast::Container currentDocument_;
    QString historyPath_;
    std::vector<mathclav::core::history::HistoryEntry> entries_;

    QLineEdit* nameEdit_ = nullptr;
    QVBoxLayout* listLayout_ = nullptr;
};
