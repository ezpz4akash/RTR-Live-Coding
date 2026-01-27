#!/bin/bash

# Find all build.sh files in subdirectories (excluding the current directory)
# and execute them.
find . -mindepth 2 -name "build.sh" -type f | while read -r script; do
    echo "------------------------------------------"
    echo "Executing: $script"
    
    # Get the directory of the build script
    script_dir=$(dirname "$script")
    
    # Move into the directory so the build script runs in its own context
    (
        cd "$script_dir" || exit
        chmod +x build.sh
        ./build.sh
    )

    echo "Finished: $script"
done

echo "------------------------------------------"
echo "All builds complete."