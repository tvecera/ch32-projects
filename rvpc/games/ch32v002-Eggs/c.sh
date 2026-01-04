#!/bin/bash
# Compilation with MCU selection
# Usage: ./c.sh <mcu> <use rca>
# Example: ./c.sh ch32v002 0

MCU="${1:-ch32v002}"
USE_RCA="${2:-0}"
MCU_UPPER=$(echo "$MCU" | tr '[:lower:]' '[:upper:]')

# Set CH32_ROOT_PATH if not set
export CH32_ROOT_PATH="${CH32_ROOT_PATH:-../../ch32/CH32LibSDK}"

echo "Using SDK: ${SDK}, USE_RCA: ${USE_RCA}"

export MCU="${MCU_UPPER}x4"
export USE_RCA="${USE_RCA}"
${CH32_ROOT_PATH}/_c1.sh "${MCU_UPPER}x4"
