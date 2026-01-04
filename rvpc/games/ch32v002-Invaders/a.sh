#!/bin/bash

# All Re-Compilation with MCU selection
# Usage: ./a.sh <mcu> <use rca>
# Example: ./a.sh ch32v002 0

MCU="${1:-ch32v002}"
USE_RCA="${2:-0}"

bash d.sh "$MCU" "$USE_RCA"
bash c.sh "$MCU" "$USE_RCA"
