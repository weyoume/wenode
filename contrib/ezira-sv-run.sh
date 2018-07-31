#!/bin/bash

EZIRAD="/usr/local/eznode-default/bin/eznode"

VERSION=`cat /etc/eznodeversion`

if [[ "$USE_WAY_TOO_MUCH_RAM" ]]; then
    EZIRAD="/usr/local/eznode-full/bin/eznode"
fi

chown -R eznode:eznode $HOME

# seed nodes come from doc/seednodes.txt which is
# installed by docker into /etc/eznode/seednodes.txt
SEED_NODES="$(cat /etc/eznode/seednodes.txt | awk -F' ' '{print $1}')"

ARGS=""

# if user did not pass in any desired
# seed nodes, use the ones above:
if [[ -z "$EZIRAD_SEED_NODES" ]]; then
    for NODE in $SEED_NODES ; do
        ARGS+=" --seed-node=$NODE"
    done
fi

# if user did pass in desired seed nodes, use
# the ones the user specified:
if [[ ! -z "$EZIRAD_SEED_NODES" ]]; then
    for NODE in $EZIRAD_SEED_NODES ; do
        ARGS+=" --seed-node=$NODE"
    done
fi

if [[ ! -z "$EZIRAD_WITNESS_NAME" ]]; then
    ARGS+=" --witness=\"$EZIRAD_WITNESS_NAME\""
fi

if [[ ! -z "$EZIRAD_MINER_NAME" ]]; then
    ARGS+=" --miner=[\"$EZIRAD_MINER_NAME\",\"$EZIRAD_PRIVATE_KEY\"]"
    ARGS+=" --mining-threads=$(nproc)"
fi

if [[ ! -z "$EZIRAD_PRIVATE_KEY" ]]; then
    ARGS+=" --private-key=$EZIRAD_PRIVATE_KEY"
fi

if [[ ! -z "$TRACK_ACCOUNT" ]]; then
    if [[ ! "$USE_WAY_TOO_MUCH_RAM" ]]; then
        ARGS+=" --enable-plugin=account_history"
    fi
    ARGS+=" --track-account-range=[\"$TRACK_ACCOUNT\",\"$TRACK_ACCOUNT\"]"
fi

NOW=`date +%s`
EZIRAD_FEED_START_TIME=`expr $NOW - 1209600`

ARGS+=" --follow-start-feeds=$EZIRAD_FEED_START_TIME"

# overwrite local config with image one
if [[ "$USE_FULL_WEB_NODE" ]]; then
  cp /etc/eznode/fullnode.config.ini $HOME/config.ini
elif [[ "$IS_BROADCAST_NODE" ]]; then
  cp /etc/eznode/config-for-broadcaster.ini $HOME/config.ini
elif [[ "$IS_AH_NODE" ]]; then
  cp /etc/eznode/config-for-ahnode.ini $HOME/config.ini
else
  cp /etc/eznode/config.ini $HOME/config.ini
fi

chown eznode:eznode $HOME/config.ini

if [[ ! -d $HOME/blockchain ]]; then
    if [[ -e /var/cache/eznode/blocks.tbz2 ]]; then
        # init with blockchain cached in image
        ARGS+=" --replay-blockchain"
        mkdir -p $HOME/blockchain/database
        cd $HOME/blockchain/database
        tar xvjpf /var/cache/eznode/blocks.tbz2
        chown -R eznode:eznode $HOME/blockchain
    fi
fi

# without --data-dir it uses cwd as datadir(!)
# who knows what else it dumps into current dir
cd $HOME

# if [[ "$USE_PUBLIC_SHARED_MEMORY" ]]; then
#   echo eznode: Downloading and uncompressing blockchain-$VERSION-latest.tar.lz4 - this may take awhile.
#   wget -qO- https://s3.amazonaws.com/steemit-dev-blockchainstate/blockchain-$VERSION-latest.tar.lz4 | lz4 -d | tar x
# fi

