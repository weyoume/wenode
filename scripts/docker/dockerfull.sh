#!/bin/bash

docker run --env USE_FULLNODE=1 \
	-d -p 2001:2001 -p 8090:8090 -p 80:8090 -p 8080:8090 --name wenode \
	-td wenode
