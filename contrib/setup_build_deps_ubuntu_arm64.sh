#!/usr/bin/env bash
set -euo pipefail

if ! command -v apt >/dev/null 2>&1; then
  echo "This script requires apt (Ubuntu/Debian)." >&2
  exit 1
fi

echo "[1/2] Updating package index..."
sudo apt update

echo "[2/2] Installing Mooncoin build dependencies (Ubuntu ARM64)..."
sudo apt install -y \
  build-essential libtool autotools-dev automake pkg-config \
  python3 bsdmainutils curl git ca-certificates \
  libevent-dev libboost-system-dev libboost-filesystem-dev libboost-thread-dev \
  libsqlite3-dev libzmq3-dev libssl-dev libfmt-dev

echo "Done."
echo "Next: ./autogen.sh && ./configure --without-gui --disable-wallet --disable-tests --disable-bench --without-miniupnpc"
