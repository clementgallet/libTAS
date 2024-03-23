#!/bin/sh

set -e
cd "$(dirname "$0")"

aclocal
autoconf
autoheader
automake --add-missing
mkdir -p build
cd build
CXXFLAGS="-O2 -g -Wall -pedantic" ../configure "$@"
make -j4
