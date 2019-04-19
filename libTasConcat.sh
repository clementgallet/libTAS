#!/bin/bash

# absolute path to the first video (eg. /home/user/tasproject01/video.avi) is passed as command line argument 1

VID_PATH=$1
VID_EXT=$(echo $VID_PATH | sed 's/^.*\././g')
VID_DIR=$(dirname $VID_PATH)
VID_NAME=$(basename -s $VID_EXT $VID_PATH)

getChunks(){
    #first video is a special case
    echo file $VID_PATH
    
    #sort command can ensure the rest are in order
    temp_var="$VID_DIR"/"$VID_NAME"_
    for i in $(sort -nk 1.$(( ${#temp_var} + 1))  <(ls -1 "$VID_DIR"/"$VID_NAME"_*"$VID_EXT"))
    do
      echo file $i
    done
}

sync
yes | ffmpeg -f concat -safe 0 -i <(getChunks) -c copy $VID_DIR/$VID_NAME-merged.mkv
