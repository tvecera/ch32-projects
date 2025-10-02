#!/bin/bash

# Compilation...

export TARGET="CH32PROG"
export GRPDIR="CH32"
export MEMMAP=""

${PICO_ROOT_PATH}/_c1.sh "$1"
