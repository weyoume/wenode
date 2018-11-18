#!/bin/bash

echo /tmp/core | tee /proc/sys/kernel/core_pattern
ulimit -c unlimited

# if we're not using PaaS mode then start node traditionally with sv to control it
if [[ ! "$USE_PAAS" ]]; then
  mkdir -p /etc/service/node
  cp /usr/local/bin/node-sv-run.sh /etc/service/node/run
  chmod +x /etc/service/node/run
  runsv /etc/service/node
else
  /usr/local/bin/startpaasnode.sh
fi
