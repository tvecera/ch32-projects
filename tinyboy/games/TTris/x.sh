#!/bin/bash

# Rebuild and write (compile + flash)
# Usage: ./x.sh <use rca>
# Example: ./x.sh 0

USE_RCA="${1:-0}"

bash a.sh "$USE_RCA" && bash e.sh
