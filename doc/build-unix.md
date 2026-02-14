# Build Mooncoin Core on Unix/Linux

This document describes a practical build flow for Mooncoin Core on Linux/Unix.

## 1. Install dependencies (Ubuntu/Debian)

```bash
sudo apt update
sudo apt install -y \
  build-essential libtool autotools-dev automake pkg-config bsdmainutils curl git \
  libevent-dev libboost-system-dev libboost-filesystem-dev libboost-thread-dev \
  libsqlite3-dev libzmq3-dev libssl-dev libfmt-dev libdb++-dev
```

Notes:

- `libdb++-dev` is needed for wallet-related objects in this code line.
- If you do not need ZMQ, you may configure with `--without-zmq`.

## 2. Generate build system

```bash
./autogen.sh
```

## 3. Configure

Headless node-focused build:

```bash
./configure --without-gui --disable-tests --disable-bench --without-miniupnpc
```

Common variants:

- Add `--without-zmq` to disable ZMQ.
- Add `--disable-wallet` if your target excludes wallet functionality.

## 4. Build

```bash
make -j"$(nproc)"
```

## 5. Verify binaries

```bash
src/mooncoind --version
src/mooncoin-cli --version
```

Optional outputs (depending on configure flags):

- `src/mooncoin-tx`
- `src/mooncoin-wallet`
- `src/qt/mooncoin-qt`

## 6. Minimal runtime check

```bash
mkdir -p "$HOME/mooncoin-data"
src/mooncoind -datadir="$HOME/mooncoin-data" -daemon
src/mooncoin-cli -datadir="$HOME/mooncoin-data" getblockchaininfo
src/mooncoin-cli -datadir="$HOME/mooncoin-data" stop
```
