#!/bin/bash

if [ -z "$1" ]; then
    echo "Usage: $0 <destination_directory>"
    exit 1
fi

DEST="$1"
mkdir -p "$DEST"

count=0
while IFS= read -r -d '' file; do
    cp -v "$file" "$DEST"
    ((count++))
done < <(find . -type f -name "*.bin" -print0)

echo "Copied $count files."