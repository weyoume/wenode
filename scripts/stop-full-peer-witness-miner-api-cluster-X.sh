#!/bin/bash

export subnet=172.20.0.
export peerCount=10
witnesses="$(cat ./contrib/credentials/witnesses.txt | awk -F' ' '{print $1}')"
witnesses=($witnesses)
keys="$(cat ./contrib/credentials/witnesses.txt | awk -F' ' '{print $2}')"
keys=($keys)

for ((i=0;i<${#witnesses[@]};i++))
	do
		echo "LOOP #"$i
		echo "stopping docker container ezira"$i" with witness "${witnesses[$i]}
		# docker run --env USE_WAY_TOO_MUCH_RAM=1 --env USE_FULL_WEB_NODE=1 --env STEEMD_WITNESS_NAME=${witnesses[$i]} --env STEEMD_PRIVATE_KEY=${keys[$i]} --env STEEMD_MINER_NAME=${witnesses[$i]} -d --name ezira$i eziranetwork/ezira
		docker stop ezira$i
		# sleep 1
done

echo "ALL DONE :D you stopped "${#witnesses[@]}" nodes"