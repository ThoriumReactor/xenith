#!/bin/bash

if [ -z "$1" ]; then
    echo "Usage: ./compile_shaders.sh <subdirectory>"
    echo "Example: ./compile_shaders.sh default"
    exit 1
fi

SUBDIR=$1
BASE_PATH="assets/shaders/$SUBDIR"

if [ ! -d "$BASE_PATH" ]; then
    echo "Error: Directory $BASE_PATH does not exist."
    exit 1
fi


SLANGC="slangc"

for file in "$BASE_PATH"/*.slang; do
    [ -e "$file" ] || continue

    filename=$(basename -- "$file")
    name="${filename%.*}"

    if [[ "$name" == *"_vert" ]]; then
        STAGE="vertex"
    elif [[ "$name" == *"_frag" ]]; then
        STAGE="fragment"
    elif [[ "$name" == *"_comp" ]]; then
        STAGE="compute"
    else
        echo "Skipping $filename: expected suffix _vert, _frag or _comp"
        continue
    fi

    OUTPUT="$BASE_PATH/$name.spv"

    echo "Compiling [$STAGE]: $filename -> $(basename "$OUTPUT")"

    "$SLANGC" \
        -target "spirv" \
        -profile "spirv_1_3" \
        -stage "$STAGE" \
        -entry "main" \
        -o "$OUTPUT" \
        "$file"

    if [ $? -eq 0 ]; then
        echo "Done."
    else
        echo "Failed to compile $filename"
    fi
done
