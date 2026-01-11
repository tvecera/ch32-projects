#!/bin/bash

# Export/Flash to hardware
# Usage: ./e.sh [sdk] [mcu]

SDK="${1:-ch32libsdk}"
MCU="${2:-ch32v002}"
MCU_UPPER=$(echo "$MCU" | tr '[:lower:]' '[:upper:]')

# Set CH32_ROOT_PATH if not set
export CH32_ROOT_PATH="${CH32_ROOT_PATH:-../sdk/CH32LibSDK}"

echo "Flashing SDK: ${SDK}"

if [ "$SDK" = "ch32libsdk" ]; then
    # CH32LibSDK uses external scripts
    cp "Makefile.${SDK}" Makefile
    ../tools/CH32LibSDK/_e1.sh
else
    # ch32fun uses Makefile
    if [ -f "Makefile.${SDK}" ]; then
        cp "Makefile.${SDK}" Makefile
    fi
    make TARGET_MCU="${MCU_UPPER}" flash
fi

bash r.sh ${SDK} ${MCU}
