#!/bin/bash

# Rebuild and write (compile + flash)
# Usage: ./x.sh <sdk> <mcu>
# Example: ./x.sh ch32fun ch32v002

SDK="${1:-ch32libsdk}"
MCU="${2:-ch32v002}"

bash a.sh "$SDK" "$MCU" && bash e.sh "$SDK" "$MCU"
