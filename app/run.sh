#!/bin/bash
set -e

# ─────────────────────────────────────────────
#  run.sh — Build & launch Nim AI
# ─────────────────────────────────────────────

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$PROJECT_ROOT/build"
BIN="$BUILD_DIR/nim_ai"

# ── 1. Dependencies check ────────────────────
echo ">>> Checking dependencies..."

MISSING=()
command -v cmake        &>/dev/null || MISSING+=("cmake")
command -v make         &>/dev/null || MISSING+=("build-essential")
dpkg -s libsfml-dev     &>/dev/null 2>&1 || MISSING+=("libsfml-dev")

if [ ${#MISSING[@]} -gt 0 ]; then
    echo ">>> Installing missing packages: ${MISSING[*]}"
    sudo apt-get update -qq
    sudo apt-get install -y "${MISSING[@]}"
fi

# ── 2. Clean build (optional --clean flag) ───
if [[ "$1" == "--clean" ]]; then
    echo ">>> Cleaning build directory..."
    rm -rf "$BUILD_DIR"
fi

# ── 3. Configure ─────────────────────────────
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

echo ">>> Configuring with CMake..."
cmake "$PROJECT_ROOT" -DCMAKE_BUILD_TYPE=Release

# ── 4. Compile ───────────────────────────────
echo ">>> Compiling ($(nproc) jobs)..."
make -j"$(nproc)"

# ── 5. Run ───────────────────────────────────
echo ">>> Launching Nim AI..."
cd "$PROJECT_ROOT"
exec "$BIN"