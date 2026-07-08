#include "mathclav/core/cursor/CursorPath.h"

namespace mathclav::core::cursor {

Container& resolveContainer(Container& root, const CursorPath& path) {
    Container* current = &root;
    for (const PathStep& step : path.steps) {
        current = &current->at(step.nodeIndex).args.at(step.slot);
    }
    return *current;
}

const Container& resolveContainer(const Container& root, const CursorPath& path) {
    const Container* current = &root;
    for (const PathStep& step : path.steps) {
        current = &current->at(step.nodeIndex).args.at(step.slot);
    }
    return *current;
}

Node& parentNode(Container& root, const CursorPath& path) {
    CursorPath parentPath = path;
    const PathStep last = parentPath.steps.back();
    parentPath.steps.pop_back();
    return resolveContainer(root, parentPath).at(last.nodeIndex);
}

const Node& parentNode(const Container& root, const CursorPath& path) {
    CursorPath parentPath = path;
    const PathStep last = parentPath.steps.back();
    parentPath.steps.pop_back();
    return resolveContainer(root, parentPath).at(last.nodeIndex);
}

} // namespace mathclav::core::cursor
