#!/bin/bash

docker rm -f ezira-test-node ;


# will make ezbuilder1 - ezbuilder99 if end=100 and start=0
end=100
start=0
witness_names=""
witness_base_name="ezbuilder"
for ((n=$start;n<$end;n++)); do
	witness_names+="$witness_base_name$((n)) "
done

echo [$witness_names]

docker run \
	--env USE_WAY_TOO_MUCH_RAM=1 \
        --env USE_FULL_WEB_NODE=1 \
        --env PRIVATE_KEY="5JbDVBTnNJh2WCPjHqEpSBKgnmm6cVE3tGNTW4HQZ9kYzDnAM11" \
        --env WITNESS_NAMES="$witness_names" \
       	--env WITNESS_NAME="ezbuilder" \
        -d -p 2001:2001 -p 8090:8090 --name ezira-test-node \
        lopudesigns/ezira-test-node

