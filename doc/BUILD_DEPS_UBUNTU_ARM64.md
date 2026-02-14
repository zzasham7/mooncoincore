# Ubuntu ARM64 Build Dependencies (Mooncoin v0.21.4)

This note captures the dependency set validated on an Ubuntu ARM64 VPS for building a headless node binary (`mooncoind`) from source.

## One-time install

```bash
bash contrib/setup_build_deps_ubuntu_arm64.sh
```

Or install manually:

```bash
sudo apt update
sudo apt install -y \
  build-essential libtool autotools-dev automake pkg-config \
  python3 bsdmainutils curl git ca-certificates \
  libevent-dev libboost-system-dev libboost-filesystem-dev libboost-thread-dev \
  libsqlite3-dev libzmq3-dev libssl-dev libfmt-dev
```

## Configure and build (headless node)

```bash
./autogen.sh
./configure --without-gui --disable-wallet --disable-tests --disable-bench --without-miniupnpc
make -j"$(nproc)"
```

## Notes

- `--without-gui`: no Qt wallet GUI.
- `--disable-wallet`: no wallet/BerkeleyDB requirement.
- `--disable-tests --disable-bench`: faster build, fewer artifacts.
- `--without-miniupnpc`: no UPnP/NAT-PMP.
- ZMQ is kept enabled by default in the command above (requires `libzmq3-dev`).
