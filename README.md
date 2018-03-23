## libTAS

GNU/Linux software to give TAS tools to games. Code orginates from [SuperMeatBoyTaser](https://github.com/DeathlyDeep/SuperMeatBoyTaser). It requires a GNU/Linux system with a recent kernel (at least 3.17 for the `memfd_create` syscall). Supported archs are `x86_64` and `x86` (not much tested). To run OpenGL games, you will need a card supporting at least OpenGL 3.0.

## Install

You can download the latest version of the software in the [Releases](https://github.com/clementgallet/libTAS/releases) page. The current dependancies are:

* `libc6`, `libgcc1`, `libstdc++6`
* `libqt5core5a`, `libqt5gui5`, `libqt5widgets5` with Qt version at least 5.6
* `libx11-6`, `libxcb1`, `libxcb-keysyms1`
* `libavcodec57`, `libavformat57`, `libavutil55`, `libswscale4`
* `libswresample2`, `libasound2`
* `libfontconfig1`, `libfreetype6`
* `libtar0`, `zlib1g`

Installing with the debian package will install all the required packages as well. The ffmpeg API used in the software is quite recent (> 3.2) so it is possible you will need to update your ffmpeg libraries.

If you don't have a Linux system beforehand, an easy way is to use a virtual machine to run the system. Grab a virtualization software (e.g. VirtualBox) and a Linux distribution (e.g. Ubuntu) whose architecture matches a supported architecture of the game you want to run (in most cases amd64, sometimes only i386 is available). Note for Ubuntu users that you need a recent version (17.10 minimum).

## Compile

If you want to compile it yourself, you will need `cmake`. From the root directory just type:

    mkdir build
    cd build
    cmake ..
    make

The current mandatory dependancies so far are `libx11-dev`, `qtbase5-dev`, `libtar-dev`, `zlib1g-dev`, `libsdl2-dev`, `extra-cmake-modules`, `libxcb1-dev`, `libxcb-keysyms1-dev`, `libasound2-dev`

To enable audio and video dumping, you will need `libavcodec-dev`, `libavformat-dev`, `libavutil-dev`, `libswscale-dev`, `libswresample-dev`.

To enable audio playback, you will need `libswresample-dev`

To enable HUD on the game screen, you will need `libfreetype6-dev`, `libfontconfig1-dev`

Cmake will detect the presence of these libraries and disable the corresponding features if necessary.
If you want to manually enable/disable a feature, you must add just after the `cmake` command:

- `-DENABLE_DUMPING=ON/OFF`: enable/disable video and audio dumping
- `-DENABLE_SOUND=ON/OFF`: enable/disable audio playback
- `-DENABLE_HUD=ON/OFF`: enable/disable displaying informations on top of the game screen
- `-DENABLE_FILEIO_HOOKING=ON/OFF`: enable/disable file opening/closing hooks to handle savefiles

Be careful that you must compile your code in the same arch as the game. If you have an amd64 system and you only have access to a i386 game, the easiest way is to build a virtual machine with a i386 system. You could also try to cross-compile the code to i386. To do that, use the provided toolchain file as followed: `cmake -DCMAKE_TOOLCHAIN_FILE=../32bit.toolchain.cmake ..`. However, many users failed to do this due to some libraries that don't like this operation.

## Run

To run this program, just type:

    ./linTAS [game_executable_path [game_cmdline_arguments]]

You can type `./linTAS -h` to have a description of the program options.

The program prompts a graphical user interface where you can start the game or change several options. Details of the different options are available [here](https://github.com/clementgallet/libTAS/wiki/Menu-Options)

There are a few things to take care before being able to run a game. You might want to look at the software [usage](https://github.com/clementgallet/libTAS/wiki/Usage).

Here are the default controls when the game has started:

- frame advancing, using the `V` key
- pause/play, using the `pause` key
- fast forward, using the `tab` key

Note: the game starts up **paused**.

## License

libTAS is distributed under the terms of the GNU General Public License v3.

    Copyright (C) 2016-2018 clementgallet
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
