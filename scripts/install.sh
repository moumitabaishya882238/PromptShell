#!/usr/bin/env bash
set -euo pipefail

SOURCE="${1:-/e/PromptShell/backend/build/bin/promptshell.exe}"
INSTALL_DIR="${2:-$HOME/bin}"

if [[ ! -f "$SOURCE" ]]; then
  echo "Source executable not found: $SOURCE" >&2
  exit 1
fi

mkdir -p "$INSTALL_DIR"
cp -f "$SOURCE" "$INSTALL_DIR/promptshell.exe"

case ":$PATH:" in
  *":$INSTALL_DIR:"*)
    echo "$INSTALL_DIR is already on PATH"
    ;;
  *)
    echo "Installed to $INSTALL_DIR"
    echo "Add this to your shell profile if needed:"
    echo "  export PATH=\"$INSTALL_DIR:\$PATH\""
    ;;
esac
