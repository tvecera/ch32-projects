#!/bin/bash
# Delete temporary files of one project

# Get the directory where this script is located
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# make clean
rm -f ./*.bin
rm -f ./*.elf
rm -f ./*.hex
rm -f ./*.lst
rm -f ./*.map
rm -f ./*.siz
rm -f ./*.sym
rm -f build/*.o
rm -f build/*.d
rmdir build