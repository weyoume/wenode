#!/bin/bash
docker run \
	--env USE_WAY_TOO_MUCH_RAM=1 \
	--env USE_FULLNODE=1 \
	--env USE_NGINX_FRONTEND=1 \
	--env SEED_NODES=peer.weyoume.io:2001 \
	-d -p 2001:2001 -p 8090:8090 --name testnet-wenode \
	weyoume/testnet-wenode

