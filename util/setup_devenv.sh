#!/usr/bin/env bash
set -euo pipefail

source "$HOME/.microtone_env"

sudo apt update
sudo apt install -y \
    build-essential \
    cmake \
    git \
    libasound2-dev

mkdir -p "$DEVELOPER_DIR"
cd "$DEVELOPER_DIR"

if [ ! -d "$SOURCE_DIR" ]; then
    git clone https://github.com/DanielToby/microtone.git
else
    echo "microtone already exists at $SOURCE_DIR"
fi

mkdir -p "$BUILD_DIR"
