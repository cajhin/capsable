#!/bin/bash
set -e
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
mkdir -p "$SCRIPT_DIR/build"
clang "$SCRIPT_DIR/src/capsable-core.c" -o "$SCRIPT_DIR/build/capsable-core"
for bin in intercept uinput; do
    if [ ! -f "$SCRIPT_DIR/build/$bin" ]; then
        cp "$SCRIPT_DIR/dist/bin/$bin" "$SCRIPT_DIR/build/$bin"
    fi
done
