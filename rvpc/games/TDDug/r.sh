#!/bin/bash
# Reset device
# Usage: ./r.sh

echo "Resetting device..."
# Set CH32_ROOT_PATH if not set
export CH32_ROOT_PATH="${CH32_ROOT_PATH:-../../../sdk/CH32LibSDK}"
../../../tools/CH32LibSDK/_r1.sh

