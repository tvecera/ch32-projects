#!/usr/bin/env python3
"""
MIT License

Copyright (c) 2026 Tomas Vecera, tomas@vecera.dev

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

============================================================================
BitBang to SPI Font Converter
============================================================================

Converts preprocessed BitBang font format to SPI format with correct
256-byte stride for use with SPI-based video rendering.

Input format (BitBang):
  - 2 pixels per byte (high nibble = first pixel, low nibble = second)
  - Pixel value: 0x04 = white, 0x00 = black
  - 128-char font: 4096 bytes (8 rows * 128 chars * 4 bytes)
  - 64-char font: 2048 bytes (8 rows * 64 chars * 4 bytes)

Output format (SPI):
  - 1 bit per pixel, 8 pixels per byte, MSB = leftmost
  - 1 = white, 0 = black
  - Fixed 256-byte stride per row (padding with zeros)
  - Total: 2048 bytes (8 rows * 256 bytes)

Usage:
  # Convert single font and print to stdout
  python3 convert_to_spi.py <input_file.c> <bitbang_array> <spi_array>

  # Convert and update the C file in place
  python3 convert_to_spi.py <input_file.c> <bitbang_array> <spi_array> --update

  # Convert all fonts in directory
  python3 convert_to_spi.py --all

Example:
  python3 convert_to_spi.py text_mode_font_80.c TextModeFont80Data TextModeFont80DataSpi --update

============================================================================
"""

import sys
import re
import os

# Font definitions: (filename, bitbang_array, spi_array)
FONTS = [
    ("text_mode_font_thin_8x8.c", "TextModeFontThin8x8Data", "TextModeFontThin8x8DataSpi"),
    ("text_mode_font_bold_8x8.c", "TextModeFontBold8x8Data", "TextModeFontBold8x8DataSpi"),
    ("text_mode_font_thin_8x8_64.c", "TextModeFontThin8x8Data", "TextModeFontThin8x8DataSpi"),
    ("text_mode_font_bold_8x8_64.c", "TextModeFontBold8x8Data", "TextModeFontBold8x8DataSpi"),
    ("text_mode_font_rvpc.c", "TextModeFontRvpcData", "TextModeFontRvpcDataSpi"),
    ("text_mode_font_zx.c", "TextModeFontZxData", "TextModeFontZxDataSpi"),
    ("text_mode_font_80.c", "TextModeFont80Data", "TextModeFont80DataSpi"),
    ("text_mode_font_81.c", "TextModeFont81Data", "TextModeFont81DataSpi"),
]


def extract_array_data(file_content, array_name):
    """Extract hex values from a C array definition."""
    # Find array definition: ... array_name[...] = { ... };
    # Handle static, const, __attribute__, etc.
    pattern = rf'{re.escape(array_name)}\s*\[\d+\]\s*(?:__attribute__\s*\(\s*\(aligned\(\d+\)\)\s*\))?\s*=\s*\{{'
    match = re.search(pattern, file_content)

    if not match:
        raise ValueError(f"Array '{array_name}' not found in file")

    # Find the opening brace and match to closing brace
    start_pos = match.end() - 1  # Position of '{'
    brace_count = 1
    pos = start_pos + 1
    while pos < len(file_content) and brace_count > 0:
        if file_content[pos] == '{':
            brace_count += 1
        elif file_content[pos] == '}':
            brace_count -= 1
        pos += 1

    array_content = file_content[start_pos + 1:pos - 1]

    # Extract all hex values
    hex_pattern = r'0x([0-9A-Fa-f]{2})'
    hex_values = re.findall(hex_pattern, array_content)

    return [int(h, 16) for h in hex_values]


def find_array_bounds(file_content, array_name):
    """Find the start and end positions of an array definition in the file."""
    # Find the array declaration start (including static const etc.)
    pattern = rf'(static\s+)?const\s+unsigned\s+char\s+{re.escape(array_name)}\s*\[\d+\]\s*(?:__attribute__\s*\(\s*\(aligned\(\d+\)\)\s*\))?\s*=\s*\{{'
    match = re.search(pattern, file_content)

    if not match:
        return None, None

    start_pos = match.start()

    # Find closing brace and semicolon
    brace_start = match.end() - 1
    brace_count = 1
    pos = brace_start + 1
    while pos < len(file_content) and brace_count > 0:
        if file_content[pos] == '{':
            brace_count += 1
        elif file_content[pos] == '}':
            brace_count -= 1
        pos += 1

    # Skip to semicolon
    while pos < len(file_content) and file_content[pos] != ';':
        pos += 1
    if pos < len(file_content):
        pos += 1  # Include semicolon

    return start_pos, pos


