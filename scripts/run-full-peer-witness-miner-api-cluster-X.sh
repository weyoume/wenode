#!/bin/bash

export subnet=172.20.0.
export peerCount=${1:-10}
witnesses="$(cat ./contrib/credentials/witnesses.txt | awk -F' ' '{print $1}')"
witnesses=($witnesses)
keys="$(cat ./contrib/credentials/witnesses.txt | awk -F' ' '{print $2}')"
keys=($keys)

for ((i=0;i<${#witnesses[@]};i++))
	do
		echo "#### LOOP "$1" ####"
		echo "starting docker container ezira"$i" with witness "${witnesses[$i]}
		echo "no linked ports, nginx-router will handle"
		echo "image eziranetwork/ezira"
		echo "not load balanced internally"
		echo "full web node"
		echo "peer, witness, api"

		docker run --env USE_WAY_TOO_MUCH_RAM=1 --env USE_FULL_WEB_NODE=1 --env STEEMD_WITNESS_NAME=${witnesses[$i]} --env STEEMD_PRIVATE_KEY=${keys[$i]}  -d --name ezira$i eziranetwork/ezira && 
		# sleep 1
done

echo "ALL DONE :D you started "${#witnesses[@]}" nodes"