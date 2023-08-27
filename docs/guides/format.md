---
layout: page
title: Moviefile format
permalink: /guides/format/
---

The tool records inputs into a movie file which has a `.ltm` extension. It is actually a `tar.gz` archive which contains four text files.

### Config

The first one is `config.ini`, containing all meta-data of the movie file. It consists of a list of key/value pairs. Here is an example of such a file, with added comments:

    [General]
    exec_path=~/SuperMeatBoy ; path of the game executable
    command_line_args= ; command-line options for the executable
    md5=a6fa991f40ef518b302a4a492af7259c ; md5 hash of the game executable
    frame_count=81 ; frame count of the movie file
    mouse_support=true ; are mouse inputs sent to the game
    nb_controllers=0 ; up to 4 controllers are supported
    initial_time_sec=1 ; number of seconds of the initial system time
    initial_time_nsec=0 ; number of nanoseconds of the initial system time
    length_sec=52 ; number of seconds of movie length
    length_nsec=733333333 ; number of nanoseconds of movie length
    framerate_num=60 ; numerator of the number of frames per second
    framerate_den=1 ; denominator of the number of frames per second
    rerecord_count=0
    authors=me ; authors of the movie
    libtas_major_version=1
    libtas_minor_version=4
    libtas_patch_version=0
    savestate_frame_count=13 ; savestate frame for savestate movies
    auto_restart=false ; does the game restarts automatically when closed
    variable_framerate=true ; is the user allowed to change framerate

    ; Time hack values

    [mainthread_timetrack]
    clock=-1
    clock_gettime=-1
    gettimeofday=-1
    sdl_getperformancecounter=-1
    sdl_getticks=-1
    time=-1

### Inputs

The second file is the `inputs` text file. In this file, each line that starts with the character `|` is an input frame.
The content of one line depends on the two settings `mouse_support` and `nb_controllers` above.

The line is a concatenation of sections for each device. Each section starts with `|` followed by a identifier.

#### Keyboard

Keyboard starts with `|K`, followed by a list of Xlib KeySym (u32) values of each pressed key. They are encoded into an hex string and separated by the `:` character. The list is unordered, the order of the events that will be send to the game for a single frame is arbitrary. If this section is not present, the default value for the keyboard is no pressed keys. Example: `|Kffe1:20:ff53` 

#### Mouse

Mouse starts with `|M`, followed by mouse inputs in the format: `xpos:ypos:X:12345`. `xpos` and `ypos` are x and y coordinates of the pointer (i32) in decimal strings (can be negative). The reference mode of these coordinates are described by `X`, which is a single character, either `A` (absolute coordinates) or `R` (relative coordinates). This is followed by 5 characters which are either a digit if the corresponding mouse button is pressed, or the character `.`. When decoding the file, the code only checks if the character is a `.` or not. If this section is not present, default value is 0,0 absolute coords and no button pressed. Example: `|M202:264:A:1....`

#### Controllers

Each controller starts with `|C` then the controller number (between 1 and 4). The controller inputs are in the format: `axis_left_x:axis_left_y:axis_right_x:axis_right_y:trigger_left:trigger_right:ABXYbgs()[]udlr`. The first 6 values are the axis values (i16) encoded in decimal strings. Then each character is either a button character or `.`, indicating if the corresponding button was pressed or not. The order of the buttons are:
* `A`, `B`, `X`, `Y` buttons
* back (`b`), guide (`g`) and start (`s`) buttons
* left stick (`(`) and right stick (`)`)
* left shoulder (`[`) and right shoulder (`]`)
* dpad up (`u`), down (`d`), left (`l`) and right (`r`)

If this section is not present, default value is 0 for axes and no button pressed. Example: `|C1-3977:-89:0:0:0:0:A..............`

#### Flags

Flags starts with `|F`, followed by the format `R1234ILUO`. Each character is either a flag character or `.`:
* `R`: reset input
* `1`, `2`, `3`, `4`: controller X was attached
* `I`, `L`, `U`, `O`: controller X was detached

If this section is not present, default value is no flag. Example: `|FR........`

#### Framerate

When variable_framerate is enabled, we need to specify the framerate of each frame. Starts with `|T`, followed by `num:den`. `num` and `den` are the framerate numerator/denominator (u32). If this section is not present, default value is the metadata value. Example: `|T60:1`

### Annotations

The third file is the `annotations.txt` text file, containing movie annotations.

### Editor

The last file is `editor.ini`, containing informations for the input editor. It consists of a list of key/value pairs. The first section `input_names` contains the order and name of single inputs in the editor. The second section `nondraw_frames` stores the list of non-draw frames, so that they are shown in the input editor. Here is an example of such a file:

    [input_names]
    1\input=@Variant(\0\0\0\x7f\0\0\0\fSingleInput\0\0\0\0\0\0\0\xffQ)
    1\name=Left
    2\input=@Variant(\0\0\0\x7f\0\0\0\fSingleInput\0\0\0\0\0\0\0\xffS)
    2\name=Right
    3\input=@Variant(\0\0\0\x7f\0\0\0\fSingleInput\0\0\0\0\0\0\0\xffR)
    3\name=Up
    4\input=@Variant(\0\0\0\x7f\0\0\0\fSingleInput\0\0\0\0\0\0\0\xffT)
    4\name=Down
    5\input=@Variant(\0\0\0\x7f\0\0\0\fSingleInput\0\0\0\0\0\0\0\0x)
    5\name=x
    6\input=@Variant(\0\0\0\x7f\0\0\0\fSingleInput\0\0\0\0\0\0\0\0z)
    6\name=z
    size=6

    [nondraw_frames]
    1\frame=1
    2\frame=3
    3\frame=6
    4\frame=48
    5\frame=60
    6\frame=102
    7\frame=103
    8\frame=105
    size=8
