#!/usr/bin/env python3
"""
PNG to Pattern Converter
Converts a PNG image to C array format for use in pattern headers.

Usage:
  python3 convert.py <input.png> [output_name]

The PNG must be:
- 1-bit (black/white) or will be converted (threshold at 128)
- Correct size for the target VMODE (128x64 or 160x120)
"""

import sys
import os

def png_to_c_array(png_path, array_name=None):
    """Convert PNG to C array format"""
    try:
        from PIL import Image
    except ImportError:
        print("Error: PIL not installed. Run: pip3 install Pillow")
        return None

    # Load image
    img = Image.open(png_path)
    width, height = img.size

    # Determine VMODE from dimensions
    if width == 128 and height == 64:
        vmode = 0
    elif width == 160 and height == 120:
        vmode = 1
    else:
        print(f"Error: Unsupported image size {width}x{height}")
        print("  VMODE 0: 128x64")
        print("  VMODE 1: 160x120")
        return None

    # Convert to 1-bit if needed
    if img.mode != '1':
        img = img.convert('L')  # Grayscale first
        img = img.point(lambda x: 255 if x > 128 else 0, '1')

    # Convert to framebuffer format
    widthbyte = width // 8
    data = bytearray(widthbyte * height)

    pixels = img.load()
    for y in range(height):
        for x in range(width):
            if pixels[x, y]:  # White pixel
                addr = (x >> 3) + y * widthbyte
                bit = 0x80 >> (x & 7)
                data[addr] |= bit

    # Generate array name from filename if not provided
    if array_name is None:
        basename = os.path.splitext(os.path.basename(png_path))[0]
        # Remove leading numbers like "02_"
        if basename[0:2].isdigit() and basename[2] == '_':
            basename = basename[3:]
        array_name = f"{basename}_vmode{vmode}"

    # Generate C array
    lines = [
        f"// Converted from: {os.path.basename(png_path)}",
        f"// Size: {width}x{height}, {len(data)} bytes",
        f"const unsigned char __attribute__((aligned(4))) {array_name}[{len(data)}] = {{"
    ]

    for i in range(0, len(data), 16):
        chunk = data[i:i+16]
        hex_str = ", ".join(f"0x{b:02X}" for b in chunk)
        lines.append(f"    {hex_str},")

    lines.append("};")

    return "\n".join(lines), array_name, vmode


def main():
    if len(sys.argv) < 2:
        print("Usage: python3 convert.py <input.png> [output_name]")
        print("\nExample:")
        print("  python3 convert.py vmode1/02_pattern_crosshatch.png")
        sys.exit(1)

    png_path = sys.argv[1]
    array_name = sys.argv[2] if len(sys.argv) > 2 else None

    if not os.path.exists(png_path):
        print(f"Error: File not found: {png_path}")
        sys.exit(1)

    result = png_to_c_array(png_path, array_name)
    if result is None:
        sys.exit(1)

    c_code, name, vmode = result

    # Output to stdout
    print(c_code)
    print(f"\n// Array name: {name}")
    print(f"// For VMODE: {vmode}")


if __name__ == "__main__":
    main()
