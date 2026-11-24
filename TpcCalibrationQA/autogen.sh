#!/bin/sh
set -eu
srcdir=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
: "${OFFLINE_MAIN:?Source the sPHENIX environment before building}"
(cd "$srcdir"
 aclocal -I "$OFFLINE_MAIN/share"
 libtoolize --force
 automake -a --add-missing
 autoconf)
exec "$srcdir/configure" "$@"
