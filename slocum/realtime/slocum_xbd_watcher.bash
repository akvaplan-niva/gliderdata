#!/bin/bash
# Real-time inotify watcher for incoming dinkum binary Slocum files
#
# Documentation:
# https://github.com/akvaplan-niva/gliderdata/blob/main/slocum/realtime/readme.md
#
# Requirements
# * inotifywait # apt install inotify-tools
#
# Links on inotify:
# https://github.com/inotify-tools/inotify-tools/wiki
# https://developer.ibm.com/tutorials/l-inotify/ 

here="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
slocum_rt_xbd="$here/slocum_rt_xbd.bash"

echo "[$(date -Is)] Starting Slocum RT XBD watcher"
echo "CAC:$SLOCUM_CAC_CACHE"

inotifywait -m -r "/var/opt/sfmc-dockserver/stations/default/gliders" \
  --quiet \
  --event close_write \
  --event moved_to \
  --include '[0-9]{4}-[0-9]{3}-[0-9]{1,}-[0-9]{1,}\.[a-z]bd$' \
  --format '%w %f %e' |
while read -r dir file event; do
  echo "[$(date -Is)] Slocum RT watcher <- $dir$file [$event]"
  if [[ "$dir" =~ /([^/]+)/(from-glider) ]]; then
    glider_id="${BASH_REMATCH[1]}"
    "$slocum_rt_xbd" "$glider_id" "$dir$file"
  fi
done
