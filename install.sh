#!/bin/sh
set -e

BIN_URL="https://github.com/yourname/cardiac/releases/latest/download/cardiac"
INSTALL_DIR="/usr/local/bin"

echo "Installing cardiac..."

curl -fsSL "$BIN_URL" -o cardiac
chmod +x cardiac

if [ ! -w "$INSTALL_DIR" ]; then
    echo "Requesting sudo to install into $INSTALL_DIR"
    sudo mv cardiac "$INSTALL_DIR/cardiac"
else
    mv cardiac "$INSTALL_DIR/cardiac"
fi

echo "Installed cardiac to $INSTALL_DIR/cardiac"
echo "Run: cardiac --help"
