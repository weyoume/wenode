# Quickstart Guide

## Low memory node:

Suitable for:
- Seed nodes.
- Producer nodes.
- Exchanges.

```
docker run \
    -d -p 2001:2001 -p 8090:8090 \
    --name wenode \
    --restart unless-stopped \ 
    weyoume/wenode
```

## Full API Node:

Suitable for:
- Web Applications.
- Mobile Applications.
- Public API nodes.

```
docker run \
    --env USE_FULLNODE=1 \
    -d -p 2001:2001 -p 8090:8090 \
    --name wenode \
    --restart unless-stopped \
    weyoume/wenode
```

You need to use add `USE_FULLNODE=1` as stated above.
You can use `contrib/config-for-fullnode.ini` as a base for your `config.ini` file.

## Exchanges:

Use low memory node.

Ensure that your `config.ini` contains:
```
enable-plugin = account_history
public-api = database_api login_api
track-account-range = ["yourexchangeid", "yourexchangeid"]
```

This configuration exists in Docker with the following command:

```
docker run -d --env TRACK_ACCOUNT="yourexchangeid" \
    --name wenode \
    --restart unless-stopped \
    weyoume/wenode
```

## Resource usage:

Ensure that you have enough resources available.
Check `shared-file-size =` in your `config.ini` to reflect your needs.
Blockchain data takes over **64GB** of storage space.
Set it to at least 25% more than current size. Provided values are expected to grow significantly over time.

## Full node:
Shared memory file for full node uses something about **200GB**.

## Exchange node:
Shared memory file for exchange node users over **24GB**.

## Seed node:
Shared memory file for seed node uses over **24GB**.

## Other use cases:
Shared memory file size varies, depends on your specific configuration but it is expected to be somewhere between "seed node" and "full node" usage.
