#!/bin/sh

Usage ()
{
    echo "usage: run.sh [-s|--sdl sdlpath] gameexecutable [game options]"
}

gamepath=/home/clement/supermeatboy/amd64/SuperMeatBoy
sdlpath=../supermeatboy/amd64/libSDL2-2.0.so.0
movieopt=

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
    -r | --read)    shift
                    movieopt="-r $1"
                    ;;
    -w | --write)   shift
                    movieopt="-w $1"
                    ;;
    -*)             Usage
                    exit
                    ;;
    *)              gamepath=$1
                    shift
                    break
                    ;;
    esac
    shift
done

# Some games do not work if it was not launched inside its folder
cd ${gamepath%/*}

# Get the list of all shared libraries used by the game
# Source: http://unix.stackexchange.com/a/101833
SHLIBS=

mypipe=/tmp/libpipe
if [ ! -p "$mypipe" ]
then
    mkfifo $mypipe
fi
ldd ./${gamepath##*/} | awk '/=>/{print $(NF-1)}' > $mypipe &

while read lib
do SHLIBS="$SHLIBS -l $lib"
done < $mypipe

# Launching the game with the libTAS library as LD_PRELOAD
LD_PRELOAD=$OLDPWD/bin/libTAS.so ./${gamepath##*/} "$@" &
cd - > /dev/null
sleep 1

# Get the absolute path for the SDL library
case $sdlpath in
    /*) absolute=$sdlpath;;
     *) absolute=$PWD/$sdlpath;;
esac

# Launch the TAS program
./bin/linTAS -s $absolute $SHLIBS $movieopt

