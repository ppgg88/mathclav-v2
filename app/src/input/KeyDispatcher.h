#pragma once

#include "input/ModeState.h"

#include "mathclav/core/ast/Node.h"
#include "mathclav/core/cursor/CursorPath.h"
#include "mathclav/core/cursor/MultiChoiceCycle.h"

#include <functional>
#include <vector>

class QKeyEvent;

namespace mathclav::app::input {

// The editor's keyboard controller: owns the document/cursor/mode/undo
// state and replicates action()'s dispatch order (index.py 490-804) as an
// ordered sequence of checks in dispatch(). UI concerns (repainting the
// FormulaView, showing the multi-choice popup) are pushed out through
// callbacks so this class stays independent of any specific widget.
//
// Deliberately dropped from legacy, with no replacement (see plan doc):
// the `Alt_R/c/C/z/Z within 1000ms of Ctrl` recovery hack (index.py
// 514-524, a Tkinter-specific workaround). Escape is recognized but still
// a no-op pending a proper quit+session-save confirmation flow.
class KeyDispatcher {
public:
    using Container = mathclav::core::ast::Container;
    using Node = mathclav::core::ast::Node;
    using CursorPath = mathclav::core::cursor::CursorPath;

    // Returns true if the event was handled (dispatcher consumed it; the
    // caller should not do anything else with it).
    bool dispatch(QKeyEvent* event);

    [[nodiscard]] const Container& document() const { return document_; }
    [[nodiscard]] const CursorPath& cursorPath() const { return cursorPath_; }
    [[nodiscard]] Mode mode() const { return mode_; }

    // Replaces the document wholesale (session restore at startup, "Effacer
    // Tout" with an empty Container) and positions the cursor at its end,
    // matching legacy's initial cursor placement after restoring last.pkl
    // (index.py line 114). Clears undo history: there's nothing meaningful
    // to undo back to before a freshly-loaded document. Fires onChanged.
    void loadDocument(Container document);

    // Inserts a saved history entry's nodes flatly at the cursor, without
    // auto-descending into any of them (index.py's ajout(), 922-928:
    // `for data in input.content: self.result[self.rg].content.append(data)`).
    // Called directly by MainWindow when HistoryDialog's
    // entryInsertRequested fires -- this is a user-initiated action from a
    // dialog MainWindow itself opened, not something dispatch() detects,
    // so it doesn't need to go through a callback.
    void insertHistoryEntry(const Container& nodes);

    // Copies the document's LaTeX source to the clipboard (index.py's
    // copy_to_clipboard, also reachable via Ctrl+C in dispatch()). Public
    // so a toolbar button can call it directly, same as Ctrl+C.
    void copyToClipboard() const;

    // Cycles Normal -> Math -> Greek -> Normal (index.py's switchMode(),
    // 978-991).
    void cycleMode();

    // Sets the mode directly, for a three-button mode selector (an
    // ergonomics improvement over both legacy's cycling button and this
    // rewrite's own first toolbar draft: which mode is active and how to
    // reach any other one directly are both visible at a glance).
    void setModeDirect(Mode mode);

    // Fired whenever the document/cursor changed and the view should
    // reparse+repaint.
    std::function<void()> onChanged;
    // Fired whenever the mode changed (Normal/Math/Greek), e.g. to update
    // a status label.
    std::function<void(Mode)> onModeChanged;
    // Fired for an actual multi-candidate cycle (candidates.size() > 1),
    // matching legacy's popup-only-when-cycling behavior -- single-choice
    // inserts never call this.
    std::function<void(const std::vector<Node>& candidates, int activeIndex)> onMultiChoice;
    // Fired at the start of every dispatch() call, before the key is
    // processed: tells the UI to hide any lingering multi-choice popup
    // immediately rather than waiting out its 1.5s auto-dismiss timer.
    // Pressing any other key implicitly finalizes whatever was last
    // cycled to (it's already committed in the tree -- see
    // triggerMultiChoice -- so there's nothing left to "confirm", only
    // the now-stale preview to clear). If this same keypress goes on to
    // trigger a fresh cycle of its own, onMultiChoice fires again right
    // after and the popup reappears with new content in the same call, so
    // continuing to cycle the same trigger never visibly flickers.
    std::function<void()> onMultiChoiceShouldHide;
    std::function<void()> onHistoryRequested;
    std::function<void()> onRaptorTriggered;
    // F10 (index.py line 540-542): opens the grapher on the current document.
    std::function<void()> onGraphRequested;
    // F9 (index.py line 695-697): opens the credits dialog.
    std::function<void()> onCreditsRequested;

private:
    void setMode(Mode mode);
    void pushUndoSnapshot();
    void undo();

    // Routes through core::cursor::MultiChoiceCycleState, matching legacy's
    // multiple_choice() (index.py 806-861): always snapshots for undo
    // first, inserts/cycles, and only notifies onMultiChoice when there's
    // more than one candidate to show.
    void triggerMultiChoice(int triggerId, const std::vector<Node>& candidates);

    // The '/' physical-key special case (index.py 766-778): converts the
    // node immediately before the cursor into the numerator of a new
    // fraction, landing the cursor in the denominator. Bypasses
    // MultiChoiceCycleState (this is a one-off structural edit, not a
    // cyclable choice) -- see CursorOps.h's note on insertNode's
    // descendInto parameter, added specifically for this case.
    void convertPrecedingIntoFractionNumerator();

    Container document_;
    CursorPath cursorPath_;
    Mode mode_ = Mode::Normal;
    mathclav::core::cursor::MultiChoiceCycleState cycle_;

    struct Snapshot {
        Container document;
        CursorPath cursorPath;
    };
    std::vector<Snapshot> undoStack_;
};

} // namespace mathclav::app::input
