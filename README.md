## libTAS

GNU/Linux software to give TAS tools to games. Code orginates from [SuperMeatBoyTaser](https://github.com/DeathlyDeep/SuperMeatBoyTaser). It requires a GNU/Linux system with a recent kernel (at least 3.17 for the `memfd_create` syscall). Supported archs are `x86_64` and `x86` (not much tested).

## Compile

Compiling is done using cmake. From the root directory just type:

    mkdir build
    cd build
    cmake ..
    make

The current mandatory dependancies so far are `libx11-dev`, `libfltk1.3-dev`, `libtar-dev`, `zlib1g-dev`, `libsdl2-dev`

To enable audio and video dumping, you will need `libavcodec-dev`, `libavformat-dev`, `libavutil-dev`, `libswscale-dev`, `libswresample-dev`.

To enable audio playback, you will also need `libswresample-dev`, `libasound2-dev`

To enable HUD on the game screen, you will need `libfreetype6-dev`, `libfontconfig1-dev`

Cmake will detect the presence of these libraries and disable the corresponding features if necessary.
If you want to manually enable/disable a feature, you must add just after the `cmake` command:

- `-DENABLE_DUMPING=ON/OFF`: enable/disable video and audio dumping
- `-DENABLE_SOUND=ON/OFF`: enable/disable audio playback
- `-DENABLE_HUD=ON/OFF`: enable/disable displaying informations on top of the game screen
- `-DENABLE_CUSTOM_MALLOC=ON/OFF`: enable/disable malloc hooks (unused)
- `-DENABLE_FILEIO_HOOKING=ON/OFF`: enable/disable file opening/closing hooks to handle savefiles

Be careful that you must compile your code in the same arch as the game. If you have an amd64 system and you only have access to a i386 game, then you must cross-compile the code to i386. To do that, use the provided toolchain file as followed: `cmake -DCMAKE_TOOLCHAIN_FILE=../32bit.toolchain.cmake ..`

## Run

To run this program, just type:

    ./linTAS [game_executable_path [game_cmdline_arguments]]

You can type `./linTAS -h` to have a description of the program options.

The program prompts a graphical user interface where you can start the game or change several options.

Here are the default controls when the game has started:

- frame advancing, using the `V` key
- pause/play, using the `pause` key
- fast forward, using the `tab` key

Note: the game starts up **paused**.

## Input Format

The tool records inputs into a movie file which has a `.ltm` extension. It is actually a `tar.gz` archive which contains two text files. The first one is `config.prefs`, containing all meta-data of the movie file. It consists of a list of key/value pairs. Here is an example of such a file, with added comments:
```
; FLTK preferences file format 1.0
; vendor: movie
; application: config

[.]

frame_count:159
keyboard_support:1
mouse_support:0
nb_controllers:0 ; up to 4 controllers are supported
initial_time_sec:0 ; number of seconds of the initial system time
initial_time_nsec:0 ; number of nanoseconds of the initial system time
rerecord_count:107
framerate:60 ; number of frames per second
movie_length_sec:2 ; number of seconds of the movie length
movie_length_nsec:650000000 ; number of nanoseconds of the movie length
game_name:FEZ.bin.x86_64 ; name of the game binary
libtas_major_version:1
libtas_minor_version:0

; The following values affect sync. These values are related to a time hack:
; Some game expects the time to advance, and wait in a loop, querying the time
; constantly until the time has advanced enough. However, we only advance time
; during a frame boundary, triggered by a screen display call.
; To avoid this softlock, we let the time advance after a threshold number of
; calls from the same time-querying function. The values below are the
; threshold values for each time-querying function. A value of -1 means we
; never advance time.
; Moreover, we make a difference between the main thread (= rendering thread)
; calling these functions and other threads. While enabling this hack on
; the main thread affects sync, the game still runs deterministically if values
; are unchanged. However, enabling this hack on other threads make the replays
; non-deterministic.
[./mainthread_timetrack]

time:-1
gettimeofday:-1
clock:-1
clock_gettime:100
sdl_getticks:-1
sdl_getperformancecounter:-1

[./secondarythread_timetrack]

time:-1
gettimeofday:-1
clock:-1
clock_gettime:-1
sdl_getticks:-1
sdl_getperformancecounter:-1
```

The second file is the `inputs` text file. In this file, each line that starts with the character `|` is an input frame.
The content of one line depends on the three settings `keyboard_support`, `mouse_support` and `nb_controllers` above.

If `keyboard_support=1`, a list of Xlib KeySym (u32) values of each pressed key is appended to the line. They are encoded into an hex string and separated by the `:` character. The list ends with the `|` character.

If `mouse_support=1`, mouse inputs in the format: `xpos:ypos:12345` are appended to the line. `xpos` and `ypos` are x and y coordinates of the pointer (i32) in decimal strings (can be negative). This is followed by 5 characters which are either a digit if the corresponding mouse button is pressed, or the character `.`. When decoding the file, the code only checks if the character is a `.` or not. The list ends with the `|` character.

For each controller from 1 to `nb_controllers`, the corresponding controller inputs is appended to the line, in the format: `axis_left_x:axis_left_y:axis_right_x:axis_right_y:trigger_left:trigger_right:ABXYbgs()[]udlr`. The first 6 values are the axis values (i16) encoded in decimal strings. Then each character is either a button character or `.`, indicating if the corresponding button was pressed or not. The order of the buttons are:
- `A`, `B`, `X`, `Y` buttons
- back (`b`), guide (`g`) and start (`s`) buttons
- left stick (`(`) and right stick (`)`)
- left shoulder (`[`) and right shoulder (`]`)
- dpad up (`u`), down (`d`), left (`l`) and right (`r`)

Each controller inputs end with the `|` character.

## Troubleshooting

#### ERROR: ld.so: object 'XXX/libTAS.so' from LD_PRELOAD cannot be preloaded (wrong ELF class: ELFCLASS64)

This means that you are trying to run a 32-bit game but you have compiled the project for a 64-bit arch. You must use the toolchain file as described above to compile the project for a 32-bit game

#### libXXX.so not found!

Some Linux games add libraires to the library path before executing the main binary. Because we are lauching the game executable directly, it does not know where to look for the libraries.

To fix this, you need to add two options to set the extra library directory, as well as the path where the game must run to detect those libraries.

The game should include a script that launches the correct game binary as well as include the corresponding libraries. Here is an exemple from VVVVVV:

```
#!/bin/bash
# VVVVVV Shell Script
# Written by Ethan "flibitijibibo" Lee

# Move to script's directory
cd "`dirname "$0"`"

# Get the system architecture
UNAME=`uname`
ARCH=`uname -m`

# Set the right libpath, execute.
if [ "$UNAME" == "Darwin" ]; then
	export DYLD_LIBRARY_PATH=$DYLD_LIBRARY_PATH:./osx/
	./osx/vvvvvv.osx
else
	if [ "$ARCH" == "x86_64" ]; then
		export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:./x86_64/
		./x86_64/vvvvvv.x86_64
	else
		export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:./x86/
		./x86/vvvvvv.x86
	fi
fi
```

In this exemple, for a `x86_64` arch, under `File>Executable Options...`, you must set `Run path` to `/pathtogame` and `Lib path` to `/pathtogame/x86_64`

## License

libTAS is distributed under the terms of the GNU General Public License v3.

    Copyright (C) 2016 clementgallet
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>
