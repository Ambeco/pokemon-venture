#!/bin/sh
# Usage: cc1cache.sh <cc1> <cc1 args...> < preprocessed.c > out.s
#
# Compiler cache for the cpp | preproc | cc1 pipeline (ccache can't wrap cc1 directly). The key is the
# hash of the compiler version, its arguments and the fully preprocessed input, so a header edit only
# recompiles the files whose preprocessed text actually changed. Entries are gzipped assembly in
# $CC1_CACHE_DIR (default ~/.cache/pokemon-venture-cc1); delete the directory to clear it, or
# `find $dir -atime +14 -delete` to trim. Warnings are only shown on a cache miss.
set -e
dir=${CC1_CACHE_DIR:-$HOME/.cache/pokemon-venture-cc1}
mkdir -p "$dir"
tmp=$(mktemp)
trap 'rm -f "$tmp" "$tmp.s"' EXIT
cat > "$tmp"
key=$( { stat -c '%s %Y' "$1"; printf '%s\n' "$@"; cat "$tmp"; } | sha256sum | cut -c1-64 )
if [ -f "$dir/$key.s.gz" ]; then
    touch "$dir/$key.s.gz"
    gzip -dc "$dir/$key.s.gz"
    exit 0
fi
"$@" < "$tmp" > "$tmp.s"
gzip -1 -c "$tmp.s" > "$dir/$key.s.gz.$$" && mv "$dir/$key.s.gz.$$" "$dir/$key.s.gz"
cat "$tmp.s"
