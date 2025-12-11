#!/bin/bash
# slocum_xcd2xba.bash [-c dir] <xcd-file> <xba-filename>
# Converts a compressed dinkum binary (XCD) into DBD ASCII
#
# Requirements
# * compexp
# * dbd2asc
#
# Like `dbd2asc`, .cac directory must be provided in using -c <dir> 
# or via env $SLOCUM_CAC_CACHE
#
# Useful documentation on Slocum file formats (prior to compressed binary):
# https://github.com/kerfoot/slocum-utils/blob/master/doco/dbd_file_format.txt
set -euo pipefail

help="Usage: $0 [-c cache-dir] <xcd-file> [xba-file]"

while getopts "hc:" opt; do
  case $opt in
    c) cache="$OPTARG" ;;
    h) echo $help && exit ;;
    *) exit 1 ;;
  esac
done
shift $((OPTIND-1))

xcd="${1:-}"
[[ -z "$xcd" || ! -f "$xcd" ]] && { echo $help; exit 1; }

base=$(basename "$xcd")
xba="${2:-"${xcd%cd}ba"}"

# Uncompress
tmp_dir=$(mktemp -d)
tmp_dinkum_file="$tmp_dir/$base"
# echo "compexp x $xcd $tmp_dinkum_file"
compexp x "$xcd" "$tmp_dinkum_file"

# Find .cac
# $ head -n20 2025-12-10_test/apn_936/rt_xcd/080* | strings -s ' ' | grep _list_crc
cache="${cache:-${SLOCUM_CAC_CACHE:-}}"
[ ! -d "$cache" ] && { echo "Slocum .cac cache dir ($cache) missing or empty"; exit 1; }

# Create XBA
tmpxba="${tmp_dinkum_file%cd}ba"
# echo "dbd2asc -c $cache $tmp_dinkum_file > $tmpxba [$xba]"
dbd2asc -c "$cache" "$tmp_dinkum_file" > "$tmpxba"

# Save XBA …or send to std out
if [[ -n "${2:-}" ]]; then
  [[ -f "$tmpxba" ]] && mv -f "$tmpxba" "$xba"
else
  [[ -f "$tmpxba" ]] && cat "$tmpxba"
fi

# Cleanup
[[ -d "$tmp_dir" ]] && rm -f "$tmp_dir/*"