#pragma once

#include "mathclav/core/ast/Node.h"

#include <cstddef>
#include <vector>

namespace mathclav::core::cursor {

using ast::Container;
using ast::Node;

// One "descend into an argument slot" step. Replaces legacy's four
// independently-mutated parallel arrays (index.py lines 79-90: `self.rg`,
// `self.result[]`, `self.cursor[]`, `self.i[]`, which must always agree by
// hand) with values that only ever mean one thing: nodeIndex is this
// composite Node's position in the *parent* container, slot is which of
// its argument args we've descended into.
struct PathStep {
    std::size_t nodeIndex = 0;
    std::size_t slot = 0;

    friend bool operator==(const PathStep&, const PathStep&) = default;
};

// Addresses an exact position in the document: `steps` is the chain of
// descents from the root (depth == legacy's `rg`), `offset` is the
// insertion/cursor position within the innermost container (== legacy's
// `cursor[rg]`).
struct CursorPath {
    std::vector<PathStep> steps;
    std::size_t offset = 0;

    friend bool operator==(const CursorPath&, const CursorPath&) = default;
};

// The single way to reach the container the cursor is currently in --
// unlike legacy, an invalid path fails a bounds check here instead of
// silently desynchronizing four separate arrays.
[[nodiscard]] Container& resolveContainer(Container& root, const CursorPath& path);
[[nodiscard]] const Container& resolveContainer(const Container& root, const CursorPath& path);

// The composite Node currently being edited into, i.e. the parent of the
// container path.steps.back() points into. Only callable when
// path.steps is non-empty (depth > 0).
[[nodiscard]] Node& parentNode(Container& root, const CursorPath& path);
[[nodiscard]] const Node& parentNode(const Container& root, const CursorPath& path);

} // namespace mathclav::core::cursor
