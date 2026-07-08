#include "input/KeyDispatcher.h"

#include "input/KeymapTables.h"
#include "input/PhysicalKey.h"

#include "mathclav/core/cursor/CursorOps.h"
#include "mathclav/core/latex/LatexSerializer.h"

#include <QClipboard>
#include <QGuiApplication>
#include <QKeyEvent>
#include <QKeySequence>

#include <chrono>
#include <utility>

namespace mathclav::app::input {

namespace ast = mathclav::core::ast;
namespace cursor = mathclav::core::cursor;
namespace latex = mathclav::core::latex;

namespace {

// One id per candidate family, so MultiChoiceCycleState's "same trigger
// pressed again" check (unlike legacy's string-comparison heuristic, see
// MultiChoiceCycle.h) is unambiguous by construction.
enum TriggerId : int {
    kEquals = 1,
    kTimes,
    kPlus,
    kMinus,
    kBang,
    kBrace,
    kAmpersand,
    kLess,
    kGreater,
    kQuote,
    kSpace,
    kDollar,
    kEnter,
    kPowerIndice,
    kParens,
    kNorm,
    kSubscriptOnly,
    kMathLetterBase = 100, // + (letter - 'A'), one id per Math-mode A-Z key
};

Node sym(const wchar_t* s) {
    return Node::symbol(s);
}

Node comp(ast::Kind k) {
    return Node::composite(k);
}

} // namespace

bool KeyDispatcher::dispatch(QKeyEvent* event) {
    const QString text = event->text();

    if (onMultiChoiceShouldHide) onMultiChoiceShouldHide();

    // Ctrl toggles Greek mode (index.py 504-513). Legacy only reacts to
    // *left* Ctrl (`keyname == 'Control_L'`); Qt doesn't expose left/right
    // without native-code lookups, and there's no reason a French AZERTY
    // keyboard user would want the two Ctrl keys to behave differently
    // here, so this deliberately treats either as the toggle.
    if (event->key() == Qt::Key_Control) {
        setMode(mode_ == Mode::Greek ? Mode::Normal : Mode::Greek);
        return true;
    }

    // The AZERTY "²" key toggles Math mode (index.py 526-534,
    // `keyname == 'twosuperior'`). This is genuinely character-based in
    // legacy (it reads touche.keysym, not touche.keycode), not a
    // physical-key check -- see KeymapTables.h's note on how legacy's
    // other-looking "physical" checks turn out to be character-based too.
    if (text == QStringLiteral("²")) {
        setMode(mode_ == Mode::Math ? Mode::Normal : Mode::Math);
        return true;
    }

    // No-action keys (index.py line 537): bare modifiers, and a few
    // characters that never do anything in any mode, plus '^' while in
    // Math mode specifically (there it's reserved for the physical Caret
    // trigger below, not for literal insertion).
    switch (event->key()) {
        case Qt::Key_Alt:
        case Qt::Key_AltGr:
        case Qt::Key_Shift:
        case Qt::Key_CapsLock:
        case Qt::Key_Meta:
            return true;
        default:
            break;
    }
    if (text == QStringLiteral("\\") || text == QStringLiteral("\t") || text == QStringLiteral("#")
        || text == QStringLiteral("`") || (mode_ == Mode::Math && text == QStringLiteral("^"))) {
        return true;
    }

    // Navigation / editing / undo / copy (index.py 544-693). Escape is
    // recognized but a no-op until quit+session-save gets its own
    // confirmation flow.
    switch (event->key()) {
        case Qt::Key_F10:
            if (onGraphRequested) onGraphRequested();
            return true;
        case Qt::Key_F9:
            if (onCreditsRequested) onCreditsRequested();
            return true;
        case Qt::Key_Escape:
            return true;
        case Qt::Key_Left:
            cursor::moveLeft(document_, cursorPath_);
            if (onChanged) onChanged();
            return true;
        case Qt::Key_Right:
            cursor::moveRight(document_, cursorPath_);
            if (onChanged) onChanged();
            return true;
        case Qt::Key_Up:
            cursor::moveUp(document_, cursorPath_);
            if (onChanged) onChanged();
            return true;
        case Qt::Key_Down:
            cursor::moveDown(document_, cursorPath_);
            if (onChanged) onChanged();
            return true;
        case Qt::Key_Backspace:
            pushUndoSnapshot();
            cursor::deleteBackward(document_, cursorPath_);
            if (onChanged) onChanged();
            return true;
        case Qt::Key_Delete:
            pushUndoSnapshot();
            cursor::deleteForward(document_, cursorPath_);
            if (onChanged) onChanged();
            return true;
        default:
            break;
    }
    if (event->matches(QKeySequence::Undo)) {
        undo();
        return true;
    }
    if (event->matches(QKeySequence::Copy)) {
        copyToClipboard();
        return true;
    }

    // "raptor" easter egg (index.py line 700): pressing '=' when the whole
    // document reads exactly "raptor".
    if (text == QStringLiteral("=") && latex::render(document_) == L"raptor") {
        if (onRaptorTriggered) onRaptorTriggered();
        return true;
    }

    // Mode-independent multi-choice triggers (index.py 704-750): active
    // regardless of Normal/Math/Greek.
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        triggerMultiChoice(kEnter, {sym(L"\\newline")});
        return true;
    }
    if (text == QStringLiteral("=")) {
        triggerMultiChoice(kEquals, {sym(L"="), sym(L"\\approx "), sym(L"\\neq "), sym(L"\\equiv "),
                                      sym(L"\\sim "), sym(L"\\simeq "), sym(L"\\propto ")});
        return true;
    }
    if (text == QStringLiteral("*")) {
        triggerMultiChoice(kTimes, {sym(L"\\times "), sym(L"\\cdot "), sym(L"\\wedge "), sym(L"\\ast "),
                                     sym(L"\\odot "), sym(L"\\otimes ")});
        return true;
    }
    if (text == QStringLiteral("+")) {
        triggerMultiChoice(kPlus, {sym(L"+ "), sym(L"\\pm "), sym(L"\\mp "), sym(L"\\oplus ")});
        return true;
    }
    if (text == QStringLiteral("-")) {
        triggerMultiChoice(kMinus, {sym(L"- "), sym(L"\\mp "), sym(L"\\pm "), sym(L"\\ominus ")});
        return true;
    }
    if (text == QStringLiteral("!")) {
        triggerMultiChoice(kBang, {sym(L"! "), sym(L"\\neg "), sym(L"\\not ")});
        return true;
    }
    if (text == QStringLiteral("{") || text == QStringLiteral("}")) {
        triggerMultiChoice(kBrace, {comp(ast::Kind::ParenCurly)});
        return true;
    }
    if (text == QStringLiteral("&")) {
        triggerMultiChoice(kAmpersand, {sym(L"\\wedge "), sym(L"\\vee "), sym(L"& ")});
        return true;
    }
    if (text == QStringLiteral("<")) {
        triggerMultiChoice(kLess, {sym(L"<"), sym(L">"), sym(L"\\leq "), sym(L"\\geq "), sym(L"\\ll "), sym(L"\\gg ")});
        return true;
    }
    if (text == QStringLiteral(">")) {
        triggerMultiChoice(kGreater,
                            {sym(L">"), sym(L"<"), sym(L"\\geq "), sym(L"\\leq "), sym(L"\\gg "), sym(L"\\ll ")});
        return true;
    }
    if (text == QStringLiteral("\"")) {
        triggerMultiChoice(kQuote, {comp(ast::Kind::Text)});
        return true;
    }
    if (event->key() == Qt::Key_Space) {
        triggerMultiChoice(kSpace, {sym(L"\\: ")});
        return true;
    }
    if (text == QStringLiteral("$")) {
        triggerMultiChoice(kDollar, {sym(L"\\$")});
        return true;
    }

