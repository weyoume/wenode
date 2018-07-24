#!/bin/bash

docker run --env USE_WAY_TOO_MUCH_RAM=1 --env USE_FULL_WEB_NODE=1 \
	-d -p 2001:2001 -p 8090:8090 -p 80:8090 -p 8080:8090  --name ezira0 \
	-td ezira4
