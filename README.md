Mooncoin Core
=============

Mooncoin Core is the reference full-node and wallet implementation for the Mooncoin network.
This codebase is based on Litecoin Core v0.21.4 and carries Mooncoin-specific consensus and product behavior.

Website: https://mooncoincore.org

Windows First Start: 0 Connections Fix
--------------------------------------

If your first Windows startup shows `connections = 0`, create `mooncoin.conf` in your data directory and restart.

Place file here:

- Custom data directory you selected on first launch (recommended), for example: `D:\MooncoinData\mooncoin.conf`
- Or default app data directory: `%APPDATA%\Mooncoin\mooncoin.conf`

Use this bootstrap list:

```conf
addnode=87.98.254.62:44664
addnode=159.196.219.68:44664
addnode=51.38.80.19:44664
addnode=51.77.231.146:44664
addnode=158.69.210.223:44664
addnode=71.197.80.244:44664
addnode=24.52.248.184:44664
addnode=89.185.100.228:44664
```

After restart, check `Help -> Debug window -> Information` and confirm connections are no longer zero.

Project Context
---------------

Mooncoin evolved through multiple codebase generations (early Dogecoin/Litecoin lineage, then Mooncoin wallet branches such as 0.13 and 0.17).  
This repository is the modern Mooncoin Core track on top of v0.21.4, with controlled migration of consensus and wallet behavior.

Why This Upgrade Matters
------------------------

This release is the result of extensive migration, build debugging, and cross-platform validation work after a long period of limited upstream maintenance.  
The goal is to raise the technical baseline of Mooncoin Core while preserving chain continuity and operational stability for existing nodes.

Key outcomes:

- Modernized runtime and build baseline (Linux/WSL/Windows workflows validated).
- Improved operator reproducibility through documented build and deployment paths.
- Wallet/GUI usability restored on the current code line.
- Mainnet synchronization behavior verified against the live chain.

Activation Policy (Taproot / MWEB)
----------------------------------

- This release does not activate Taproot or MWEB on mainnet by default.
- Current policy prioritizes chain consistency and safe rollout before any new consensus activation.
- MWEB-related code paths and wallet support are present, but production activation remains a governance and coordination decision.
- If/when activation is scheduled, it should be done via a clearly announced height-based parameter update, with broad miner/node readiness first.

Taproot / MWEB: Scope and Meaning
---------------------------------

Taproot and MWEB are primarily security, privacy, and efficiency feature families.  
They are not intended to rewrite Mooncoin history or alter historical ledger validity.

Important boundary for this release line:

- No historical block rewrite.
- No retroactive change to already-confirmed chain data.
- No change to past transaction validity rules on existing history.
- Baseline consensus continuity is preserved for the current chain state.

In practice, activation discussion is about future block-height behavior only, under explicit coordination, not about changing the past.

Baseline Upgrade Benefits
-------------------------

Beyond consensus safety, the v0.21.4 baseline lift provides concrete operational benefits:

- Better long-term maintainability on a modern, widely-audited codebase.
- Improved compatibility with current toolchains and operating systems.
- More predictable cross-platform build pipeline (especially WSL/Linux -> Windows).
- Stronger node operation ergonomics (diagnostics, indexing paths, deployment repeatability).
- Cleaner path for future feature governance (including optional Taproot/MWEB scheduling) without emergency-style migrations.

Community Upgrade Coordination
------------------------------

Mooncoin is an actively maintained community chain, and successful upgrades depend on coordinated adoption.

To reduce fork risk and avoid isolated-chain scenarios:

- Miners should upgrade before activation windows and monitor rejected block rate.
- Public nodes/explorers should upgrade and verify `blocks == headers` at tip.
- Exchanges/services should validate deposit/withdraw on upgraded nodes in staging first.
- Release announcements should include:
  - exact version/tag/commit
  - upgrade deadline or activation height (if applicable)
  - rollback and troubleshooting guidance

Network Ports
-------------

- Mainnet P2P: `44664`
- Mainnet RPC: `44663`
- Testnet P2P: `14664`
- Testnet RPC: `14663`

Current Release Notes (v0.21.4 line)
------------------------------------

- MoonPoW migration height is consensus-locked in code for this release line.
- Wallet and GUI builds are enabled and validated.
- Branding target binaries:
  - `mooncoind`
  - `mooncoin-cli`
  - `mooncoin-tx`
  - `mooncoin-wallet`
  - `mooncoin-qt`

Address and Feature Compatibility
---------------------------------

- Legacy base58 addresses and bech32 (`moon1...`) addresses are supported.
- `MWEB` is intentionally not active on current Mooncoin chain state (`defined`, `active=false`).
- Do not use "Generate MWEB address" for production payments on this release line.

Build
-----

Prerequisites (Ubuntu/Debian):

```bash
sudo apt update
sudo apt install -y \
  build-essential libtool autotools-dev automake pkg-config bsdmainutils curl git \
  libevent-dev libboost-system-dev libboost-filesystem-dev libboost-thread-dev \
  libsqlite3-dev libdb++-dev libssl-dev libfmt-dev
```

Native Linux build (headless/server style):

```bash
./autogen.sh
./configure --without-gui --disable-tests --disable-bench --without-miniupnpc
make -j"$(nproc)"
```

Windows x64 cross-build from WSL/Linux (recommended reproducible route):

```bash
sudo apt install -y g++-mingw-w64-x86-64 cmake nsis
cd depends
make HOST=x86_64-w64-mingw32 -j"$(nproc)"
cd ..
./autogen.sh
CONFIG_SITE=$PWD/depends/x86_64-w64-mingw32/share/config.site \
./configure --prefix=/ --with-gui=qt5 --enable-zmq
make -j"$(nproc)"
```

Quick Start (Mainnet Daemon)
----------------------------

Create data dir and config:

```bash
mkdir -p "$HOME/mooncoin-data"
cat > "$HOME/mooncoin-data/mooncoin.conf" <<'EOF'
server=1
daemon=1
listen=1
txindex=1
port=44664
rpcport=44663
rpcbind=127.0.0.1
rpcallowip=127.0.0.1
dnsseed=1
discover=1
onlynet=ipv4
EOF
```

Start and verify:

```bash
src/mooncoind -datadir="$HOME/mooncoin-data" -daemon
src/mooncoin-cli -datadir="$HOME/mooncoin-data" getnetworkinfo
src/mooncoin-cli -datadir="$HOME/mooncoin-data" getblockchaininfo
```

Notes:

- If your network has weak DNS seed coverage, add stable peers with `addnode=IP:44664`.
- For public full-node contribution, ensure inbound TCP `44664` is open at cloud firewall and host firewall.

Security Notes
--------------

- Wallet encryption passphrase protects keys inside `wallet.dat`; keep backups.
- Different `wallet.dat` backups may require different passphrases if password was changed over time.
- Newer wallet files may not open in older clients after upgrade.

License
-------

Released under the MIT license. See [COPYING](COPYING).
