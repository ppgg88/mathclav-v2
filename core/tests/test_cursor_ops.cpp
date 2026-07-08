#include <catch2/catch_test_macros.hpp>

#include "mathclav/core/ast/Node.h"
#include "mathclav/core/cursor/CursorOps.h"

using namespace mathclav::core::ast;
using namespace mathclav::core::cursor;

namespace {

// Builds "A[frac 1/2]B" as the root document, i.e. Symbol(A), Fraction(1,2),
// Symbol(B) -- the standard fixture used across these tests to exercise
// descending into / out of a two-slot composite.
Container makeFixture() {
    Container root;
    root.push_back(Node::symbol(L"A"));

    Node frac = Node::composite(Kind::Fraction);
    frac.args[0].push_back(Node::symbol(L"1"));
    frac.args[1].push_back(Node::symbol(L"2"));
    root.push_back(std::move(frac));

    root.push_back(Node::symbol(L"B"));
    return root;
}

} // namespace

// --- moveRight (index.py 565-588) ------------------------------------------

TEST_CASE("moveRight over a plain symbol just advances", "[cursor]") {
    Container root = makeFixture();
    CursorPath path; // offset 0, before "A"
    moveRight(root, path);
    REQUIRE(path.steps.empty());
    REQUIRE(path.offset == 1); // now after "A", before the fraction
}

TEST_CASE("moveRight over a composite auto-descends into its first slot", "[cursor]") {
    Container root = makeFixture();
    CursorPath path{{}, 1}; // positioned right before the fraction
    moveRight(root, path);
    REQUIRE(path.steps.size() == 1);
    REQUIRE(path.steps[0] == PathStep{1, 0}); // fraction is at index 1, slot 0 (numerator)
    REQUIRE(path.offset == 0);
}

TEST_CASE("moveRight out of the last slot pops out without moving the parent offset", "[cursor]") {
    Container root = makeFixture();
    // Inside the fraction's denominator (slot 1), positioned after its "2".
    CursorPath path{{PathStep{1, 1}}, 1};
    moveRight(root, path);
    REQUIRE(path.steps.empty());
    REQUIRE(path.offset == 2); // right after the fraction, unchanged
}

TEST_CASE("moveRight out of a non-last slot advances to the next sibling slot", "[cursor]") {
    Container root = makeFixture();
    // Inside the numerator (slot 0), positioned after its "1".
    CursorPath path{{PathStep{1, 0}}, 1};
    moveRight(root, path);
    REQUIRE(path.steps.size() == 1);
    REQUIRE(path.steps[0] == PathStep{1, 1});
    REQUIRE(path.offset == 0); // start of the denominator
}

TEST_CASE("moveRight at the end of the document root is a no-op", "[cursor]") {
    Container root = makeFixture();
    CursorPath path{{}, 3};
    moveRight(root, path);
    REQUIRE(path.steps.empty());
    REQUIRE(path.offset == 3);
}

// --- moveLeft (index.py 607-628) --------------------------------------------

TEST_CASE("moveLeft over a composite descends into the END of its LAST slot", "[cursor]") {
    Container root = makeFixture();
    CursorPath path{{}, 2}; // right after the fraction
    moveLeft(root, path);
    REQUIRE(path.steps.size() == 1);
    REQUIRE(path.steps[0] == PathStep{1, 1}); // last slot (denominator)
    REQUIRE(path.offset == 1);                // end of "2"
}

TEST_CASE("moveLeft out of slot 0 at offset 0 pops out AND steps back before the composite",
          "[cursor]") {
    Container root = makeFixture();
    CursorPath path{{PathStep{1, 0}}, 0}; // start of the numerator
    moveLeft(root, path);
    REQUIRE(path.steps.empty());
    REQUIRE(path.offset == 1); // before the fraction, not after
}

