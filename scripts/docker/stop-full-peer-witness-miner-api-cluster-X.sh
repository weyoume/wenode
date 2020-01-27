#!/bin/bash

export subnet=172.20.0.
producers="$(cat ./contrib/credentials/producers.txt | awk -F' ' '{print $1}')"
producers=($producers)
export peerCount=${1:-${#producers[@]}}
keys="$(cat ./contrib/credentials/producers.txt | awk -F' ' '{print $2}')"
keys=($keys)
nodescount=0
for ((i=0;i<$peerCount;i++)) ; do
	if [[ ! -z "${producers[$i]}" ]] ; then
		echo "#### LOOP "$i" ####"
		echo "stopping docker container wenode"$i" with producer "${producers[$i]}
		# docker run --env USE_FULLNODE=1 --env PRODUCER_NAME=${producers[$i]} --env PRIVATE_KEY=${keys[$i]} --env MINER_NAME=${producers[$i]} -d --name wenode$i weyoume/wenode
		docker stop wenode$i & 
		((nodescount++))
		sleep 1
	fi
done

echo "ALL DONE :D you stopped "$nodescount" nodes out of "$peerCount" desired"