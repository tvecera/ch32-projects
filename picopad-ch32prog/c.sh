#!/bin/bash

# Compilation...

export TARGET="CH32PROG"
export GRPDIR="CH32"
export MEMMAP=""

../tools/PicoLibSDK/_c1.sh "$1"
