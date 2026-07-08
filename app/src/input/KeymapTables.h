#pragma once

#include "mathclav/core/ast/Node.h"

#include <QChar>

#include <vector>

namespace mathclav::app::input {

using mathclav::core::ast::Node;

// Math mode (index.py self.corespondance[2], lines 313-349): the candidate
// list to cycle through for a given A-Z letter. 'H' is handled separately
// by KeyDispatcher (opens the history browser in legacy, index.py 760-762)
// and is not part of this table.
//
// Dispatched by *produced character*, not by native scan code: legacy
// indexes this table by Windows VK code (`key - 65`), which looks
// physical-key-like at first glance, but WM_KEYDOWN's VK_A..VK_Z are
// actually assigned by the active keyboard layout to whichever key
// currently *produces* that letter (confirmed: on AZERTY, the physical
// key at the US-'A' scan code position delivers VK_Q, not VK_A) -- so
// `corespondance[2][key-65]` already means "the candidates for the key
// that types letter X", which is exactly QKeyEvent::text() on any
// platform/layout, no native code needed.
[[nodiscard]] std::vector<Node> mathModeCandidates(QChar letterUpper);

// Greek mode (index.py self.corespondance[0]/[1], lines 239-301): a single
// symbol per letter, chosen by the case of the produced character
// (index.py 797-801: `char.isupper()`).
[[nodiscard]] Node greekUppercase(QChar letterUpper);
[[nodiscard]] Node greekLowercase(QChar letterUpper);

} // namespace mathclav::app::input
