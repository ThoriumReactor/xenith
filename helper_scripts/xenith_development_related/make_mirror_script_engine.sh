#!/bin/bash

if [ -z "$1" ]; then
  echo "Usage: $0 <path/filename_without_extension>"
  exit 1
fi

BASE="$1"

SRC_FILE="src/engine/${BASE}.cpp"
HDR_FILE="include/engine/${BASE}.h"

# Create directories
mkdir -p "$(dirname "$SRC_FILE")"
mkdir -p "$(dirname "$HDR_FILE")"

# Create files if they don’t exist
[ -f "$SRC_FILE" ] || touch "$SRC_FILE"
[ -f "$HDR_FILE" ] || touch "$HDR_FILE"

echo "Created:"
echo "  $SRC_FILE"
echo "  $HDR_FILE"
