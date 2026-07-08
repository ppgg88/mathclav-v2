#include "mathclav/core/ast/Node.h"

#include "mathclav/core/ast/NodeCatalog.h"

#include <cassert>

namespace mathclav::core::ast {

namespace {

// Ported from the *intent* of matrice()/system() in latex.py (795-854), not
// their literal code: matrice() there is dead/broken (hardcodes exactly 2
// args regardless of n*m, its row loop starts at i=1 skipping row 0, and
// __str__ never substitutes into the template at all -- and it isn't wired
// to any keyboard shortcut in index.py either, so it's unreachable). system()
// is sound and reachable-in-principle, so both kinds are built the way
// system() actually works: rows*cols args, one placeholder per cell.
std::wstring buildGridTemplate(Kind kind, int rows, int cols) {
    assert(rows > 0 && cols > 0 && rows * cols <= kMaxSlots);
    const wchar_t* begin = kind == Kind::Matrix ? L"\\begin{pmatrix}" : L"\\left\\{\\begin{matrix}";
    const wchar_t* end = kind == Kind::Matrix ? L"\\end{pmatrix}" : L"\\end{matrix}\\right.";

    std::wstring out = begin;
    int slotIndex = 0;
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            out += placeholderFor(slotIndex++);
            if (c != cols - 1) {
                out += L" &";
            } else if (r != rows - 1) {
                out += L"\\\\";
            }
        }
    }
    out += end;
    return out;
}

} // namespace

Node Node::symbol(std::wstring literal) {
    Node n;
    n.kind = Kind::Symbol;
    n.literal = std::move(literal);
    return n;
}

Node Node::composite(Kind kind) {
    assert(kind != Kind::Matrix && kind != Kind::System &&
           "use Node::makeMatrix/makeSystem for grid kinds");
    Node n;
    n.kind = kind;
    const auto& s = spec(kind);
    n.args.assign(static_cast<std::size_t>(s.arity), Container{});
    return n;
}

Node Node::makeMatrix(int rows, int cols) {
    Node n;
    n.kind = Kind::Matrix;
    n.grid = GridDims{rows, cols};
    n.args.assign(static_cast<std::size_t>(rows) * static_cast<std::size_t>(cols), Container{});
    n.dynamicTemplate = buildGridTemplate(Kind::Matrix, rows, cols);
    return n;
}

Node Node::makeSystem(int rows, int cols) {
    Node n;
    n.kind = Kind::System;
    n.grid = GridDims{rows, cols};
    n.args.assign(static_cast<std::size_t>(rows) * static_cast<std::size_t>(cols), Container{});
    n.dynamicTemplate = buildGridTemplate(Kind::System, rows, cols);
    return n;
}

} // namespace mathclav::core::ast
