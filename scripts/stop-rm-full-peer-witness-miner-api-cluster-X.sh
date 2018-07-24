#!/bin/bash

export subnet=172.20.0.
witnesses="$(cat ./contrib/credentials/witnesses.txt | awk -F' ' '{print $1}')"
witnesses=($witnesses)
export peerCount=${1:-${#witnesses[@]}}
keys="$(cat ./contrib/credentials/witnesses.txt | awk -F' ' '{print $2}')"
keys=($keys)
nodescount=0
for ((i=0;i<$peerCount;i++))
	if [[ ! -z ${witnesses[$i]}]] ; then
		do
			echo "#### LOOP "$1" ####"
			echo "stopping docker container ezira"$i" with witness "${witnesses[$i]}
			# docker run --env USE_WAY_TOO_MUCH_RAM=1 --env USE_FULL_WEB_NODE=1 --env STEEMD_WITNESS_NAME=${witnesses[$i]} --env STEEMD_PRIVATE_KEY=${keys[$i]} --env STEEMD_MINER_NAME=${witnesses[$i]} -d --name ezira$i eziranetwork/ezira
			docker stop ezira$i &
			(($nodescount++))
			sleep 1
done

echo "ALL DONE :D you stopped "$nodescount" nodes out of "$peerCount" desired"

wait

./scripts/rm-full-peer-witness-miner-api-cluster-X.sh