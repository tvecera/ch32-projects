#!/bin/bash
# Compilation...

MCU="${1:-ch32v002}"
MCU_UPPER=$(echo "$MCU" | tr '[:lower:]' '[:upper:]')
export MCU="${MCU_UPPER}x4"

# Set CH32_ROOT_PATH if not set
export CH32_ROOT_PATH="${CH32_ROOT_PATH:-../sdk/CH32LibSDK}"

../tools/CH32LibSDK/_c1.sh
