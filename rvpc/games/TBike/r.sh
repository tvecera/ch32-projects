#!/bin/bash
# Reset device
# Usage: ./r.sh

echo "Resetting device..."
# Set CH32_ROOT_PATH if not set
export CH32_ROOT_PATH="${CH32_ROOT_PATH:-../../ch32/CH32LibSDK}"
${CH32_ROOT_PATH}/_r1.sh

