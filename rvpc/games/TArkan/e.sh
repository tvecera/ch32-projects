#!/bin/bash
# Export/Flash to hardware
# Usage: ./e.sh
# Set CH32_ROOT_PATH if not set

echo "Flashing..."
export CH32_ROOT_PATH="${CH32_ROOT_PATH:-../../../sdk/CH32LibSDK}"
../../../tools/CH32LibSDK/_e1.sh
