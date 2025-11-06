#!/bin/bash
# Convenience wrapper to build the executable using PyInstaller and LunaQt.spec
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$SCRIPT_DIR"

if ! command -v pyinstaller >/dev/null 2>&1; then
	echo "PyInstaller not found. Installing into current environment..."
	python3 -m pip install --upgrade pip
	python3 -m pip install pyinstaller
fi

echo "Building executable with PyInstaller (LunaQt.spec)..."
pyinstaller LunaQt.spec

if [ -f "dist/LunaQt" ]; then
	echo "Build complete: dist/LunaQt"
else
	echo "ERROR: Build did not produce dist/LunaQt"
	exit 1
fi
