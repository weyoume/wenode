#!/bin/bash

export subnet=172.20.0.
witnesses="$(cat ./contrib/credentials/witnesses.txt | awk -F' ' '{print $1}')"
witnesses=($witnesses)
export peerCount=${1:-${#witnesses[@]}}
keys="$(cat ./contrib/credentials/witnesses.txt | awk -F' ' '{print $2}')"
keys=($keys)
nodescount=0
for ((i=0;i<$peerCount;i++))
if [[ ! -z ${witnesses[$i]}]]; then
	do
		echo "#### LOOP "$1" ####"
		echo "starting docker container ezira"$i" with witness "${witnesses[$i]}
		echo "no linked ports, nginx-router will handle"
		echo "image eziranetwork/ezira"
		echo "not load balanced internally"
		echo "full web node"
		echo "peer, witness, api"

		docker run --env USE_WAY_TOO_MUCH_RAM=1 --env USE_FULL_WEB_NODE=1 --env STEEMD_WITNESS_NAME=${witnesses[$i]} --env STEEMD_PRIVATE_KEY=${keys[$i]}  -d --name ezira$i eziranetwork/ezira & 
		(($nodescount++))
		# sleep 1
done

echo "ALL DONE :D you started "$nodescount" nodes out of "$peerCount" desired"