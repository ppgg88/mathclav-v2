# MathClav v2

Cross-platform (Linux/Windows) rewrite of MathClav — a keyboard-driven LaTeX
math editor for AZERTY users — in C++20 and Qt6. See `../mathclav/` for the
legacy Python/Tkinter version this replaces (kept only as a functional
reference, no code is shared).

## Prerequisites

- CMake >= 3.24
- A C++20 compiler (GCC/Clang on Linux, MSVC on Windows)
- Qt6 (Widgets + Charts modules), 6.4+
- Ninja (Linux dev/CI builds)

On Ubuntu/Pop!_OS:

```sh
sudo apt install qt6-base-dev qt6-base-dev-tools qt6-charts-dev ninja-build
```

## Build

```sh
cmake --preset dev-linux      # or dev-windows on Windows
cmake --build --preset dev-linux
ctest --preset ci-linux --test-dir build/dev-linux   # unit tests
```

The `mathclav` executable is produced under `build/dev-linux/app/`.

## Features

- Keyboard-driven LaTeX expression editing, optimized for AZERTY: dedicated
  Normal / Math / Greek input modes, cycling through related symbol
  candidates (e.g. `=` → `≈` → `≠` → ...) by pressing the same key again.
  Physical-key triggers (`/` → fraction, `^` → power/subscript) use the
  actual key position rather than the character it produces, matching
  AZERTY conventions.
- Live native LaTeX rendering via [MicroTeX](https://github.com/NanoMichael/MicroTeX)
  (no LaTeX installation required).
- Function grapher (QtCharts) with asymptote detection.
- History of saved expressions, light/dark theme, session autosave.
- Packaged for both Linux (AppImage) and Windows (NSIS installer).

## Status

Feature-complete: core expression tree/cursor engine, live rendering, all
three input modes with multi-choice cycling, grapher, history/settings/session
persistence, help/credits, and cross-platform packaging are all implemented
and tested. See the project plan for phase-by-phase implementation notes.
