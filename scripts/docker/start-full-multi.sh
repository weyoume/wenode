#!/bin/bash

# this will remove the currently running docker node process
# be careful
docker rm -f testnet-wenode ;

# EITHER ADD YOUR PRIVATE KEYS AS A LIST IN ./scripts/private-keys
# eg. no whitespaces start and finish of the file
# either
# "privatekey1 privatekey2 privatek3"
# or
# "privatekey1 \
# privatekey2 \
# privatekey3"

# and use the following PRIVATE_KEYS definition
PRIVATE_KEYS=$(cat /home/haz/things/git/weyoume/wenode/scripts/private-keys)

# OR
# PRIVATE_KEYS="privatekey1 privatekey2 privatekey3"

# The node software will automatically match the private keys with
# all the corresponding witness account names you input or generate


# either generate with the following code
# will make webuilder1 - webuilder99 if start=0 and end=100
start=1
end=50
witness_names=""
witness_base_name="webuilder"
for ((n=$start;n<$end;n++)); do
	witness_names+="$witness_base_name$((n)) "
done

echo [$witness_names]

docker run \
	--env USE_WAY_TOO_MUCH_RAM=1 \
	--env USE_FULLNODE=1 \
	--env USE_NGINX_FRONTEND=1 \
	--env PRIVATE_KEYS=$PRIVATE_KEYS \
	--env WITNESS_NAMES="$witness_names" \
	--env WITNESS_NAME="webuilder" \
	-d -p 2001:2001 -p 8090:8090 --name testnet-wenode \
	weyoume/testnet-wenode