    // Mode-specific fallthrough (index.py 752-804). The triggerId passed
    // below is never consulted: all of these are single-candidate inserts,
    // which triggerMultiChoice always routes through the non-cycling
    // direct-insert path.
    if (mode_ == Mode::Normal) {
        if (!text.isEmpty() && text.at(0).isPrint()) {
            triggerMultiChoice(0, {Node::symbol(text.toStdWString())});
        }
        return true;
    }

    if (mode_ == Mode::Math) {
        if (text == QStringLiteral("h") || text == QStringLiteral("H")) {
            if (onHistoryRequested) onHistoryRequested();
            return true;
        }
        if (text.size() == 1 && text.at(0).isLetter()) {
            const QChar upper = text.at(0).toUpper();
            std::vector<Node> candidates = mathModeCandidates(upper);
            if (!candidates.empty()) {
                triggerMultiChoice(kMathLetterBase + (upper.toLatin1() - 'A'), candidates);
                return true;
            }
        }
        const PhysicalKey phys = fromEvent(event);
        if (phys == PhysicalKey::Slash) {
            convertPrecedingIntoFractionNumerator();
            return true;
        }
        if (phys == PhysicalKey::Caret) {
            triggerMultiChoice(kPowerIndice, {comp(ast::Kind::Power), comp(ast::Kind::Subscript)});
            return true;
        }
        if (text == QStringLiteral("(") || text == QStringLiteral(")")) {
            triggerMultiChoice(kParens, {comp(ast::Kind::Paren), comp(ast::Kind::ParenSquare)});
            return true;
        }
        if (text == QStringLiteral("|")) {
            triggerMultiChoice(kNorm, {comp(ast::Kind::Norm), comp(ast::Kind::NormDouble)});
            return true;
        }
        if (text == QStringLiteral("_")) {
            triggerMultiChoice(kSubscriptOnly, {comp(ast::Kind::Subscript)});
            return true;
        }
        if (!text.isEmpty()) {
            triggerMultiChoice(0, {Node::symbol(text.toStdWString())});
        }
        return true;
    }

