#!/bin/bash
# Slocum RT file pipeline for a single compressed dinkum binary (XCD)
#
# Requirements
# * slocum_xcd2xba.bash
# * rclone
set -euo pipefail
glider_id=$1
xcd=$2 # full path of xcd
mission_dir="${3:-${RTD_MISSION_DIR:-}}"
mission_blob="${4:-${RTD_MISSION_BLOB:-}}"

xcdbase="$( basename $xcd)"
xba="${xcdbase%cd}ba"

rt_xcd_dir="$mission_dir/$glider_id/rt_dinkum_cbinary"
rt_xba_dir="$mission_dir/$glider_id/rt_dinkum_ascii"

echo "[$(date -Is)] Slocum RT file pipeline: $glider_id $xcdbase"
echo "[$(date -Is)] Slocum RT cbinary: $rt_xcd_dir"
echo "[$(date -Is)] Slocum RT ascii: $rt_xba_dir"

# 0. Create/ensure local mission archive dir
mkdir -p $rt_xcd_dir
mkdir -p $rt_xba_dir

# 1. Copy incoming file to local archive
cp -f "$xcd" "$rt_xcd_dir"

# 2. Convert to dinkum ascii using slocum_xcd2xba.bash
here="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
xcd2xba="$here/slocum_xcd2xba.bash"
[[ ! -x "$xcd2xba" ]] && { echo "ERROR missing: $xcd2xba"; exit 1; }

$xcd2xba "$xcd" "$rt_xba_dir/$xba"

# 3. Send dinkum ascii (XBA) to RTD and blob storage
if [ -s "$rt_xba_dir/$xba" ]; then
  # -> RTD
  url="https://rtd.akvaplan.no/slocum/xba"
  echo "[$(date -Is)] POST $url <- $xba"
  curl -s -u "$RTD_API_USER:$RTD_API_KEY_CREATE" \
    --data-binary @"$rt_xba_dir/$xba" \
    -H "content-type: text/plain" \
    "$url"
  # -> Blob

fi

# 4. Send dinkum binary to cloud blob
# FIXME
# echo $mission_blob