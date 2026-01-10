#!/bin/bash

# Compilation... Compile one project

# ---------------------------------------------------
# Edit this line to setup path to GCC RISC-V compiler
# ---------------------------------------------------
# GCC_CH32_PATH="/opt/gcc-ch32/bin"

# Auto-update path
if [ "$GCC_CH32_PATH_ISSET" != "YES" ]; then
    export GCC_CH32_PATH_ISSET=YES
    export PATH="$GCC_CH32_PATH:$PATH"
fi

# Get the directory where this script is located
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

make all
# If you want to see all error messages, compile using this command:
# "$SCRIPT_DIR/_tools/make" all 2> err.txt

if [ $? -ne 0 ]; then
    # Compilation error, stop and wait for key press
    echo "ERROR!"
    read -p "Press any key to continue..."
fi