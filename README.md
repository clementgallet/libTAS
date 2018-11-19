## libTAS

GNU/Linux software to give TAS tools to games. Code orginates from [SuperMeatBoyTaser](https://github.com/DeathlyDeep/SuperMeatBoyTaser). It requires a GNU/Linux system with a recent kernel (at least 3.17 for the `memfd_create` syscall). Supported archs are `x86_64` and `x86` (not much tested). To run OpenGL games, you will need a card supporting at least OpenGL 3.0.

## Supported Games

Most work has been done to support games using the SDL library (which is the case of many indie games), and there is initial developpement to support other game engines (some Unity games should work). Also, running Steam games require to enable the dummy Steam client, which is only partially implemented. If possible, use a drm-free version of the game. For more details, check the [Game Compatibility](http://tasvideos.org/EmulatorResources/libTAS/GameCompatibility.html) page.

## Install

You can download the latest version of the software in the [Releases](https://github.com/clementgallet/libTAS/releases) page. The current dependancies are:

* `libc6`, `libgcc1`, `libstdc++6`
* `libqt5core5a`, `libqt5gui5`, `libqt5widgets5` with Qt version at least 5.6
* `libx11-6`, `libxcb1`, `libxcb-keysyms1`, `libxcb-xkb1`, `libxcb-cursor0`
* `ffmpeg`
* `libswresample2`, `libasound2`
* `libfontconfig1`, `libfreetype6`

Installing with the debian package will install all the required packages as well.

If you don't have a Linux system beforehand, an easy way is to use a virtual machine to run the system. Grab a virtualization software (e.g. VirtualBox) and a Linux distribution (e.g. Ubuntu). If you have a 64-bit computer, install a 64-bit Linux distribution, which will allow you to run both 32-bit and 64-bit games (using the corresponding version of libTAS). Note for Ubuntu users that you need a recent version (17.10 minimum).

## Building

An PKGBUILD is available for Arch Linux on the [AUR](https://aur.archlinux.org/packages/libtas/).

### Dependancies

You will need to download and install the following to build libTAS:

* Deb: `apt-get install build-essential cmake extra-cmake-modules libx11-dev qtbase5-dev qt5-default libsdl2-dev libxcb1-dev libxcb-keysyms1-dev libxcb-xkb-dev libxcb-cursor-dev libasound2-dev libswresample-dev ffmpeg`
* Arch: `pacman -S base-devel cmake extra-cmake-modules qt5-base xcb-util-cursor alsa-lib ffmpeg`

To enable HUD on the game screen, you will also need:

* Deb: `apt-get install libfreetype6-dev libfontconfig1-dev`
* Arch: `pacman -S fontconfig freetype2`

### Cloning

    git clone https://github.com/clementgallet/libTAS.git
    cd libTAS

### Building

    mkdir build && cd build
    cmake ..
    make

Cmake will detect the presence of these libraries and disable the corresponding features if necessary.
If you want to manually enable/disable a feature, you must add just after the `cmake` command:

- `-DENABLE_HUD=ON/OFF`: enable/disable displaying informations on the game screen

Be careful that you must compile your code in the same arch as the game. If you have an amd64 system and you only have access to a i386 game, the easiest way is to build a virtual machine with a i386 system. You could also try to cross-compile the code to i386. To do that, use the provided toolchain file as followed: `cmake -DCMAKE_TOOLCHAIN_FILE=../32bit.toolchain.cmake ..`. However, many users failed to do this due to some libraries that don't like this operation.

## Run

To run this program, just type:

    libTAS [game_executable_path [game_cmdline_arguments]]

You can type `libTAS -h` to have a description of the program options.

The program prompts a graphical user interface where you can start the game or change several options. Details of the different options are available [here](http://tasvideos.org/EmulatorResources/libTAS/MenuOptions.html)

There are a few things to take care before being able to run a game. You might want to look at the software [usage](http://tasvideos.org/EmulatorResources/libTAS/Usage.html).

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