TEST_CASE("moveLeft out of a non-zero slot at offset 0 jumps to the previous sibling's end",
          "[cursor]") {
    Container root = makeFixture();
    CursorPath path{{PathStep{1, 1}}, 0}; // start of the denominator
    moveLeft(root, path);
    REQUIRE(path.steps.size() == 1);
    REQUIRE(path.steps[0] == PathStep{1, 0});
    REQUIRE(path.offset == 1); // end of the numerator's "1"
}

// --- moveUp/moveDown (index.py 544-563, 590-605) ----------------------------

TEST_CASE("moveDown at document root is an explicit no-op", "[cursor]") {
    // Legacy relies on Python's negative-index wraparound here
    // (`self.result[self.rg-1]` aliasing `self.result[0]` when rg==0); this
    // is a deliberate behavior fix, not a port.
    Container root = makeFixture();
    CursorPath path{{}, 1};
    moveDown(root, path);
    REQUIRE(path.steps.empty());
    REQUIRE(path.offset == 1);
}

TEST_CASE("moveUp at document root is a no-op", "[cursor]") {
    Container root = makeFixture();
    CursorPath path{{}, 1};
    moveUp(root, path);
    REQUIRE(path.steps.empty());
    REQUIRE(path.offset == 1);
}

TEST_CASE("moveDown jumps from numerator to denominator, landing at its end", "[cursor]") {
    Container root = makeFixture();
    CursorPath path{{PathStep{1, 0}}, 1};
    moveDown(root, path);
    REQUIRE(path.steps[0] == PathStep{1, 1});
    REQUIRE(path.offset == 1);
}

TEST_CASE("moveDown from the last slot pops out without moving the parent offset", "[cursor]") {
    Container root = makeFixture();
    CursorPath path{{PathStep{1, 1}}, 1};
    moveDown(root, path);
    REQUIRE(path.steps.empty());
    // "Without moving the parent offset" means the parent's offset keeps
    // its already-correct invariant value (nodeIndex + 1, set when we
    // originally descended) -- not that path.offset (which was tracking
    // the *child* container while nested) is left unchanged verbatim.
    REQUIRE(path.offset == 2);
}

TEST_CASE("moveUp jumps from denominator to numerator, landing at its end", "[cursor]") {
    Container root = makeFixture();
    CursorPath path{{PathStep{1, 1}}, 1};
    moveUp(root, path);
    REQUIRE(path.steps[0] == PathStep{1, 0});
    REQUIRE(path.offset == 1);
}

TEST_CASE("moveUp from slot 0 pops out AND steps back before the composite", "[cursor]") {
    Container root = makeFixture();
    CursorPath path{{PathStep{1, 0}}, 1};
    moveUp(root, path);
    REQUIRE(path.steps.empty());
    REQUIRE(path.offset == 1); // parent offset decremented, matching legacy 599-605
}

// --- insertNode (multiple_choice, index.py 806-855) -------------------------

TEST_CASE("insertNode inserts a symbol at the cursor and advances", "[cursor]") {
    Container root;
    CursorPath path;
    insertNode(root, path, Node::symbol(L"x"));
    REQUIRE(root.size() == 1);
    REQUIRE(root[0].literal == L"x");
    REQUIRE(path.offset == 1);
    REQUIRE(path.steps.empty());
}

TEST_CASE("insertNode auto-descends into a freshly-inserted composite's first slot", "[cursor]") {
    Container root;
    CursorPath path;
    insertNode(root, path, Node::composite(Kind::Fraction));
    REQUIRE(root.size() == 1);
    REQUIRE(path.steps.size() == 1);
    REQUIRE(path.steps[0] == PathStep{0, 0});
    REQUIRE(path.offset == 0);
}

TEST_CASE("insertNode with descendInto=false does not descend", "[cursor]") {
    Container root;
    CursorPath path;
    insertNode(root, path, Node::composite(Kind::Sqrt), /*descendInto=*/false);
    REQUIRE(path.steps.empty());
    REQUIRE(path.offset == 1);
}

// --- deleteBackward (index.py 630-662) --------------------------------------

