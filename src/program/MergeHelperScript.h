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
    #first video is a special case
    echo file $VID_PATH
    
    #sort command can ensure the rest are in order
    temp_var="$VID_DIR"/"$VID_NAME"_
    for i in $(ls -v "$VID_DIR"/"$VID_NAME"_*"$VID_EXT")
    do
      echo file $i
    done
}

sync
ffmpeg -f concat -safe 0 -i <(getChunks) -c copy -y $VID_DIR/$VID_NAME-merged.mkv

)";

#endif
