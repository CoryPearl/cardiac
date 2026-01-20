#!/bin/sh
set -e

BIN_URL="https://github.com/CoryPearl/cardiac/releases/latest/download/cardiac"
INSTALL_DIR="/usr/local/bin"
BIN_PATH="$INSTALL_DIR/cardiac"

echo "Installing/updating cardiac..."

if command -v cardiac >/dev/null 2>&1; then
    echo "cardiac already installed at $(command -v cardiac)"
    echo "Updating to latest version..."
fi

curl -fsSL "$BIN_URL" -o cardiac
chmod +x cardiac

if [ ! -w "$INSTALL_DIR" ]; then
    echo "Requesting sudo to install/update in $INSTALL_DIR"
    sudo mv cardiac "$BIN_PATH"
else
    mv cardiac "$BIN_PATH"
fi

echo "cardiac installed/updated successfully at $BIN_PATH"
echo "Run: cardiac --help"
