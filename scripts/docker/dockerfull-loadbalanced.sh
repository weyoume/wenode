#!/bin/bash

docker run --env USE_FULLNODE=1 --env USE_MULTICORE_READONLY=1 \
	-d -p 11111:11111 -p 12345:12345 --name wenode-lb \
	weyoume/wenode-lb
