#include "input/PhysicalKey.h"

#include <QKeyEvent>
#include <QtGlobal>

namespace mathclav::app::input {

namespace {

#if defined(Q_OS_WIN)

// Direct reuse of the raw Windows Virtual-Key codes legacy checked
// (index.py key==191 / key==221): nativeVirtualKey() delivers exactly the
// same wParam value Tkinter's `event.keycode` reflected on Windows, so
// this is a literal port, not a reinterpretation.
constexpr quint32 kVkOem2Slash = 191; // VK_OEM_2, the '/' '?' key
constexpr quint32 kVkOem6Caret = 221; // VK_OEM_6, the ']' '}' key (AZERTY: circumflex dead-key)

PhysicalKey fromNative(quint32 nativeVirtualKey) {
    if (nativeVirtualKey == kVkOem2Slash) return PhysicalKey::Slash;
    if (nativeVirtualKey == kVkOem6Caret) return PhysicalKey::Caret;
    return PhysicalKey::Unknown;
}

#elif defined(Q_OS_LINUX)

// XKB keycodes (X11 and Wayland alike -- both are XKB-backed), empirically
// confirmed by dumping a live `setxkbmap fr` keymap with xmodmap -pke
// rather than derived from the AB/AD row-numbering convention by hand (an
// earlier version of this table used that convention directly and got
// both positions off by one -- AD11, not AD12, and keycode 60, not 61):
//   keycode 34 = dead_circumflex ...   (AD11: AZERTY's "^ ¨" dead key)
//   keycode 60 = colon slash ...       (the ":" "/" key -- '/' is the
//                                        shifted symbol, matching legacy's
//                                        own VK 191 being the "/" key's
//                                        physical counterpart, not
//                                        necessarily its unshifted symbol)
constexpr quint32 kXkbSlash = 60;
constexpr quint32 kXkbCaret = 34;

PhysicalKey fromNative(quint32 nativeScanCode) {
    if (nativeScanCode == kXkbSlash) return PhysicalKey::Slash;
    if (nativeScanCode == kXkbCaret) return PhysicalKey::Caret;
    return PhysicalKey::Unknown;
}

#else

PhysicalKey fromNative(quint32) {
    return PhysicalKey::Unknown;
}

#endif

} // namespace

PhysicalKey fromEvent(const QKeyEvent* event) {
#if defined(Q_OS_WIN)
    return fromNative(event->nativeVirtualKey());
#else
    return fromNative(event->nativeScanCode());
#endif
}

} // namespace mathclav::app::input
