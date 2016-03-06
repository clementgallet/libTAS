#!/bin/sh

Usage ()
{
    echo "usage: run.sh [-s|--sdl sdlpath] gameexecutable"
}

gamepath=/home/clement/supermeatboy/amd64/SuperMeatBoy
sdlpath=../supermeatboy/amd64/libSDL2-2.0.so.0

# Parse command-line arguments
while [ $# -gt 0 ]
do
    case "$1" in
    -s | --sdl)     shift
                    sdlpath=$1
                    ;;
    -h | --help)    Usage
                    exit
                    ;;
    -*)             Usage
                    exit
                    ;;
    *)              gamepath=$1
                    ;;
    esac
    shift
done

# Some games do not work if it was not launched inside its folder

cd ${gamepath%/*}
LD_PRELOAD=$OLDPWD/bin/libTAS.so ./${gamepath##*/} &
cd - > /dev/null
sleep 1
./bin/linTAS -s $(pwd)/$sdlpath

