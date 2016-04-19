## libTAS

GNU/Linux software to (hopefully) give TAS tools to games. Code orginates from [SuperMeatBoyTaser](https://github.com/DeathlyDeep/SuperMeatBoyTaser).

## Compile

Compiling is done using cmake. From the root directory just type `cd build && cmake ..`, then `make`.

The current mandatory dependancies so far are `libx11-dev`.

To enable audio and video dumping, you will need:

- libavcodec
- libavformat
- libavutil
- libswscale
- libswrescale

To enable audio playback, you will also need:

- libswrescale
- libasound

To enable HUD on top of the game screen (currently not working, disabled by default), you will need:

- libfreetype

Cmake will detect the presence of these libraries and disable the corresponding features if necessary.
If you want to manually disable a feature, you must add just after the `cmake` command either `-DENABLE_DUMPING=OFF`, `-DENABLE_SOUND=OFF` or `-DENABLE_HUD=OFF`.

Be careful that you must compile your code in the same arch as the game. If you have an amd64 system and you only have access to a i386 game, then you must cross-compile the code to i386. To do that, use the provided toolchain file as followed: `cmake -DCMAKE_TOOLCHAIN_FILE=32bit.toolchain.cmake ..`

## Run

To run this program, just type:

    ./run.sh [options] game_executable_path [game_commandline_arguments]

You can type `./run.sh -h` to have a description of the program options.

For now, what you can do is:

- frame advancing, using the `V` key
- pause/play, using the `pause` key
- fast forward, using the `tab` key
- record and playback inputs
- dump the audio/video

Note: the game starts up **paused**.

## Licence

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


