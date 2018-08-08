#!/bin/bash

num=99
offset=1
witness_names=""
witness_base_name="ezbuilder"
for n in $num ;	do
	$witness_names+="$witness_base_name$((n+offset))"
done

docker run \
	--env USE_WAY_TOO_MUCH_RAM=1 \
        --env USE_FULL_WEB_NODE=1 \
        --env EZNODE_PRIVATE_KEY="5JbDVBTnNJh2WCPjHqEpSBKgnmm6cVE3tGNTW4HQZ9kYzDnAM11" \
        --env EZIRA_WITNESS_NAMES=$witness_names \
       	--env EZNODE_WITNESS_NAME="ezbuilder" \
        -d -p 2001:2001 -p 8090:8090 --name ezira-test-node \
        lopudesigns/ezira-test-node

