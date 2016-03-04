#!/bin/sh

Usage ()
{
    echo "usage: system_page [[[-f file ] [-i]] | [-h]]"
}

gamepath=/home/clement/supermeatboy/amd64/SuperMeatBoy
sdlpath=/home/clement/supermeatboy/amd64/libSDL2-2.0.so.0

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

LD_PRELOAD=./bin/libTAS.so $gamepath &
sleep 1
./bin/linTAS -s $sdlpath

