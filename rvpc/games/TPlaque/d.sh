#!/bin/bash
# Delete/Clean
# Usage: ./d.sh
# Example: ./d.sh

echo "Cleaning..."
# Set CH32_ROOT_PATH if not set
export CH32_ROOT_PATH="${CH32_ROOT_PATH:-../../ch32/CH32LibSDK}"
${CH32_ROOT_PATH}/_d1.sh
