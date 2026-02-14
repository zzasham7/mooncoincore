# `mooncoin.conf` Configuration File

The configuration file is used by `mooncoind`, `mooncoin-qt`, and `mooncoin-cli`.

Default filename:

- `mooncoin.conf`

## Typical locations

The active location depends on runtime `-datadir` and existing client settings.

Common defaults:

- Windows: `%APPDATA%\\MooncoinData\\mooncoin.conf`
- Linux: `$HOME/mooncoin-data/mooncoin.conf` (recommended) or your custom `-datadir`
- macOS: `$HOME/Library/Application Support/MooncoinData/mooncoin.conf`

Compatibility note:

- Existing installs may still use legacy paths (for example `$HOME/.litecoin/`) depending on prior client settings and data migration history.

## Minimal node config

```ini
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
```

## Peer bootstrap example

```ini
addnode=87.98.254.62:44664
addnode=159.196.219.68:44664
```

## Authentication

Recommended:

- Use cookie auth (local process access).
- Or set `rpcauth` entries generated with `share/rpcauth/rpcauth.py`.

Legacy `rpcuser`/`rpcpassword` works, but cookie/rpcauth is preferred.

## Notes

- `fallbackfee` can be set if fee estimator has insufficient history.
- Keep RPC bound to loopback unless you have a hardened private network design.
- You can find an example configuration in [share/examples/mooncoin.conf](../share/examples/mooncoin.conf).
