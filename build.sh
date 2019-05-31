#!/bin/sh

set -e
cd "$(dirname "$0")"

aclocal
autoconf
autoheader
automake --add-missing
./configure "$@"
make -j4
