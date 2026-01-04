#!/bin/bash

# Rebuild and write (compile + flash)
# Usage: ./x.sh <mcu> <use rca>
# Example: ./x.sh ch32v002 0

MCU="${1:-ch32v002}"
USE_RCA="${2:-0}"

bash a.sh "$MCU" "$USE_RCA" && bash e.sh "$MCU" "$USE_RCA"
