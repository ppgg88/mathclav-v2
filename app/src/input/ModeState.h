#pragma once

namespace mathclav::app::input {

// Ported from legacy's self.mode (index.py): 0=Normal, 1=Math, 2=Greek.
enum class Mode {
    Normal,
    Math,
    Greek,
};

} // namespace mathclav::app::input
