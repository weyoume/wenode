#!/bin/bash

docker run --env USE_WAY_TOO_MUCH_RAM=1 --env USE_FULL_WEB_NODE=1 --env USE_MULTICORE_READONLY=1 \
	-d -p 11111:11111 -p 12345:12345 --name ezira-lb \
	eziranetwork/ezira-lb
