#!/bin/bash
# Real-time detection of incoming files from a Slocum,
# restricted to compressed binary dinkum, 
# ie. files  with extensions: .dcd .ecd .scd .tcd
#
# Run: source .env && ./slocum_rt_watcher.bash
#
# Requirements
# * inotifywait # apt install inotify-tools
#
# Links on inotify
# https://github.com/inotify-tools/inotify-tools/wiki
# https://developer.ibm.com/tutorials/l-inotify/ 

watch_dir="${1:-"/var/opt/gmc/gliders"}"
mission_dir="${2:-${RTD_MISSION_DIR:-}}"
mission_blob="${3:-${RTD_MISSION_BLOB:-}}"

here="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
cmd="$( basename $0)"
usage="Usage: $cmd [watch-dir] [mission-dir] [mission-blob]"

[ ! -d "$watch_dir" ] && { echo "$usage"; exit 1; }

slocum_rt_pipeline="$here/slocum_rt_pipeline_xcd_file.bash"
[[ ! -x "$slocum_rt_pipeline" ]] && { echo "ERROR missing: $slocum_rt_pipeline"; exit 1; }

echo "[$(date -Is)] Slocum RT watcher active for $watch_dir"
echo "[$(date -Is)] Mission dir: $mission_dir"
echo "[$(date -Is)] Mission blob: $mission_blob"

inotifywait -m -r "$watch_dir" \
    --quiet \
    --timeout 600 \
    --event close_write \
    --event moved_to \
    --include '\.(dcd|ecd|scd|tcd)$' \
    --format '%w %f %e' |
while read -r dir file event; do
  echo "[$(date -Is)] <- $dir$file [$event]"
  if [[ "$dir" =~ /([^/]+)/(from-glider) ]]; then
      glider_id="${BASH_REMATCH[1]}"
      "$slocum_rt_pipeline" "$glider_id" "$dir$file" "$mission_dir" "$mission_blob"
  fi
done
