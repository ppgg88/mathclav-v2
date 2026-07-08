#include <catch2/catch_test_macros.hpp>

#include "mathclav/core/ast/Node.h"
#include "mathclav/core/cursor/MultiChoiceCycle.h"

using namespace mathclav::core::ast;
using namespace mathclav::core::cursor;

namespace {
using Clock = std::chrono::steady_clock;

std::vector<Node> equalsFamily() {
    // Ported from index.py line 713.
    return {Node::symbol(L"="), Node::symbol(L"\\approx "), Node::symbol(L"\\neq ")};
}

constexpr int kEqualsTrigger = 1;
constexpr int kOtherTrigger = 2;
} // namespace

TEST_CASE("a fresh press inserts the first candidate", "[multichoice]") {
    Container root;
    CursorPath path;
    MultiChoiceCycleState state;

    const int index = state.press(root, path, kEqualsTrigger, equalsFamily(), Clock::now());

    REQUIRE(index == 0);
    REQUIRE(root.size() == 1);
    REQUIRE(root[0].literal == L"=");
    REQUIRE(path.offset == 1);
}

TEST_CASE("repressing the same trigger within the window cycles to the next candidate",
          "[multichoice]") {
    Container root;
    CursorPath path;
    MultiChoiceCycleState state;
    const Clock::time_point t0 = Clock::now();

    state.press(root, path, kEqualsTrigger, equalsFamily(), t0);
    const int second = state.press(root, path, kEqualsTrigger, equalsFamily(), t0 + std::chrono::milliseconds(200));

    REQUIRE(second == 1);
    REQUIRE(root.size() == 1); // replaced, not appended
    REQUIRE(root[0].literal == L"\\approx ");
    REQUIRE(path.offset == 1);
}

TEST_CASE("cycling wraps back to the first candidate after the last", "[multichoice]") {
    Container root;
    CursorPath path;
    MultiChoiceCycleState state;
    const Clock::time_point t0 = Clock::now();

    state.press(root, path, kEqualsTrigger, equalsFamily(), t0);
    state.press(root, path, kEqualsTrigger, equalsFamily(), t0 + std::chrono::milliseconds(200));
    const int third =
        state.press(root, path, kEqualsTrigger, equalsFamily(), t0 + std::chrono::milliseconds(400));

    REQUIRE(third == 2);
    REQUIRE(root[0].literal == L"\\neq ");

    const int fourth =
        state.press(root, path, kEqualsTrigger, equalsFamily(), t0 + std::chrono::milliseconds(600));

    REQUIRE(fourth == 0); // wrapped
    REQUIRE(root[0].literal == L"=");
}

TEST_CASE("a different trigger starts fresh instead of cycling", "[multichoice]") {
    Container root;
    CursorPath path;
    MultiChoiceCycleState state;
    const Clock::time_point t0 = Clock::now();

    state.press(root, path, kEqualsTrigger, equalsFamily(), t0);
    const int index =
        state.press(root, path, kOtherTrigger, equalsFamily(), t0 + std::chrono::milliseconds(200));

    REQUIRE(index == 0);
    REQUIRE(root.size() == 2); // appended, not replaced
}

TEST_CASE("pressing the same trigger after the cycle window has elapsed starts fresh",
          "[multichoice]") {
    Container root;
    CursorPath path;
    MultiChoiceCycleState state;
    const Clock::time_point t0 = Clock::now();

    state.press(root, path, kEqualsTrigger, equalsFamily(), t0);
    const int index =
        state.press(root, path, kEqualsTrigger, equalsFamily(), t0 + std::chrono::milliseconds(1600));

    REQUIRE(index == 0);
    REQUIRE(root.size() == 2); // appended, not replaced: the window had elapsed
}

TEST_CASE("cycling through composite candidates re-descends after each replace", "[multichoice]") {
    // Ported from index.py 850-855 combined with the `if self.rg !=
    // self.rg_prev_: pop back` correction (lines 820-824): Math mode's 'T'
    // key cycles cos/sin/tan, each of which auto-descends into its own
    // first slot on insert, so a repress must first discard that descent
    // before replacing.
    std::vector<Node> trigFamily = {Node::composite(Kind::Cos), Node::composite(Kind::Sin)};
    constexpr int kTrigTrigger = 3;

    Container root;
    CursorPath path;
    MultiChoiceCycleState state;
    const Clock::time_point t0 = Clock::now();

    state.press(root, path, kTrigTrigger, trigFamily, t0);
    REQUIRE(root.size() == 1);
    REQUIRE(root[0].kind == Kind::Cos);
    REQUIRE(path.steps.size() == 1); // auto-descended into cos's argument
    REQUIRE(path.steps[0] == PathStep{0, 0});
    REQUIRE(path.offset == 0);

    const int second = state.press(root, path, kTrigTrigger, trigFamily, t0 + std::chrono::milliseconds(200));

    REQUIRE(second == 1);
    REQUIRE(root.size() == 1); // cos replaced by sin, not appended alongside it
    REQUIRE(root[0].kind == Kind::Sin);
    REQUIRE(path.steps.size() == 1); // re-descended into sin's argument
    REQUIRE(path.steps[0] == PathStep{0, 0});
    REQUIRE(path.offset == 0);
}
