#!/bin/bash

docker rm -f wenode ;

# will make webuilder1 - webuilder99 if end=100 and start=0

start=1
end=50
producer_names=""
producer_base_name="webuilder"
for ((n=$start;n<$end;n++)); do
	producer_names+="$producer_base_name$((n)) "
done

echo [$producer_names]

docker run \
  --env USE_FULLNODE=1 \
  --env USE_NGINX_FRONTEND=1 \
  --env PRIVATE_KEY=$PRIVATE_KEY \
  --env PRODUCER_NAMES="$producer_names" \
  --env PRODUCER_NAME="webuilder" \
  -d -p 2001:2001 -p 8090:8090 --name wenode \
  weyoume/wenode

