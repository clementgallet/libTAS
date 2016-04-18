#!/bin/sh

# Remove stall socket here
rm -f /tmp/libTAS.socket

Usage ()
{
    echo "Usage: ./run.sh [options] game_executable_path [game_cmdline_arguments]"
    echo "Options are:"
    echo "  -d, --dump FILE     Start a audio/video encode into the specified FILE"
    echo "  -r, --read MOVIE    Play game inputs from MOVIE file"
    echo "  -w, --write MOVIE   Record game inputs into the specified MOVIE file"
    echo "  -l, --lib     PATH  Manually import a library"
    echo "  -L, --libpath PATH  Indicate a path to additional libraries the game"
    echo "                      will want to import."
    echo "  -R, --runpath PATH  From which directory the game must be launched."
    echo "                      Set to the executable directory by default."
    echo "  -h, --help          Show this message"
}

gamepath=
movieopt=
dumpopt=
libdir=
rundir=
SHLIBS=

# Parse command-line arguments
while [ $# -gt 0 ]
do
    case "$1" in
    -h | --help)    Usage
                    exit
                    ;;
    -d | --dump)    shift
                    dumpopt="-d $1"
                    ;;
    -r | --read)    shift
                    movieopt="-r $1"
                    ;;
    -w | --write)   shift
                    movieopt="-w $1"
                    ;;
    -l | --lib)     shift
                    SHLIBS="${SHLIBS} -l $1"
                    ;;
    -L | --libpath) shift
                    libdir=$1
                    ;;
    -R | --runpath) shift
                    rundir=$1
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

# Change to the directory set by the user, or the game directory by default.
if [ -z $rundir ]
then
    cd "${gamepath%/*}"
else
    cd "$rundir"
fi

# Export optional library directories
echo "export LD_LIBRARY_PATH=\"$OLDPWD/$libdir:$LD_LIBRARY_PATH\""
export LD_LIBRARY_PATH="$OLDPWD/$libdir:$LD_LIBRARY_PATH"

# Get the list of all shared libraries used by the game
# Source: http://unix.stackexchange.com/a/101833

mypipe=/tmp/libpipe
if [ ! -p "$mypipe" ]
then
    mkfifo $mypipe
fi
ldd "$OLDPWD/$gamepath" | awk '/=>/{print $(NF-1)}' > $mypipe &

while read lib
do SHLIBS="$SHLIBS -l $lib"
done < $mypipe

# Launching the game with the libTAS library as LD_PRELOAD
echo "LD_PRELOAD=$OLDPWD/build/libTAS.so $OLDPWD/$gamepath $@ &"
LD_PRELOAD=$OLDPWD/build32/libTAS.so "$OLDPWD/$gamepath" "$@" &
cd - > /dev/null
sleep 1

# Launch the TAS program
echo "./build/linTAS $SHLIBS $movieopt $dumpopt"
./build32/linTAS $SHLIBS $movieopt $dumpopt