if [[ "$USE_PUBLIC_BLOCKLOG" ]]; then
  if [[ ! -e $HOME/blockchain/block_log ]]; then
    if [[ ! -d $HOME/blockchain ]]; then
      mkdir -p $HOME/blockchain
    fi
    echo "eznode: Downloading a block_log and replaying the blockchain"
    echo "This may take a little while..."
    wget -O $HOME/blockchain/block_log https://s3.amazonaws.com/steemit-dev-blockchainstate/block_log-latest
    ARGS+=" --replay-blockchain"
  fi
fi

# slow down restart loop if flapping
sleep 1

mv /etc/nginx/nginx.conf /etc/nginx/nginx.original.conf
cp /etc/nginx/eznode.nginx.conf /etc/nginx/nginx.conf

#start multiple read-only instances based on the number of cores
#attach to the local interface since a proxy will be used to loadbalance
if [[ "$USE_MULTICORE_READONLY" ]]; then
    exec chpst -ueznode \
        $EZIRAD \
            --rpc-endpoint=127.0.0.1:8091 \
            --p2p-endpoint=0.0.0.0:2001 \
            --data-dir=$HOME \
            $ARGS \
            $EZIRAD_EXTRA_OPTS \
            2>&1 &
    # sleep for a moment to allow the writer node to be ready to accept connections from the readers
    sleep 30
    PORT_NUM=8092
    cp /etc/nginx/healthcheck.conf.template /etc/nginx/healthcheck.conf
    CORES=$(nproc)
    PROCESSES=$((CORES * 4))
    for (( i=2; i<=$PROCESSES; i++ ))
      do
        echo server 127.0.0.1:$PORT_NUM\; >> /etc/nginx/healthcheck.conf
        ((PORT_NUM++))
    done
    echo } >> /etc/nginx/healthcheck.conf
    PORT_NUM=8092
    for (( i=2; i<=$PROCESSES; i++ ))
      do
        exec chpst -ueznode \
        $EZIRAD \
          --rpc-endpoint=127.0.0.1:$PORT_NUM \
          --data-dir=$HOME \
          --read-forward-rpc=127.0.0.1:8091 \
          --read-only \
          2>&1 &
          ((PORT_NUM++))
          sleep 1
    done
    # start nginx now that the config file is complete with all endpoints
    # all of the read-only processes will connect to the write node onport 8091
    # nginx will balance all incoming traffic on port 8090
    rm /etc/nginx/sites-enabled/default
    cp /etc/nginx/healthcheck.conf /etc/nginx/sites-enabled/default
    /etc/init.d/fcgiwrap restart
    echo daemon off\; >> /etc/nginx/nginx.conf
    service nginx restart
elif [[ "$USE_NGINX_FRONTEND" ]]; then
    cp /etc/nginx/healthcheck.conf.template /etc/nginx/healthcheck.conf
    echo server 127.0.0.1:8091\; >> /etc/nginx/healthcheck.conf
    echo } >> /etc/nginx/healthcheck.conf
    rm /etc/nginx/sites-enabled/default
    cp /etc/nginx/healthcheck.conf /etc/nginx/sites-enabled/default
    /etc/init.d/fcgiwrap restart
    service nginx restart
    exec chpst -ueznode \
        $EZIRAD \
            --rpc-endpoint=0.0.0.0:8091 \
            --p2p-endpoint=0.0.0.0:2001 \
            --data-dir=$HOME \
            $ARGS \
            $EZIRAD_EXTRA_OPTS \
            2>&1
else
    exec chpst -ueznode \
        $EZIRAD \
            --rpc-endpoint=0.0.0.0:8090 \
            --p2p-endpoint=0.0.0.0:2001 \
            --data-dir=$HOME \
            $ARGS \
            $EZIRAD_EXTRA_OPTS \
            2>&1
fi