    if (mode_ == Mode::Greek) {
        if (text.size() == 1 && text.at(0).isLetter()) {
            const QChar upper = text.at(0).toUpper();
            const Node candidate = text.at(0).isUpper() ? greekUppercase(upper) : greekLowercase(upper);
            triggerMultiChoice(0, {candidate});
        }
        return true;
    }

    return false;
}

void KeyDispatcher::setMode(Mode mode) {
    mode_ = mode;
    if (onModeChanged) onModeChanged(mode_);
}

void KeyDispatcher::cycleMode() {
    switch (mode_) {
        case Mode::Normal: setMode(Mode::Math); return;
        case Mode::Math: setMode(Mode::Greek); return;
        case Mode::Greek: setMode(Mode::Normal); return;
    }
}

void KeyDispatcher::setModeDirect(Mode mode) {
    setMode(mode);
}

void KeyDispatcher::pushUndoSnapshot() {
    undoStack_.push_back(Snapshot{document_, cursorPath_});
}

void KeyDispatcher::undo() {
    if (undoStack_.empty()) {
        return;
    }
    Snapshot snapshot = std::move(undoStack_.back());
    undoStack_.pop_back();
    document_ = std::move(snapshot.document);
    cursorPath_ = std::move(snapshot.cursorPath);
    if (onChanged) onChanged();
}

void KeyDispatcher::copyToClipboard() const {
    QGuiApplication::clipboard()->setText(QString::fromStdWString(latex::render(document_)));
}

void KeyDispatcher::triggerMultiChoice(int triggerId, const std::vector<Node>& candidates) {
    if (candidates.empty()) {
        return;
    }
    pushUndoSnapshot();
    if (candidates.size() == 1) {
        // Matches legacy's multiple_choice(): a single-candidate list never
        // cycles (index.py's `if len(temp) == 1` branch always takes the
        // direct-insert path, regardless of repeated presses or timing).
        // Bypassing MultiChoiceCycleState here isn't just an optimization:
        // routing single-candidate calls through it would let its "same
        // trigger within the window" check misfire whenever two unrelated
        // single-candidate inserts reuse a triggerId (e.g. plain character
        // passthrough), erasing the wrong node instead of just inserting.
        cursor::insertNode(document_, cursorPath_, candidates[0]);
        if (onChanged) onChanged();
        return;
    }
    const int activeIndex =
        cycle_.press(document_, cursorPath_, triggerId, candidates, std::chrono::steady_clock::now());
    if (onMultiChoice) {
        onMultiChoice(candidates, activeIndex);
    }
    if (onChanged) onChanged();
}

void KeyDispatcher::convertPrecedingIntoFractionNumerator() {
    Container& container = cursor::resolveContainer(document_, cursorPath_);
    if (cursorPath_.offset == 0) {
        return; // nothing before the cursor to convert
    }
    pushUndoSnapshot();

    Node preceding = std::move(container[cursorPath_.offset - 1]);
    container.erase(container.begin() + static_cast<std::ptrdiff_t>(cursorPath_.offset - 1));
    cursorPath_.offset -= 1;

    cursor::insertNode(document_, cursorPath_, Node::composite(ast::Kind::Fraction), /*descendInto=*/true);
    cursor::insertNode(document_, cursorPath_, std::move(preceding), /*descendInto=*/false);
    cursor::moveDown(document_, cursorPath_);

    if (onChanged) onChanged();
}

void KeyDispatcher::loadDocument(Container document) {
    document_ = std::move(document);
    cursorPath_ = CursorPath{};
    cursorPath_.offset = document_.size();
    undoStack_.clear();
    if (onChanged) onChanged();
}

void KeyDispatcher::insertHistoryEntry(const Container& nodes) {
    if (nodes.empty()) {
        return;
    }
    pushUndoSnapshot();
    for (const Node& node : nodes) {
        cursor::insertNode(document_, cursorPath_, node, /*descendInto=*/false);
    }
    if (onChanged) onChanged();
}

} // namespace mathclav::app::input
