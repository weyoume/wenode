args=""
# EITHER ADD YOUR PRIVATE KEYS AS A LIST IN ./scripts/private-keys
# eg. no whitespaces start and finish of the file
# either
# "privatekey1 privatekey2 privatek3"
# or
# "privatekey1 \
# privatekey2 \
# privatekey3"

# and use the following PRIVATE_KEYS definition
PRIVATE_KEYS=$(cat ./scripts/private-keys)
# OR
# PRIVATE_KEYS="privatekey1 privatekey2 privatekey3"

if [[ ! -z "$PRIVATE_KEYS" ]]; then
    for PRIVATE_KEY in $PRIVATE_KEYS ; do
        args+=" --private-key=$PRIVATE_KEY"
    done
fi

# The node software will automatically match the private keys with
# all the corresponding producer account names you input or generate

# either generate with the following code
# will make webuilder1 - webuilder99 if start=0 and end=100
start=1
end=50
producer_base_name="webuilder"
args+=" --producer=\"$producer_base_name\""
for ((n=$start;n<$end;n++)); do
	args+=" --producer=\"$producer_base_name$((n))\""
done

# add argument
# args+=' --replay-blockchain'
# add argument
args+=' --rpc-endpoint=127.0.0.1:8090'

# add argument
args+=' --p2p-endpoint=0.0.0.0:2001'

# add argument
args+=' --data-dir="dbs/osx"'

# add argument
args+=' --config="contrib/config.ini"'

# or set producer names manually as a list
# producer_names="producername1 producername2 producername3"
# or use a file
# producer_names=$(cat ./scripts/private-producer-names)
# producer names don't need to be private but the repo ignores
# files beginning with scripts/private* when commiting and publishing

# output producer names
echo [$producer_names]

# $repos is an environment variable which is 
# an absolute path to the directory your github 
# repo for the WeYouMe node software resides in

./osx_build/programs/node/node $args