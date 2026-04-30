#!/bin/bash
set -e
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
clang "$SCRIPT_DIR/src/capsable.c" -o "$SCRIPT_DIR/build/capsable-core"
mkdir -p ~/.local/share/capsable ~/.local/bin
cp "$SCRIPT_DIR/build/capsable-core" "$SCRIPT_DIR/build/intercept" "$SCRIPT_DIR/build/uinput" ~/.local/share/capsable/
chmod +x ~/.local/share/capsable/*
cp "$SCRIPT_DIR/src/capsable" ~/.local/bin/capsable
chmod +x ~/.local/bin/capsable
echo "deployed"
