docker run --env USE_WAY_TOO_MUCH_RAM=1 --env USE_FULL_WEB_NODE=1 \
	-d -p 2001:2001 -p 8090:8090 --name ezira-full \
	ezira/eznode
