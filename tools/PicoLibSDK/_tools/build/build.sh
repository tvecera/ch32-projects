#!/bin/bash
# Build elf2uf2 and LoaderCrc for multiple platforms

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TOOLS_DIR="$(dirname "$SCRIPT_DIR")"

# Create output directories
mkdir -p "$TOOLS_DIR/aarch64-linux-gnu"
mkdir -p "$TOOLS_DIR/x86_64-linux-gnu"
mkdir -p "$TOOLS_DIR/x86_64-macos"
mkdir -p "$TOOLS_DIR/aarch64-macos"

echo "=== Building Docker images for each platform ==="
echo "Building amd64 image (this may take a while with QEMU)..."
docker build --platform linux/amd64 -t picolib-tools-builder-amd64 "$SCRIPT_DIR"
echo "Building arm64 image..."
docker build --platform linux/arm64 -t picolib-tools-builder-arm64 "$SCRIPT_DIR"

echo ""
echo "=== Building for x86_64-linux-gnu (using QEMU emulation) ==="
docker run --rm --platform linux/amd64 -v "$SCRIPT_DIR:/build" picolib-tools-builder-amd64 bash -c "
    g++ -O2 -static -o /build/elf2uf2/elf2uf2-x86_64 /build/elf2uf2/main.cpp
    g++ -O2 -static -o /build/PicoPadLoaderCrc/LoaderCrc-x86_64 /build/PicoPadLoaderCrc/LoaderCrc.cpp
"
cp "$SCRIPT_DIR/elf2uf2/elf2uf2-x86_64" "$TOOLS_DIR/x86_64-linux-gnu/elf2uf2"
cp "$SCRIPT_DIR/PicoPadLoaderCrc/LoaderCrc-x86_64" "$TOOLS_DIR/x86_64-linux-gnu/LoaderCrc"
rm -f "$SCRIPT_DIR/elf2uf2/elf2uf2-x86_64" "$SCRIPT_DIR/PicoPadLoaderCrc/LoaderCrc-x86_64"
echo "Done: x86_64-linux-gnu"

echo ""
echo "=== Building for aarch64-linux-gnu ==="
docker run --rm --platform linux/arm64 -v "$SCRIPT_DIR:/build" picolib-tools-builder-arm64 bash -c "
    g++ -O2 -static -o /build/elf2uf2/elf2uf2-aarch64 /build/elf2uf2/main.cpp
    g++ -O2 -static -o /build/PicoPadLoaderCrc/LoaderCrc-aarch64 /build/PicoPadLoaderCrc/LoaderCrc.cpp
"
cp "$SCRIPT_DIR/elf2uf2/elf2uf2-aarch64" "$TOOLS_DIR/aarch64-linux-gnu/elf2uf2"
cp "$SCRIPT_DIR/PicoPadLoaderCrc/LoaderCrc-aarch64" "$TOOLS_DIR/aarch64-linux-gnu/LoaderCrc"
rm -f "$SCRIPT_DIR/elf2uf2/elf2uf2-aarch64" "$SCRIPT_DIR/PicoPadLoaderCrc/LoaderCrc-aarch64"
echo "Done: aarch64-linux-gnu"

echo ""
echo "=== Building for macOS ==="
# Detect current macOS architecture
if [[ "$(uname -s)" == "Darwin" ]]; then
    CURRENT_ARCH="$(uname -m)"

    # Build for current architecture
    echo "Building for current architecture: $CURRENT_ARCH"
    if [[ "$CURRENT_ARCH" == "arm64" ]]; then
        TARGET_DIR="$TOOLS_DIR/aarch64-macos"
    else
        TARGET_DIR="$TOOLS_DIR/x86_64-macos"
    fi

    g++ -O2 -o "$TARGET_DIR/elf2uf2" "$SCRIPT_DIR/elf2uf2/main.cpp"
    g++ -O2 -o "$TARGET_DIR/LoaderCrc" "$SCRIPT_DIR/PicoPadLoaderCrc/LoaderCrc.cpp"
    echo "Done: $CURRENT_ARCH-macos"

    # Build for the other architecture using clang cross-compilation
    if [[ "$CURRENT_ARCH" == "arm64" ]]; then
        echo "Building for x86_64 using clang cross-compilation..."
        clang++ -O2 -arch x86_64 -o "$TOOLS_DIR/x86_64-macos/elf2uf2" "$SCRIPT_DIR/elf2uf2/main.cpp"
        clang++ -O2 -arch x86_64 -o "$TOOLS_DIR/x86_64-macos/LoaderCrc" "$SCRIPT_DIR/PicoPadLoaderCrc/LoaderCrc.cpp"
        echo "Done: x86_64-macos (cross-compiled)"
    else
        echo "Building for arm64 using clang cross-compilation..."
        clang++ -O2 -arch arm64 -o "$TOOLS_DIR/aarch64-macos/elf2uf2" "$SCRIPT_DIR/elf2uf2/main.cpp"
        clang++ -O2 -arch arm64 -o "$TOOLS_DIR/aarch64-macos/LoaderCrc" "$SCRIPT_DIR/PicoPadLoaderCrc/LoaderCrc.cpp"
        echo "Done: arm64-macos (cross-compiled)"
    fi
else
    echo "Skipping macOS builds (not running on macOS)"
fi

echo ""
echo "=== Build complete ==="
echo "Output directories:"
ls -la "$TOOLS_DIR"/*/
