#!/bin/bash

# All Re-Compilation with SDK and MCU selection
# Usage: ./a.sh <sdk> <mcu>
# Example: ./a.sh ch32fun ch32v002

SDK="${1:-ch32libsdk}"
MCU="${2:-ch32v002}"

bash d.sh "$SDK" "$MCU"
bash c.sh "$SDK" "$MCU"
