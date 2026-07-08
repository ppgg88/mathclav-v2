#include "mathclav/core/cursor/MultiChoiceCycle.h"

#include "mathclav/core/cursor/CursorOps.h"

#include <cassert>

namespace mathclav::core::cursor {

int MultiChoiceCycleState::press(Container& root,
                                  CursorPath& path,
                                  int triggerId,
                                  const std::vector<Node>& candidates,
                                  std::chrono::steady_clock::time_point now) {
    assert(!candidates.empty());

    int nextIndex = 0;
    const bool cycling = lastTriggerId_.has_value() && *lastTriggerId_ == triggerId &&
                          (now - lastPressTime_) < kCycleWindow;

    if (cycling) {
        nextIndex = (lastCandidateIndex_ + 1) % static_cast<int>(candidates.size());

        // Restore to where the previous candidate was inserted, discarding
        // any auto-descended editing done since (index.py lines 820-826).
        path = insertionPath_;
        Container& container = resolveContainer(root, path);
        container.erase(container.begin() + static_cast<std::ptrdiff_t>(path.offset - 1));
        path.offset -= 1;
    }

    insertNode(root, path, candidates[static_cast<std::size_t>(nextIndex)], /*descendInto=*/false);

    // Record the pre-descend position for the *next* potential cycle before
    // applying this candidate's own auto-descend.
    insertionPath_ = path;
    lastCandidateIndex_ = nextIndex;
    lastTriggerId_ = triggerId;
    lastPressTime_ = now;

    const Container& container = resolveContainer(root, path);
    const Node& inserted = container[path.offset - 1];
    if (!inserted.args.empty()) {
        path.steps.push_back(PathStep{path.offset - 1, 0});
        path.offset = 0;
    }

    return nextIndex;
}

} // namespace mathclav::core::cursor
