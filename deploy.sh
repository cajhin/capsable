#!/bin/bash
set -e
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
mkdir -p ~/bin/capsable
cp "$SCRIPT_DIR/build/"* ~/bin/capsable/
cp "$SCRIPT_DIR/start_capsable.sh" ~/bin/capsable/start_capsable.sh
chmod +x ~/bin/capsable/*
echo "deployed"
