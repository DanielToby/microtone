#!/usr/bin/env bash
set -euo pipefail

source "$HOME/.microtone_env"

cd "$SOURCE_DIR"

git fetch origin
git reset --hard origin/main
git submodule update --init --recursive
