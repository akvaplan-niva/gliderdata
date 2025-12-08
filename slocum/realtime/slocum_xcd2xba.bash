#!/bin/bash
# slocum_xcd2xba.bash
# Converts a compressed dinkum binary Slocum file into DBD ASCII
#
# Env: SLOCUM_CAC_CACHE must point to a folder with relevant `.cac` files
#
# Requirements
# * compexp
# * dbd2asc
#
# Useful documentation on Slocum file formats prior to compressed binary: 
# https://github.com/kerfoot/slocum-utils/blob/master/doco/dbd_file_format.txt
set -euo pipefail

xcd="${1:-}"
[[ -z "$xcd" || ! -f "$xcd" ]] && { echo "Usage:\n\n$0 <xcd> [xba]"; exit 1; }

base=$(basename "$xcd")
xba="${2:-"${xcd%cd}ba"}"

cache=$SLOCUM_CAC_CACHE
echo "SLOCUM_CAC_CACHE:$cache"
tmpdir=$(mktemp -d)

# Uncompress
tmpfile="$tmpdir/$base"
compexp x "$xcd" "$tmpfile"

# (Rename)
# Rename (expand 8x3 filename, eg. "07950000.tcd" => "apn_936-2025-336-4-0.tcd")
# tmp_renamed=$(rename_dbd_files "$tmpfile")
tmp_renamed="$tmpfile"

# Create XBA
tmpxba="${tmp_renamed%cd}ba"
echo "dbd2asc -c $cache $tmp_renamed > $tmpxba"
if ! dbd2asc -c "$cache" "$tmp_renamed" > "$tmpxba"; then
  exit $?
fi
mv "$tmpxba" "$xba"
wc -l $xba

# Cleanup
[[ -d "$tmpdir" ]] && rm -f "$tmpdir/*"