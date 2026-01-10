#!/bin/bash
# Flash the compiled program

# Get the directory where this script is located
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Detect operating system and architecture
OS="$(uname -s)"
ARCH="$(uname -m)"

case "$OS" in
    Darwin*)
        case "$ARCH" in
            arm64|aarch64)
                MINICHLINK="$SCRIPT_DIR/minichlink/aarch64-macos/minichlink"
                ;;
            x86_64)
                MINICHLINK="$SCRIPT_DIR/minichlink/x86_64-macos/minichlink"
                ;;
            *)
                echo "Unsupported architecture: $ARCH"
                exit 1
                ;;
        esac
        ;;
    Linux*)
        case "$ARCH" in
            aarch64)
                MINICHLINK="$SCRIPT_DIR/minichlink/aarch64-linux-gnu/minichlink"
                ;;
            x86_64)
                MINICHLINK="$SCRIPT_DIR/minichlink/x86_64-linux-gnu/minichlink"
                ;;
            *)
                echo "Unsupported architecture: $ARCH"
                exit 1
                ;;
        esac
        ;;
    *)
        echo "Unsupported operating system: $OS"
        exit 1
        ;;
esac

# Check if minichlink exists
if [ ! -x "$MINICHLINK" ]; then
    echo "Error: minichlink not found at $MINICHLINK"
    exit 1
fi

"$MINICHLINK" -E -w *.bin 0x08000000

if [ $? -ne 0 ]; then
    read -p "Press any key to continue..."
fi