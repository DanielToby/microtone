#!/usr/bin/env bash
set -euo pipefail

source "$HOME/.microtone_env"

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

cmake "$SOURCE_DIR" -DENABLE_GPIO_CONTROL:BOOL=ON
make -j"$(nproc)" -
