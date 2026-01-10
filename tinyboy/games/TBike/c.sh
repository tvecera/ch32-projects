#!/bin/bash
# Compilation with MCU selection
# Usage: ./c.sh

# Set CH32_ROOT_PATH if not set
export CH32_ROOT_PATH="${CH32_ROOT_PATH:-../../../sdk/CH32LibSDK}"

../../../tools/CH32LibSDK/_c1.sh