def bitbang_to_spi(bitbang_data):
    """
    Convert BitBang font data to SPI format.

    BitBang format:
    - 2 pixels per byte (nibbles), 0x04=white, 0x00=black
    - Layout: row * (char_count * 4) + char * 4 + byte_offset

    SPI format:
    - 8 pixels per byte (bits), 1=white, 0=black, MSB first
    - Layout: row * 256 + char_index (256-byte stride)
    """
    size = len(bitbang_data)

    # Determine character count from size
    # BitBang: 8 rows * char_count * 4 bytes
    if size == 4096:
        char_count = 128
    elif size == 2048:
        char_count = 64
    else:
        raise ValueError(f"Unexpected BitBang data size: {size} bytes. Expected 2048 (64 chars) or 4096 (128 chars)")

    row_stride_bb = char_count * 4  # BitBang bytes per row

    # Output: 2048 bytes (8 rows * 256 bytes)
    spi_data = [0] * 2048

    for row in range(8):
        for char_idx in range(char_count):
            # Read 4 BitBang bytes (8 pixels) for this character row
            bb_offset = row * row_stride_bb + char_idx * 4

            # Convert 4 BitBang bytes to 1 SPI byte
            spi_byte = 0
            for byte_idx in range(4):
                bb_byte = bitbang_data[bb_offset + byte_idx]

                # Extract 2 pixels from nibbles
                pixel_left = (bb_byte >> 4) & 0x0F
                pixel_right = bb_byte & 0x0F

                # Convert to bits (0x04 -> 1, 0x00 -> 0)
                bit_left = 1 if pixel_left else 0
                bit_right = 1 if pixel_right else 0

                # Pack into SPI byte (MSB first)
                # byte_idx 0: bits 7,6 | byte_idx 1: bits 5,4 | byte_idx 2: bits 3,2 | byte_idx 3: bits 1,0
                bit_pos_left = 7 - (byte_idx * 2)
                bit_pos_right = 6 - (byte_idx * 2)

                spi_byte |= (bit_left << bit_pos_left)
                spi_byte |= (bit_right << bit_pos_right)

            # Write to SPI array with 256-byte stride
            spi_offset = row * 256 + char_idx
            spi_data[spi_offset] = spi_byte

    return spi_data, char_count


def format_c_array(data, array_name, char_count):
    """Format data as C array definition."""
    lines = []

    lines.append(f"// SPI font data: {char_count} characters, 256-byte stride")
    lines.append(f"// Layout: font[row * 256 + char_index], 8 pixels per byte, MSB first")
    lines.append(f"static const unsigned char {array_name}[{len(data)}] __attribute__ ((aligned(4))) = {{")

    # Print 16 bytes per line
    for i in range(0, len(data), 16):
        row = data[i:i + 16]
        hex_values = ", ".join(f"0x{val:02X}" for val in row)
        comma = "," if i + 16 < len(data) else ""
        lines.append(f"    {hex_values}{comma}")

    lines.append("};")

    return "\n".join(lines)


def convert_font(input_file, input_array_name, output_array_name, update_file=False):
    """Convert a single font file."""
    # Read input file
    with open(input_file, 'r') as f:
        content = f.read()

    # Extract BitBang data
    try:
        bitbang_data = extract_array_data(content, input_array_name)
    except ValueError as e:
        print(f"Error: {e}", file=sys.stderr)
        return False

    print(f"// Input: {input_file}", file=sys.stderr)
    print(f"// Array: {input_array_name} ({len(bitbang_data)} bytes)", file=sys.stderr)

    # Convert to SPI format
    spi_data, char_count = bitbang_to_spi(bitbang_data)
    print(f"// Output: {output_array_name} ({len(spi_data)} bytes, {char_count} chars)", file=sys.stderr)

    # Generate C array
    c_array = format_c_array(spi_data, output_array_name, char_count)

    if update_file:
        # Find and replace existing SPI array, or append if not found
        start, end = find_array_bounds(content, output_array_name)

        if start is not None:
            # Replace existing array
            new_content = content[:start] + c_array + content[end:]
            print(f"// Replaced existing {output_array_name} in {input_file}", file=sys.stderr)
        else:
            # Append after BitBang array
            bb_start, bb_end = find_array_bounds(content, input_array_name)
            if bb_start is not None:
                # Insert after BitBang array with blank line
                new_content = content[:bb_end] + "\n\n" + c_array + content[bb_end:]
                print(f"// Added {output_array_name} after {input_array_name} in {input_file}", file=sys.stderr)
            else:
                print(f"Error: Could not find position to insert SPI array", file=sys.stderr)
                return False

        with open(input_file, 'w') as f:
            f.write(new_content)

        print(f"// Updated {input_file}", file=sys.stderr)
    else:
        # Output to stdout
        print(c_array)

    return True


def convert_all_fonts(directory="."):
    """Convert all font files in the directory."""
    success_count = 0
    for filename, bb_array, spi_array in FONTS:
        filepath = os.path.join(directory, filename)
        if os.path.exists(filepath):
            print(f"\n{'='*60}", file=sys.stderr)
            print(f"Processing {filename}...", file=sys.stderr)
            if convert_font(filepath, bb_array, spi_array, update_file=True):
                success_count += 1
        else:
            print(f"Skipping {filename} (not found)", file=sys.stderr)

    print(f"\n{'='*60}", file=sys.stderr)
    print(f"Converted {success_count}/{len(FONTS)} fonts", file=sys.stderr)


def main():
    if len(sys.argv) < 2:
        print("Usage:")
        print("  python3 convert_to_spi.py <input_file.c> <bitbang_array> <spi_array> [--update]")
        print("  python3 convert_to_spi.py --all")
        print()
        print("Options:")
        print("  --update    Update the C file in place instead of printing to stdout")
        print("  --all       Convert all known fonts in current directory")
        print()
        print("Example:")
        print("  python3 convert_to_spi.py text_mode_font_80.c TextModeFont80Data TextModeFont80DataSpi --update")
        sys.exit(1)

    if sys.argv[1] == "--all":
        convert_all_fonts()
    else:
        if len(sys.argv) < 4:
            print("Error: Need input_file, bitbang_array, and spi_array arguments", file=sys.stderr)
            sys.exit(1)

        input_file = sys.argv[1]
        input_array_name = sys.argv[2]
        output_array_name = sys.argv[3]
        update_file = "--update" in sys.argv

        if not convert_font(input_file, input_array_name, output_array_name, update_file):
            sys.exit(1)


if __name__ == "__main__":
    main()
