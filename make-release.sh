#!/bin/bash
set -e
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$SCRIPT_DIR"

if [ ! -f build/capsable-core ] || [ ! -f build/intercept ] || [ ! -f build/uinput ]; then
    echo "missing binaries in build/ — run compile.sh first" >&2
    exit 1
fi

rm -rf dist
mkdir -p dist/bin
cp build/capsable-core build/intercept build/uinput dist/bin/
cp src/capsable dist/capsable
chmod +x dist/bin/* dist/capsable

cat > dist/deploy.sh <<'EOF'
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
EOF
chmod +x dist/deploy.sh

echo "release prepared in dist/"
