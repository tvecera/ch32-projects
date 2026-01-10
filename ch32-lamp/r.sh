#!/bin/bash
# Reset device...

# Set CH32_ROOT_PATH if not set
export CH32_ROOT_PATH="${CH32_ROOT_PATH:-../sdk/CH32LibSDK}"

../tools/CH32LibSDK/_r1.sh
