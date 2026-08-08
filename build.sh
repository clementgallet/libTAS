#!/bin/sh

set -e
cd "$(dirname "$0")"

SKIP_APPIMAGE=0
if [ "$1" = "--skip-appimage" ]; then
    SKIP_APPIMAGE=1
    shift
fi

aclocal
autoconf
autoheader
automake --add-missing
mkdir -p build
cd build
CXXFLAGS="-O2 -g -Wall -pedantic" ../configure "$@"
make -j$(nproc)

# Build .AppImage

ARCH=$(uname -m)
if [ $SKIP_APPIMAGE -eq 0 ]; then
    wget -nc https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-$ARCH.AppImage
    chmod +x linuxdeploy-$ARCH.AppImage
    wget -nc https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-$ARCH.AppImage
    chmod +x linuxdeploy-plugin-qt-$ARCH.AppImage
fi

APPDIR_PATH=$(readlink -f ./AppDir)

make prefix=/usr DESTDIR=$APPDIR_PATH install
if [ $SKIP_APPIMAGE -eq 0 ]; then
    ./linuxdeploy-$ARCH.AppImage --appimage-extract-and-run --appdir AppDir/ --plugin qt --output appimage
fi
