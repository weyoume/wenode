#!/bin/bash

echo "starting docker container WeYouMe with credentials in config.ini"
echo "linked ports 2001:2001 8090:8090"
echo "image WeYouMe/WeYouMe"
echo "not load balanced internally"
echo "full web node"

docker run \
	--env USE_WAY_TOO_MUCH_RAM=1 \
	--env USE_FULLNODE=1 \
		--env USE_NGINX_FRONTEND=1 \
	-d -p 2001:2001 -p 8090:8090 --name WeYouMe \
	WeYouMe/WeYouMe

