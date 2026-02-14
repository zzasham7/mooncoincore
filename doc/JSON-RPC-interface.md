# JSON-RPC Interface

`mooncoind` enables JSON-RPC by default when running as server.
`mooncoin-qt` can expose RPC when started with `-server`.

## Security model

RPC provides privileged control of node and wallet operations.
Treat RPC access as administrative access.

## Core rules

- Bind RPC to localhost unless remote administration is strictly required.
- Do not expose RPC directly to the public Internet.
- Use cookie auth or `rpcauth`; avoid weak static credentials.
- Restrict clients with `rpcallowip` and network firewall policy.

## Baseline secure settings

```ini
server=1
rpcbind=127.0.0.1
rpcallowip=127.0.0.1
rpcport=44663
```

## Quick test

```bash
src/mooncoin-cli -datadir="$HOME/mooncoin-data" getblockchaininfo
src/mooncoin-cli -datadir="$HOME/mooncoin-data" getnetworkinfo
```

## Production guidance

- Prefer a local API service that talks to loopback RPC.
- Put public APIs behind a separate service layer, rate limiting, and auth.
- Keep node host firewall and cloud security rules minimal and explicit.
