#!/usr/bin/env bash
# Builds a portable MathClav-x86_64.AppImage from an already-built tree
# (expects the layout `cmake --preset ci-linux && cmake --build --preset
# ci-linux` produces: app/MathClav + app/res/, the MicroTeX resource
# directory app/CMakeLists.txt's own post-build step already deploys next
# to the binary for a normal dev build -- reused as-is here).
#
# Usage: packaging/linux/build-appimage.sh <build-dir> [output-dir]
#   e.g. packaging/linux/build-appimage.sh build/ci-linux dist
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"

BUILD_DIR="${1:?usage: build-appimage.sh <build-dir> [output-dir]}"
OUT_DIR="${2:-$REPO_ROOT/dist}"
BUILD_DIR="$(cd "$BUILD_DIR" && pwd)"
BINARY="$BUILD_DIR/app/MathClav"
RES_DIR="$BUILD_DIR/app/res"

if [[ ! -x "$BINARY" ]]; then
    echo "error: $BINARY not found or not executable -- build the project first" >&2
    exit 1
fi
if [[ ! -d "$RES_DIR" ]]; then
    echo "error: $RES_DIR not found -- MicroTeX's res/ wasn't deployed next to the binary" >&2
    exit 1
fi

TOOLS_DIR="$SCRIPT_DIR/.tools"
mkdir -p "$TOOLS_DIR"
LINUXDEPLOY="$TOOLS_DIR/linuxdeploy-x86_64.AppImage"
LINUXDEPLOY_QT="$TOOLS_DIR/linuxdeploy-plugin-qt-x86_64.AppImage"

fetch_if_missing() {
    local dest="$1" url="$2"
    if [[ ! -x "$dest" ]]; then
        echo "Downloading $(basename "$dest")..."
        curl -fL --retry 3 -o "$dest" "$url"
        chmod +x "$dest"
    fi
}

# Pinned to linuxdeploy's "continuous" release, the same one the AppImage
# project itself recommends CI consumers track.
fetch_if_missing "$LINUXDEPLOY" \
    "https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage"
fetch_if_missing "$LINUXDEPLOY_QT" \
    "https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage"

APPDIR="$BUILD_DIR/AppDir"
rm -rf "$APPDIR"
mkdir -p "$APPDIR/usr/bin" "$APPDIR/usr/share/applications" "$APPDIR/usr/share/icons/hicolor/256x256/apps"

cp "$BINARY" "$APPDIR/usr/bin/mathclav"
cp -r "$RES_DIR" "$APPDIR/usr/bin/res"
cp "$SCRIPT_DIR/mathclav.desktop" "$APPDIR/usr/share/applications/"
cp "$SCRIPT_DIR/mathclav.png" "$APPDIR/usr/share/icons/hicolor/256x256/apps/"

mkdir -p "$OUT_DIR"
OUT_DIR="$(cd "$OUT_DIR" && pwd)"
# linuxdeploy discovers the "qt" plugin by searching PATH for an
# executable named linuxdeploy-plugin-qt* (prefix match, so the
# version-suffixed AppImage filename is found as-is).
# --appimage-extract-and-run: linuxdeploy's own AppImages are FUSE-mounted
# by default, which isn't available in every sandboxed/CI environment.
export PATH="$TOOLS_DIR:$PATH"
export QMAKE="$(command -v qmake6 || command -v qmake)"
cd "$OUT_DIR"
"$LINUXDEPLOY" --appimage-extract-and-run \
    --appdir "$APPDIR" \
    --executable "$APPDIR/usr/bin/mathclav" \
    --desktop-file "$SCRIPT_DIR/mathclav.desktop" \
    --icon-file "$SCRIPT_DIR/mathclav.png" \
    --plugin qt \
    --output appimage

echo "Built: $(find "$OUT_DIR" -maxdepth 1 -iname '*.AppImage')"
