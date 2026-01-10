#!/bin/bash
# Compilation with USE RCA selection
# Usage: ./c.sh <use rca>
# Example: ./c.sh 0

USE_RCA="${1:-0}"

# Set CH32_ROOT_PATH if not set
export CH32_ROOT_PATH="${CH32_ROOT_PATH:-../../../sdk/CH32LibSDK}"

echo "Using USE_RCA: ${USE_RCA}"

export USE_RCA="${USE_RCA}"
../../../tools/CH32LibSDK/_c1.sh
