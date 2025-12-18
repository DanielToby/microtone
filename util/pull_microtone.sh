#!/usr/bin/env bash
set -euo pipefail

source "$HOME/.microtone_env"

cd "$SOURCE_DIR"

git pull
git submodule update --init --recursive
