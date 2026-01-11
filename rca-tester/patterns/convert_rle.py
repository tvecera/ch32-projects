#!/usr/bin/env python3
"""
PNG to RLE-compressed Pattern Converter
Converts a PNG image to RLE-compressed C array format for VMODE 1.

RLE Format:
- Byte with MSB=1 (0x80-0xFF): repeat next byte (count - 0x80 + 3) times (3-130 repeats)
- Byte with MSB=0 (0x00-0x7F): next (count + 1) bytes are literal (1-128 literals)

Usage:
  python3 convert_rle.py.py <input.png> [array_name]
"""

import sys
import os

def rle_compress(data):
    """Compress data using PackBits-like RLE"""
    result = bytearray()
    i = 0
    n = len(data)

    while i < n:
        # Look for runs of identical bytes
        run_start = i
        while i < n - 1 and data[i] == data[i + 1] and i - run_start < 129:
            i += 1
        run_len = i - run_start + 1

        if run_len >= 3:
            # Encode as repeat: 0x80 + (run_len - 3), byte
            result.append(0x80 + (run_len - 3))
            result.append(data[run_start])
            i += 1
        else:
            # Look for literal sequence (non-repeating bytes)
            literal_start = run_start
            i = run_start

            while i < n:
                # Check if next 3+ bytes are the same (should switch to run)
                if i < n - 2 and data[i] == data[i + 1] == data[i + 2]:
                    break
                # Check literal length limit
                if i - literal_start >= 127:
                    break
                i += 1

            literal_len = i - literal_start
            if literal_len > 0:
                # Encode as literal: count-1, bytes...
                result.append(literal_len - 1)
                result.extend(data[literal_start:literal_start + literal_len])

    return result


def png_to_rle_array(png_path, array_name=None):
    """Convert PNG to RLE-compressed C array format"""
    try:
        from PIL import Image
    except ImportError:
        print("Error: PIL not installed. Run: pip3 install Pillow")
        return None

    # Load image
    img = Image.open(png_path)
    width, height = img.size

    # Only VMODE 1 supported for RLE
    if width != 160 or height != 120:
        print(f"Error: RLE compression only for VMODE 1 (160x120)")
        print(f"  Got: {width}x{height}")
        return None

    # Convert to 1-bit if needed
    if img.mode != '1':
        img = img.convert('L')
        img = img.point(lambda x: 255 if x > 128 else 0, '1')

    # Convert to framebuffer format
    widthbyte = width // 8  # 20 bytes per line
    raw_data = bytearray(widthbyte * height)  # 2400 bytes

    pixels = img.load()
    for y in range(height):
        for x in range(width):
            if pixels[x, y]:  # White pixel
                addr = (x >> 3) + y * widthbyte
                bit = 0x80 >> (x & 7)
                raw_data[addr] |= bit

    # Compress
    compressed = rle_compress(raw_data)

    # Generate array name
    if array_name is None:
        basename = os.path.splitext(os.path.basename(png_path))[0]
        if basename[0:2].isdigit() and basename[2] == '_':
            basename = basename[3:]
        array_name = f"{basename}_vmode1_rle"

    ratio = len(compressed) / len(raw_data) * 100

    # Generate C array
    lines = [
        f"// RLE compressed from: {os.path.basename(png_path)}",
        f"// Original: {len(raw_data)} bytes, Compressed: {len(compressed)} bytes ({ratio:.1f}%)",
        f"#define {array_name.upper()}_SIZE {len(compressed)}",
        f"const uint8_t __attribute__((aligned(4))) {array_name}[{len(compressed)}] = {{"
    ]

    for i in range(0, len(compressed), 16):
        chunk = compressed[i:i+16]
        hex_str = ", ".join(f"0x{b:02X}" for b in chunk)
        lines.append(f"    {hex_str},")

    lines.append("};")

    return "\n".join(lines), array_name, len(raw_data), len(compressed)


def main():
    if len(sys.argv) < 2:
        print("Usage: python3 convert_rle.py.py <input.png> [array_name]")
        print("\nConverts 160x120 PNG to RLE-compressed C array for VMODE 1")
        sys.exit(1)

    png_path = sys.argv[1]
    array_name = sys.argv[2] if len(sys.argv) > 2 else None

    if not os.path.exists(png_path):
        print(f"Error: File not found: {png_path}")
        sys.exit(1)

    result = png_to_rle_array(png_path, array_name)
    if result is None:
        sys.exit(1)

    c_code, name, orig_size, comp_size = result
    print(c_code)
    print(f"\n// Compression: {orig_size} -> {comp_size} bytes ({comp_size/orig_size*100:.1f}%)")


if __name__ == "__main__":
    main()
