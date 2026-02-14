# Build Mooncoin Core for Windows (x64)

Recommended method: cross-compile from WSL/Linux using `depends`.
This is the most reproducible path for this repository.

## 1. Install toolchain on WSL/Ubuntu

```bash
sudo apt update
sudo apt install -y \
  build-essential libtool autotools-dev automake pkg-config bsdmainutils curl git \
  g++-mingw-w64-x86-64 cmake nsis
```

## 2. Build `depends`

```bash
cd depends
make HOST=x86_64-w64-mingw32 -j"$(nproc)"
cd ..
```

## 3. Configure with `depends` sysroot

```bash
./autogen.sh
CONFIG_SITE=$PWD/depends/x86_64-w64-mingw32/share/config.site \
./configure --prefix=/ --with-gui=qt5 --enable-zmq
```

## 4. Build

```bash
make -j"$(nproc)"
```

## 5. Output artifacts

Expected Windows binaries:

- `src/mooncoind.exe`
- `src/mooncoin-cli.exe`
- `src/mooncoin-tx.exe`
- `src/mooncoin-wallet.exe`
- `src/qt/mooncoin-qt.exe`

## 6. Packaging example

```bash
mkdir -p release-win64
cp -v src/mooncoind.exe src/mooncoin-cli.exe src/mooncoin-tx.exe src/mooncoin-wallet.exe src/qt/mooncoin-qt.exe release-win64/
cd release-win64 && zip -9 -r ../mooncoin-v0.21.4-win64.zip . && cd ..
```

## Troubleshooting

- If `depends` fails with malformed `PATH`, sanitize PATH and retry from a clean shell.
- If `cmake` is missing during `depends` build, install `cmake` and rerun.
- Keep source under Linux filesystem paths in WSL (for example `~/mooncoin/litecoin`), not under `/mnt/*`.
