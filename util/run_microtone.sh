#!/usr/bin/env bash
set -euo pipefail

source "$HOME/.microtone_env"

cd "$BUILD_DIR"
./demo/asciiboard/asciiboard
