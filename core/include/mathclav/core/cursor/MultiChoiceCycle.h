#pragma once

#include "mathclav/core/cursor/CursorPath.h"

#include <chrono>
#include <optional>
#include <vector>

namespace mathclav::core::cursor {

// Reproduces MathClav's signature "one key, several symbols" behavior
// (index.py multiple_choice(), lines 806-861 -- e.g. pressing '=' inserts
// '=', and pressing it again within 1.5s replaces it with '\approx', then
// '\neq', cycling through the candidate list). This is stateful (the timer
// window spans key presses) but still fully unit-testable: the caller
// supplies `now` explicitly rather than this class reading a wall clock, so
// tests are deterministic.
//
// Deviation from legacy: legacy identifies "the same trigger pressed again"
// by comparing the *rendered LaTeX string* of the last-inserted node
// against each candidate's string (index.py line 818) -- a heuristic that
// could misfire if two unrelated triggers happen to share a candidate's
// text. Here the caller passes an explicit `triggerId` (e.g. one enum value
// per physical key/mode-independent symbol group), which is unambiguous by
// construction.
class MultiChoiceCycleState {
public:
    // Applies one keypress of a multi-candidate trigger: inserts (or, if
    // this is a same-trigger repress within the cycle window, replaces) the
    // appropriate candidate at the cursor, and returns its index within
    // `candidates`.
    //
    // Ported behavior for the composite-candidate case (e.g. Math mode's
    // 'T' key cycling cos/sin/tan/sinc, each of which auto-descends into
    // its own first slot on insert, index.py lines 850-855): a repress
    // first restores the path to where the *previous* candidate was
    // inserted -- discarding any editing done inside it after auto-descent,
    // exactly as legacy's `if self.rg != self.rg_prev_: pop back` does --
    // then erases it and inserts the new candidate in its place.
    int press(Container& root,
              CursorPath& path,
              int triggerId,
              const std::vector<Node>& candidates,
              std::chrono::steady_clock::time_point now);

private:
    static constexpr auto kCycleWindow = std::chrono::milliseconds(1500);

    std::optional<int> lastTriggerId_;
    int lastCandidateIndex_ = 0;
    std::chrono::steady_clock::time_point lastPressTime_{};
    CursorPath insertionPath_; // depth/offset of the candidate itself, pre-auto-descend
};

} // namespace mathclav::core::cursor
