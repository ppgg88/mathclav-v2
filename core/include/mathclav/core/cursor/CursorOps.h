#pragma once

#include "mathclav/core/cursor/CursorPath.h"

namespace mathclav::core::cursor {

// Pure (document, path) -> mutated (document, path) operations, each ported
// from a specific branch of legacy's action()/multiple_choice()
// (mathclav/index.py 490-861), cited per function below. One documented
// behavior change throughout: legacy's moveDown relies on Python's
// negative-index wraparound (`self.result[self.rg-1]` aliasing
// `self.result[0]` when rg==0) to no-op at document root; here that's an
// explicit no-op instead of an accidental one. deleteBackward fixes a
// genuine legacy crash (see CursorOps.cpp) rather than reproducing it.

// Right arrow (index.py 565-588): advances within the current container;
// auto-descends into a composite node's first slot when stepping over it;
// at the end of a nested slot, pops out (last slot) or advances to the
// composite's next sibling slot.
void moveRight(const Container& root, CursorPath& path);

// Left arrow (index.py 607-628): mirror of moveRight. Descending into a
// composite lands at the *end* of its *last* slot.
void moveLeft(const Container& root, CursorPath& path);

// Down arrow (index.py 544-563): jumps to the next sibling slot of the
// composite currently being edited (e.g. numerator -> denominator),
// landing at that slot's end. No-op at document root.
void moveDown(const Container& root, CursorPath& path);

// Up arrow (index.py 590-605): mirror of moveDown.
void moveUp(const Container& root, CursorPath& path);

// Inserts `node` at the cursor and advances past it. If `node` is
// composite (non-empty args) and `descendInto` is true (the default),
// auto-descends into its first slot -- ported from multiple_choice's
// single-candidate path plus the `if inser.imax>=0 and into` auto-descend
// (index.py 806-855). `descendInto=false` exists for MultiChoiceCycle,
// mirroring the `into` parameter legacy's '/' fraction-conversion special
// case passes (index.py line 772).
void insertNode(Container& root, CursorPath& path, Node node, bool descendInto = true);

// Backspace (index.py 630-662). At offset 0 inside a slot (depth > 0),
// deletes the *entire enclosing composite* -- not just "step out": an
// empty argument's Backspace removes the whole construct, discarding any
// content already typed into its other args, exactly as legacy does.
// Otherwise, if the node immediately left of the cursor has
// autoDescendOnDelete (Integral/Sum), tunnels into its last slot first so
// deletion continues inside it; else deletes that node directly.
void deleteBackward(Container& root, CursorPath& path);

// Delete (index.py 664-671): removes the node immediately right of the
// cursor. No-op past the end of the container.
void deleteForward(Container& root, CursorPath& path);

} // namespace mathclav::core::cursor
