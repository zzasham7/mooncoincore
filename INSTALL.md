Building Mooncoin Core
======================

This repository is maintained with a practical build policy focused on reproducibility.

Primary supported build paths:

- Linux/Unix native build: see `doc/build-unix.md`
- Windows x64 cross-build from WSL/Linux (`depends`): see `doc/build-windows.md`

Recommended pre-read:

- `README.md`
- `doc/dependencies.md`
- `doc/mooncoin-conf.md`

Notes:

- Legacy upstream build methods and historical release tooling were intentionally removed from this branch.
- For production node deployment, prefer explicit `-datadir` usage and documented firewall rules.
