#include "mathclav/core/cursor/CursorOps.h"

#include "mathclav/core/ast/NodeCatalog.h"

namespace mathclav::core::cursor {

void moveRight(const Container& root, CursorPath& path) {
    const Container& container = resolveContainer(root, path);
    if (path.offset < container.size()) {
        const Node& passed = container[path.offset];
        path.offset += 1;
        if (!passed.args.empty()) {
            path.steps.push_back(PathStep{path.offset - 1, 0});
            path.offset = 0;
        }
        return;
    }
    if (path.steps.empty()) {
        return; // end of document root: no-op
    }
    const std::size_t currentSlot = path.steps.back().slot;
    const Node& parent = parentNode(root, path);
    if (currentSlot + 1 >= parent.args.size()) {
        // Last slot: pop out, positioned right *after* the composite. The
        // popped step's nodeIndex is the composite's position in the parent
        // container, so the parent offset is nodeIndex + 1 -- path.offset
        // otherwise still holds the (now-irrelevant) child container's
        // offset, not the parent's.
        const std::size_t nodeIndex = path.steps.back().nodeIndex;
        path.steps.pop_back();
        path.offset = nodeIndex + 1;
    } else {
        path.steps.back().slot = currentSlot + 1;
        path.offset = 0;
    }
}

void moveLeft(const Container& root, CursorPath& path) {
    const Container& container = resolveContainer(root, path);
    if (path.offset > 0) {
        const Node& before = container[path.offset - 1];
        if (!before.args.empty()) {
            const std::size_t lastSlot = before.args.size() - 1;
            path.steps.push_back(PathStep{path.offset - 1, lastSlot});
            path.offset = before.args[lastSlot].size();
        } else {
            path.offset -= 1;
        }
        return;
    }
    if (path.steps.empty()) {
        return;
    }
    const std::size_t currentSlot = path.steps.back().slot;
    if (currentSlot == 0) {
        // Pop out, positioned right *before* the composite -- i.e. at the
        // popped step's nodeIndex itself, not decremented from the (stale)
        // child offset like a naive port of legacy's `cursor[rg] -= 1`
        // would do (that decrements the *parent's own* stored offset, which
        // this path model doesn't keep a separate slot for once descended).
        const std::size_t nodeIndex = path.steps.back().nodeIndex;
        path.steps.pop_back();
        path.offset = nodeIndex;
    } else {
        const Node& parent = parentNode(root, path);
        const std::size_t prevSlot = currentSlot - 1;
        path.offset = parent.args[prevSlot].size();
        path.steps.back().slot = prevSlot;
    }
}

void moveDown(const Container& root, CursorPath& path) {
    if (path.steps.empty()) {
        return;
    }
    const Node& parent = parentNode(root, path);
    const std::size_t currentSlot = path.steps.back().slot;
    const std::size_t lastSlotIndex = parent.args.size() - 1;
    if (currentSlot < lastSlotIndex) {
        const std::size_t nextSlot = currentSlot + 1;
        path.offset = parent.args[nextSlot].size();
        path.steps.back().slot = nextSlot;
    } else {
        // Last slot: pop out, positioned right after the composite (see the
        // matching moveRight comment for why nodeIndex + 1 is needed here).
        const std::size_t nodeIndex = path.steps.back().nodeIndex;
        path.steps.pop_back();
        path.offset = nodeIndex + 1;
    }
}

void moveUp(const Container& root, CursorPath& path) {
    if (path.steps.empty()) {
        return;
    }
    const std::size_t currentSlot = path.steps.back().slot;
    if (currentSlot > 0) {
        const Node& parent = parentNode(root, path);
        const std::size_t prevSlot = currentSlot - 1;
        path.offset = parent.args[prevSlot].size();
        path.steps.back().slot = prevSlot;
    } else {
        // Pop out AND step back before the composite (see the matching
        // moveLeft comment for why nodeIndex, not a decrement, is needed).
        const std::size_t nodeIndex = path.steps.back().nodeIndex;
        path.steps.pop_back();
        path.offset = nodeIndex;
    }
}

void insertNode(Container& root, CursorPath& path, Node node, bool descendInto) {
    Container& container = resolveContainer(root, path);
    const bool isComposite = !node.args.empty();
    const std::size_t insertedAt = path.offset;
    container.insert(container.begin() + static_cast<std::ptrdiff_t>(path.offset), std::move(node));
    path.offset += 1;
    if (isComposite && descendInto) {
        path.steps.push_back(PathStep{insertedAt, 0});
        path.offset = 0;
    }
}

void deleteBackward(Container& root, CursorPath& path) {
    if (path.offset == 0 && !path.steps.empty()) {
        // Delete the whole enclosing composite (index.py 632-638), not just
        // step out of it.
        const PathStep last = path.steps.back();
        path.steps.pop_back();
        Container& parentContainer = resolveContainer(root, path);
        parentContainer.erase(parentContainer.begin() + static_cast<std::ptrdiff_t>(last.nodeIndex));
        path.offset = last.nodeIndex;
        return;
    }
    if (path.offset == 0) {
        return; // document root, nothing to the left: no-op
    }

    Container& container = resolveContainer(root, path);
    const Node& before = container[path.offset - 1];
    if (ast::spec(before.kind).autoDescendOnDelete) {
        // Integral/Sum (index.py's `supr_opt` branch, 649-657): tunnel into
        // the last slot so Backspace edits inside it rather than deleting
        // the whole node outright.
        //
        // Fix over legacy: if that last slot is already empty, legacy
        // unconditionally does `content.pop(cursor[rg]-1)`, which under
        // Python's negative indexing means `content.pop(-1)` on an *empty*
        // list -- an uncaught IndexError that skips the render call and
        // leaves rg/cursor/i desynchronized from what's displayed. Here we
        // simply leave the cursor tunnelled into the empty slot: this
        // keypress does nothing visible, and the *next* Backspace correctly
        // falls into the branch above and deletes the whole composite.
        const std::size_t lastSlot = before.args.size() - 1;
        path.steps.push_back(PathStep{path.offset - 1, lastSlot});
        path.offset = before.args[lastSlot].size();
        if (path.offset > 0) {
            Container& innerContainer = resolveContainer(root, path);
            innerContainer.erase(innerContainer.begin() + static_cast<std::ptrdiff_t>(path.offset - 1));
            path.offset -= 1;
        }
        return;
    }

    container.erase(container.begin() + static_cast<std::ptrdiff_t>(path.offset - 1));
    path.offset -= 1;
}

void deleteForward(Container& root, CursorPath& path) {
    Container& container = resolveContainer(root, path);
    if (path.offset < container.size()) {
        container.erase(container.begin() + static_cast<std::ptrdiff_t>(path.offset));
    }
}

} // namespace mathclav::core::cursor
