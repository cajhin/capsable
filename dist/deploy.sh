#!/bin/bash
set -e
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

cat <<INFO
This will install:
  ~/.local/share/capsable/{capsable-core,intercept,uinput}
  ~/.local/bin/capsable
INFO
read -r -p "continue? [y/N] " ans
[[ "$ans" =~ ^[Yy]$ ]] || { echo "aborted"; exit 1; }

mkdir -p ~/.local/share/capsable ~/.local/bin
cp "$SCRIPT_DIR/bin/capsable-core" "$SCRIPT_DIR/bin/intercept" "$SCRIPT_DIR/bin/uinput" ~/.local/share/capsable/
chmod +x ~/.local/share/capsable/*
cp "$SCRIPT_DIR/capsable" ~/.local/bin/capsable
chmod +x ~/.local/bin/capsable
echo "deployed"
