#!/bin/bash

echo "starting docker container ezira with credentials in config.ini"
echo "linked ports 2001:2001 8090:8090"
echo "image eziranetwork/ezira"
echo "not load balanced internally"
echo "full web node"

docker run \
	--env USE_WAY_TOO_MUCH_RAM=1 \
	--env USE_FULL_CONTENT_NODE=1 \
	--env PRIVATE_KEY=PRIVATE_KEY_HERE \
	--env USE_NGINX_FRONTEND=1 \
	--env WITNESS_NAME="initminer" \
	-d -p 2001:2001 -p 8090:8090 --name ezira \
	eziranetwork/ezira

