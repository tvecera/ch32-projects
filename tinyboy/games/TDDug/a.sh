#!/bin/bash

# All Re-Compilation with USE RCA selection
# Usage: ./a.sh <use rca>
# Example: ./a.sh 0

USE_RCA="${1:-0}"

bash d.sh
bash c.sh "$USE_RCA"