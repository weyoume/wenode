#!/bin/bash

docker run \
	--env USE_WAY_TOO_MUCH_RAM=1 \
	--env USE_FULL_WEB_NODE=1 \
	--env STEEMD_PRIVATE_KEY="5KXmLRivL57fgpiFqbpBGfwMLTKq9AGmcUQ7vcLjNi67gtf2s3p" \
	--env STEEMD_WITNESS_NAME="initwitness2" \
	-d -p 2003:2001 -p 8092:8090 --name ezira-test-node2 \
	lopudesigns/ezira-test-node

