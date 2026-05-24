#!/bin/bash
# Keystone Simulator - Build and Run
#
# Automatically opens in Terminal.app for QR screen scanning to work.

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

if [ "$TERM_PROGRAM" != "Apple_Terminal" ]; then
    echo "Reopening in Terminal.app for QR scanning support..."
    QUOTED_SCRIPT_DIR="$(printf '%q' "$SCRIPT_DIR")"
    QUOTED_ARGS=()
    for ARG in "$@"; do
        QUOTED_ARGS+=("$(printf '%q' "$ARG")")
    done
    osascript -e "tell application \"Terminal\" to do script \"cd $QUOTED_SCRIPT_DIR && ./simulator.sh ${QUOTED_ARGS[*]}\""
    exit 0
fi

cd "$SCRIPT_DIR"

if [ -f ".venv/bin/activate" ]; then
    source .venv/bin/activate
fi

if [ -f "$HOME/.cargo/env" ]; then
    source "$HOME/.cargo/env"
elif [ -d "$HOME/.cargo/bin" ]; then
    export PATH="$HOME/.cargo/bin:$PATH"
fi

BUILD=true
if [ "$1" = "--no-build" ] || [ "$1" = "-n" ]; then
    BUILD=false
    shift
fi

if [ "$BUILD" = true ]; then
    echo "=== Building Cypherpunk Simulator ==="
    python3 build.py -t cypherpunk -o simulator
    echo ""
fi

echo "=== Starting Simulator ==="
exec ./build/simulator "$@"
