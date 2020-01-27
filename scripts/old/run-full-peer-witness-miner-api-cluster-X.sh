#!/bin/bash

export subnet=172.20.0.
producers="$(cat ./contrib/credentials/producers.txt | awk -F' ' '{print $1}')"
producers=($producers)
export peerCount=${1:-${#producers[@]}}
keys="$(cat ./contrib/credentials/producers.txt | awk -F' ' '{print $2}')"
keys=($keys)
nodescount=0
docker network create nodes
for ((i=0;i<$peerCount;i++)) ; do
	if [[ ! -z ${producers[$i]} ]] ; then
		echo "#### LOOP "$i" ####"
		echo "starting docker container WeYouMe"$i" with producer "${producers[$i]}
		echo "no linked ports, nginx-router will handle"
		echo "image weyoume/wenode"
		echo "not load balanced internally"
		echo "full web node"
		echo "peer, producer, api"

		docker run --ip $subnet$(($i+3)) \
			--network nodes \
			--env USE_FULLNODE=1 \
			--env USE_NGINX_FRONTEND=1 \
			--env PRODUCER_NAME=${producers[$i]} \
			--env PRIVATE_KEY=${keys[$i]}  \
			-d --name wenode$i \
			weyoume/wenode &
		((nodescount++))
		# sleep 1
	fi
done

echo "ALL DONE :D you started "$nodescount" nodes out of "$peerCount" desired"