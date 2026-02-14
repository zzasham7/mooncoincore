# Depends Build System (Mooncoin Core)

This directory contains the `depends` system used to build pinned third-party libraries,
primarily for reproducible cross-compilation.

## Recommended use in this repository

The primary supported cross-build route is:

- WSL/Linux host
- `HOST=x86_64-w64-mingw32`
- then configure Mooncoin Core with the generated `config.site`

See `../doc/build-windows.md` for the end-to-end flow.

## Quick usage

Build dependencies for current host:

```bash
make
```

Build for a target host:

```bash
make HOST=host-platform-triplet -j"$(nproc)"
```

Example (Win64):

```bash
make HOST=x86_64-w64-mingw32 -j"$(nproc)"
```

Configure Mooncoin Core to use depends output:

```bash
CONFIG_SITE=$PWD/depends/x86_64-w64-mingw32/share/config.site \
./configure --prefix=/ --with-gui=qt5 --enable-zmq
```

## Common host triplets

- `x86_64-w64-mingw32` (Windows x64)
- `x86_64-pc-linux-gnu` (Linux x64)
- `aarch64-linux-gnu` (Linux ARM64)

## Useful make options

Set with `make FOO=bar`:

- `SOURCES_PATH`: download source cache location
- `BASE_CACHE`: built package cache location
- `NO_QT=1`: skip Qt
- `NO_ZMQ=1`: skip ZMQ
- `NO_WALLET=1`: skip wallet dependencies
- `NO_BDB=1`: skip Berkeley DB
- `NO_SQLITE=1`: skip SQLite
- `NO_UPNP=1`: skip UPnP dependencies

## Helper targets

```bash
make download
make download-win
make download-linux
```

## Notes

- Keep your source tree on the Linux filesystem in WSL (for example `~/mooncoin/litecoin`) for reliable autoconf behavior.
- The `depends` system is optional for native Linux builds, but recommended for reproducible cross-builds.
