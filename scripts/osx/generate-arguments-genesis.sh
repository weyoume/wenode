#!/bin/bash

# will make webuilder1 - webuilder99 if end=100 and start=0
start=1
end=50
witness_names=""
witness_base_name=webuilder
args=" --witness=\\\"$witness_base_name\\\""
for ((n=$start;n<$end;n++)); do
	args+=" --witness=\\\"$witness_base_name$((n))\\\""
done

args+=" --rpc-endpoint=\"127.0.0.1:8090\""
args+=" --p2p-endpoint=\"0.0.0.0:2001\""
args+=" --data-dir=\"dbs/ubuntu/\""

touch scripts/generate-arguments-genesis.output
echo $args > scripts/generate-arguments-genesis.output

