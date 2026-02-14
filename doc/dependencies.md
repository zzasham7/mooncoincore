# Dependency Baseline

This document lists the practical dependency baseline used for Mooncoin Core on the current branch.

## Build essentials

- `build-essential`
- `libtool`
- `autotools-dev`
- `automake`
- `pkg-config`
- `bsdmainutils`
- `curl`
- `git`

## Runtime/build libraries

- `libevent-dev`
- `libboost-system-dev`
- `libboost-filesystem-dev`
- `libboost-thread-dev`
- `libsqlite3-dev`
- `libssl-dev`
- `libfmt-dev`
- `libdb++-dev`

## Optional components

- `libzmq3-dev` for ZMQ notifier support.
- Qt5 development packages when building `mooncoin-qt` natively.

## Windows cross-build extras (WSL/Linux)

- `g++-mingw-w64-x86-64`
- `cmake`
- `nsis` (installer packaging, optional)

## Notes

- The `depends` system pins third-party versions for reproducible cross-builds.
- Native host builds may use distro-provided versions.
