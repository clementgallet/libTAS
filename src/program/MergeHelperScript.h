/*
    Copyright 2015-2019 Clément Gallet <clement.gallet@ens-lyon.org>

    This file is part of libTAS.

    libTAS is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    libTAS is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with libTAS.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MERGE_HELPER_SCRIPT_HEADER
#define MERGE_HELPER_SCRIPT_HEADER

/* Embeds Bash helper script as a C++11 raw string.
 *
 * This avoids having to cart around the script as a separate file.
 *
 * A "raw" string also avoids having to deal with escape sequences.
 * However, if your syntax highlighter isn't aware of raw strings,
 * then this may look a bit like a train wreck. */
 
 /*try to avoid using single-quoted strings within the script, it may make it harder to run later.*/
 
const static char* MERGE_HELPER_SCRIPT = R"(

# absolute path to the first video (eg. /home/user/tasproject01/video.avi) is passed as command line argument 1

VID_PATH=$1
VID_EXT=$(echo $VID_PATH | sed "s/^.*\././g")
VID_DIR=$(dirname $VID_PATH)
VID_NAME=$(basename -s $VID_EXT $VID_PATH)

getChunks(){
    for i in $(ls -v "$VID_DIR"/"$VID_NAME"_*"$VID_EXT")
    do
      echo file $i
    done
}

sync
ffmpeg -f concat -safe 0 -i <(getChunks) -c copy -y $VID_DIR/$VID_NAME-merged.mkv

)";

#endif
