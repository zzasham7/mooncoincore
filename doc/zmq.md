# ZeroMQ (ZMQ) Notifications

Mooncoin Core can publish real-time chain and mempool events over ZMQ.

## Build requirement

Enable ZMQ at configure time and install `libzmq3-dev`.

Example:

```bash
./configure --enable-zmq
make -j"$(nproc)"
```

## Example runtime options

```bash
mooncoind \
  -zmqpubhashtx=tcp://127.0.0.1:28332 \
  -zmqpubhashblock=tcp://127.0.0.1:28332 \
  -zmqpubrawtx=tcp://127.0.0.1:28333 \
  -zmqpubrawblock=tcp://127.0.0.1:28333
```

Equivalent `mooncoin.conf` entries:

```ini
zmqpubhashtx=tcp://127.0.0.1:28332
zmqpubhashblock=tcp://127.0.0.1:28332
zmqpubrawtx=tcp://127.0.0.1:28333
zmqpubrawblock=tcp://127.0.0.1:28333
```

## Operational notes

- ZMQ sockets are publish-only from node to subscribers.
- Keep endpoints private unless protected by network controls.
- Use a local ingestion service to bridge ZMQ into your API/database pipeline.
