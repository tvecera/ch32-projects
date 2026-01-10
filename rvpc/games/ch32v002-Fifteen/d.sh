#!/bin/bash
# Delete/Clean
# Usage: ./d.sh
# Example: ./d.sh

echo "Cleaning..."
# Set CH32_ROOT_PATH if not set
export CH32_ROOT_PATH="${CH32_ROOT_PATH:-../../../sdk/CH32LibSDK}"

../../../tools/CH32LibSDK/_d1.sh
