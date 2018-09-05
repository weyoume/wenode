#!/bin/bash

docker rm -f wenode-test ;


# will make webuilder1 - webuilder99 if end=100 and start=0
PRIVATE_KEYS=$(cat ./private-keys)
start=1
end=50
witness_names=""
witness_base_name="webuilder"
for ((n=$start;n<$end;n++)); do
	witness_names+="$witness_base_name$((n)) "
done

echo [$witness_names]

docker run \
	--env USE_WAY_TOO_MUCH_RAM=1 \
        --env USE_FULL_CONTENT_NODE=1 \
				--env USE_NGINX_FRONTEND=1 \
        --env PRIVATE_KEYS=$PRIVATE_KEYS \
        --env WITNESS_NAMES="$witness_names" \
       	--env WITNESS_NAME="webuilder" \
        -d -p 2001:2001 -p 8090:8090 --name wenode-test \
        lopudesigns/wenode-test

