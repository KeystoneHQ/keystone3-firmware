#!/bin/bash
# Keystone Simulator - Build and Run
#
# Automatically opens in Terminal.app for QR screen scanning to work.

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

if [ "$TERM_PROGRAM" != "Apple_Terminal" ]; then
    echo "Reopening in Terminal.app for QR scanning support..."
    LAUNCHER_DIR="$(mktemp -d -t keystone-simulator.XXXXXX)"
    LAUNCHER="$LAUNCHER_DIR/keystone-simulator.command"
    {
        printf '#!/bin/bash\n'
        printf 'cd %q\n' "$SCRIPT_DIR"
        printf './simulator.sh'
        for ARG in "$@"; do
            printf ' %q' "$ARG"
        done
        printf '\n'
    } > "$LAUNCHER"
    chmod +x "$LAUNCHER"
    open -a Terminal "$LAUNCHER"
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

if command -v python3.12 >/dev/null 2>&1; then
    PYTHON_BIN=python3.12
else
    PYTHON_BIN=python3
fi

BUILD=true
if [ "$1" = "--no-build" ] || [ "$1" = "-n" ]; then
    BUILD=false
    shift
fi

if [ "$BUILD" = true ]; then
    echo "=== Building Cypherpunk Simulator ==="
    "$PYTHON_BIN" build.py -t cypherpunk -o simulator -s
    echo ""
fi

echo "=== Starting Simulator ==="
exec ./build/simulator "$@"
