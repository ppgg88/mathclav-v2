#include "mathclav/core/latex/CaretSplice.h"

#include "mathclav/core/latex/LatexSerializer.h"

namespace mathclav::core::latex {

using ast::Container;
using ast::Node;
using cursor::CursorPath;
using cursor::resolveContainer;

std::wstring renderWithCaret(const Container& document, const CursorPath& path) {
    Container copy = document; // deep copy: never mutate the real document

    Container& innermost = resolveContainer(copy, path);
    innermost.insert(innermost.begin() + static_cast<std::ptrdiff_t>(path.offset),
                      Node::symbol(std::wstring(1, kCaretGlyph)));

    if (!path.steps.empty()) {
        CursorPath parentPath = path;
        const std::size_t nodeIndex = parentPath.steps.back().nodeIndex;
        parentPath.steps.pop_back();
        Container& parentContainer = resolveContainer(copy, parentPath);
        parentContainer.insert(parentContainer.begin() + static_cast<std::ptrdiff_t>(nodeIndex + 1),
                                Node::symbol(kSlotBoundaryMarker));
    }

    return render(copy);
}

} // namespace mathclav::core::latex
