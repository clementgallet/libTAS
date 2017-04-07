## libTAS

GNU/Linux software to give TAS tools to games. Code orginates from [SuperMeatBoyTaser](https://github.com/DeathlyDeep/SuperMeatBoyTaser).

## Compile

Compiling is done using cmake. From the root directory just type:
    mkdir build
    cd build
    cmake ..
    make

The current mandatory dependancies so far are `libx11-dev`.

To enable audio and video dumping, you will need:

- libavcodec
- libavformat
- libavutil
- libswscale
- libswresample

To enable audio playback, you will also need:

- libswresample
- libasound

To enable HUD on the game screen, you will need:

- libfreetype

Cmake will detect the presence of these libraries and disable the corresponding features if necessary.
If you want to manually enable/disable a feature, you must add just after the `cmake` command:

- `-DENABLE_DUMPING=ON/OFF`: enable/disable video and audio dumping
- `-DENABLE_SOUND=ON/OFF`: enable/disable audio playback
- `-DENABLE_HUD=ON/OFF`: enable/disable displaying informations on top of the game screen
- `-DENABLE_CUSTOM_MALLOC=ON/OFF`: enable/disable custom dynamic memory management
- `-DENABLE_FILEIO_HOOKING=ON/OFF`: enable/disable file writing control to disable savefiles

Be careful that you must compile your code in the same arch as the game. If you have an amd64 system and you only have access to a i386 game, then you must cross-compile the code to i386. To do that, use the provided toolchain file as followed: `cmake -DCMAKE_TOOLCHAIN_FILE=../32bit.toolchain.cmake ..`

## Run

To run this program, just type:

    ./linTAS game_executable_path [game_cmdline_arguments]

You can type `./linTAS -h` to have a description of the program options.

The program prompts a text-based user interface where you can start the game or change several options.

Here are the default controls when the game has started:

- frame advancing, using the `V` key
- pause/play, using the `pause` key
- fast forward, using the `tab` key
- record and playback inputs
- dump the audio/video

Note: the game starts up **paused**.

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

In this exemple, for a `x86_64` arch, you must run:

    linTAS -L /pathtogame/x86_64 -R /pathtogame /pathtogame/x86_64/vvvvvv.x86_64


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


