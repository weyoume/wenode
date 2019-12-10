#!/bin/bash

docker rm -f wenode ;


# will make webuilder1 - webuilder99 if end=100 and start=0
start=1
end=50
witness_names=""
witness_base_name="webuilder"
for ((n=$start;n<$end;n++)); do
	witness_names+="$witness_base_name$((n)) "
done

echo [$witness_names]

docker run \
    --env USE_FULLNODE=1 \
	--env USE_NGINX_FRONTEND=1 \
	--env GEN_PRIVATE_KEY=1 \
	--env SHOW_PRIVATE_KEYS=1 \
    --env WITNESS_NAMES="$witness_names" \
    --env WITNESS_NAME="webuilder" \
    -d -p 2001:2001 -p 8090:8090 
    --name wenode \
    weyoume/wenode

