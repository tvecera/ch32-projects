#!/bin/bash
# Delete temporary files of all projects in all sub-directories

# Function to delete temporary files of one project in subdirectory
del1() {
    local dir="$1"
    if [ ! -f "$dir/d.sh" ]; then
        return
    fi
    cd "$dir" || return
    echo "Deleting $dir"
    ./d.sh
    cd ..
}

# Loop to find all sub-directories
for dir in */; do
    if [ -d "$dir" ]; then
        del1 "${dir%/}"
    fi
done