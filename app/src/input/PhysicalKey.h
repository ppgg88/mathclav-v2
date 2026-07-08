#pragma once

class QKeyEvent;

namespace mathclav::app::input {

// Math mode has exactly two triggers that legacy identifies by raw
// Windows Virtual-Key code rather than by produced character (index.py
// action(), key==191 for '/' at line 766, key==221 for '^' at line 780):
// everything else in the app -- including the A-Z letter tables -- turns
// out to already be character-based once traced through to how Windows
// actually assigns VK codes (see KeymapTables.h for that finding), so this
// enum stays deliberately narrow instead of growing into a full A-Z
// physical-key table.
//
// Slash: the ":"/"/" key, used to convert the node just before the cursor
// into the numerator of a new fraction.
// Caret: the AZERTY dead-key that types the circumflex accent (physically
// where a US layout has '['), used to trigger the power/subscript choice.
enum class PhysicalKey {
    Unknown,
    Slash,
    Caret,
};

[[nodiscard]] PhysicalKey fromEvent(const QKeyEvent* event);

} // namespace mathclav::app::input
