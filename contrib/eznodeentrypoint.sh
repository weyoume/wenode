#!/bin/bash

echo /tmp/core | tee /proc/sys/kernel/core_pattern
ulimit -c unlimited

# if we're not using PaaS mode then start eznode traditionally with sv to control it
if [[ ! "$USE_PAAS" ]]; then
  mkdir -p /etc/service/eznode
  cp /usr/local/bin/ezira-sv-run.sh /etc/service/eznode/run
  chmod +x /etc/service/eznode/run
  runsv /etc/service/eznode
else
  /usr/local/bin/startpaaseznode.sh
fi
