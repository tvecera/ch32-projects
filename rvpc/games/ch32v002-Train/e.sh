#!/bin/bash
# Export/Flash to hardware
# Usage: ./e.sh
# Set CH32_ROOT_PATH if not set

echo "Flashing..."
export CH32_ROOT_PATH="${CH32_ROOT_PATH:-../../ch32/CH32LibSDK}"
${CH32_ROOT_PATH}/_e1.sh
