docker run \
	--env USE_WAY_TOO_MUCH_RAM=1 \ 
	--env USE_FULL_WEB_NODE=1 \
	--env USE_MULTICORE_READONLY=1 \
	-d -p 2001:2001 -p 8090:8090 -p 8091:8091 --name ezira \
	eziranetwork/ezira
