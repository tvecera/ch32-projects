#!/bin/bash

# Compilation with SDK and MCU selection
# Usage: ./c.sh <sdk> <mcu>
# Example: ./c.sh ch32fun ch32v002

SDK="${1:-ch32libsdk}"
MCU="${2:-ch32v002}"
MCU_UPPER=$(echo "$MCU" | tr '[:lower:]' '[:upper:]')

# Set CH32_ROOT_PATH if not set
export CH32_ROOT_PATH="${CH32_ROOT_PATH:-../sdk/CH32LibSDK}"

echo "Using SDK: ${SDK}, MCU: ${MCU_UPPER}"

if [ "$SDK" = "ch32libsdk" ]; then
    # CH32LibSDK uses external scripts
    cp "Makefile.${SDK}" Makefile
    export MCU="${MCU_UPPER}x4"
    ../tools/CH32LibSDK/_c1.sh "${MCU_UPPER}x4"
else
    # ch32fun uses Makefile
    if [ -f "Makefile.${SDK}" ]; then
        cp "Makefile.${SDK}" Makefile
    else
        echo "Error: Makefile.${SDK} not found"
        exit 1
    fi
    make TARGET_MCU="${MCU_UPPER}" build
fi
