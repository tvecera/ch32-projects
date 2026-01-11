#!/bin/bash

# Reset device
# Usage: ./r.sh [sdk]

SDK="${1:-ch32libsdk}"
MCU="${2:-ch32v002}"
MCU_UPPER=$(echo "$MCU" | tr '[:lower:]' '[:upper:]')

# Set CH32_ROOT_PATH if not set
export CH32_ROOT_PATH="${CH32_ROOT_PATH:-../sdk/CH32LibSDK}"

echo "Resetting device, SDK: ${SDK}"

../tools/CH32LibSDK/_r1.sh
