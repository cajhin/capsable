#!/bin/bash
set -e
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
cp "$SCRIPT_DIR/build/capsable" ~/bin/capsable
cp "$SCRIPT_DIR/start_capsable.sh" ~/bin/start_capsable.sh
chmod +x ~/bin/capsable ~/bin/start_capsable.sh
echo "deployed"