TEST_CASE("deleteBackward at offset 0 inside a slot deletes the WHOLE enclosing composite",
          "[cursor]") {
    // Not just "step out": the fraction (with its numerator "1" and
    // denominator "2") is removed entirely, matching index.py 632-638.
    Container root = makeFixture();
    CursorPath path{{PathStep{1, 0}}, 0}; // start of the numerator
    deleteBackward(root, path);
    REQUIRE(path.steps.empty());
    REQUIRE(root.size() == 2); // "A", "B" -- the fraction is gone entirely
    REQUIRE(root[0].literal == L"A");
    REQUIRE(root[1].literal == L"B");
    REQUIRE(path.offset == 1);
}

TEST_CASE("deleteBackward with no node to the left at document root is a no-op", "[cursor]") {
    Container root = makeFixture();
    CursorPath path;
    deleteBackward(root, path);
    REQUIRE(root.size() == 3);
    REQUIRE(path.offset == 0);
}

TEST_CASE("deleteBackward on a plain node just removes it", "[cursor]") {
    Container root = makeFixture();
    CursorPath path{{}, 1}; // right after "A"
    deleteBackward(root, path);
    REQUIRE(root.size() == 2);
    REQUIRE(root[0].kind == Kind::Fraction);
    REQUIRE(path.offset == 0);
}

TEST_CASE("deleteBackward on a filled Sum edits inside its last slot instead of deleting it",
          "[cursor]") {
    // Ported from index.py's `supr_opt` branch (649-657): Sum has
    // autoDescendOnDelete, so Backspace right after it edits its last slot.
    Container root;
    Node sum = Node::composite(Kind::Sum);
    sum.args[2].push_back(Node::symbol(L"k"));    // body (last slot)
    root.push_back(std::move(sum));
    CursorPath path{{}, 1}; // right after the sum

    deleteBackward(root, path);

    REQUIRE(root.size() == 1); // the Sum node itself is still there
    REQUIRE(root[0].kind == Kind::Sum);
    REQUIRE(root[0].args[2].empty()); // its last slot's "k" was deleted
    REQUIRE(path.steps.size() == 1);   // cursor tunnelled into the last slot
    REQUIRE(path.steps[0] == PathStep{0, 2});
    REQUIRE(path.offset == 0);
}

TEST_CASE("deleteBackward on an EMPTY Sum's last slot tunnels in without crashing", "[cursor]") {
    // Fixes a genuine legacy bug: with an empty last slot, legacy's
    // `content.pop(cursor[rg]-1)` becomes `content.pop(-1)` on an empty
    // Python list (an uncaught IndexError that skips the render call and
    // desyncs displayed state from internal state). Here this keypress is
    // simply absorbed by the tunnel: no crash, no corruption, and the
    // *next* Backspace correctly deletes the whole Sum (see the following
    // test).
    Container root;
    root.push_back(Node::composite(Kind::Sum)); // all three args empty
    CursorPath path{{}, 1};

    deleteBackward(root, path);

    REQUIRE(root.size() == 1);
    REQUIRE(path.steps.size() == 1);
    REQUIRE(path.steps[0] == PathStep{0, 2});
    REQUIRE(path.offset == 0);

    // A second Backspace now hits the offset==0/depth>0 branch and removes
    // the whole (still-empty) Sum.
    deleteBackward(root, path);
    REQUIRE(root.empty());
    REQUIRE(path.steps.empty());
    REQUIRE(path.offset == 0);
}

// --- deleteForward (index.py 664-671) ---------------------------------------

TEST_CASE("deleteForward removes the node immediately right of the cursor", "[cursor]") {
    Container root = makeFixture();
    CursorPath path{{}, 0};
    deleteForward(root, path);
    REQUIRE(root.size() == 2);
    REQUIRE(root[0].kind == Kind::Fraction);
    REQUIRE(path.offset == 0);
}

TEST_CASE("deleteForward past the end of the container is a no-op", "[cursor]") {
    Container root = makeFixture();
    CursorPath path{{}, 3};
    deleteForward(root, path);
    REQUIRE(root.size() == 3);
}
