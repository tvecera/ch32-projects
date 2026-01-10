#!/bin/bash
# Compilation... Compile all projects in all sub-directories
MCU="${1:-ch32v002}"
USE_RCA="${2:-0}"

# Function to compile one project in subdirectory
comp1() {
    local dir="$1"
    if [ ! -f "$dir/e.sh" ]; then
        return
    fi
    cd "$dir" || return
    echo
    echo "======== Compiling $dir ========"
    ./e.sh ${MCU} ${USE_RCA}
    read -p "Press Enter to continue..."
    cd ..
}

# Loop through all subdirectories
for dir in */; do
    if [ -d "$dir" ]; then
        comp1 "${dir%/}"
    fi
done