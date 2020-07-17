## libTAS

GNU/Linux software to give TAS tools to games. Code orginates from [SuperMeatBoyTaser](https://github.com/DeathlyDeep/SuperMeatBoyTaser). It requires a GNU/Linux system with a recent kernel (at least 3.17 for the `memfd_create` syscall). Supported archs are `x86_64` and `x86`. You will need the Mesa llvm OpenGL driver to support savestates for games using OpenGL.

Discord server: https://discord.gg/3MBVAzU

## Supported Games

Initial work was done to support games using the SDL library (which is the case of many indie games), but other engines are supported now.

By default, you should look for a drm-free version of the game. Games installed through Steam may or may not work. Some Steam games are in fact drm-free, they work without Steam running, or they just need some simple operations. For the other games, there is a "Virtual Steam client" setting in libTAS which implements a dummy Steam client, but it doesn't support every game. Of course, all of this only applies to games that don't have any active protection measure. For more details, check the [Game Compatibility](http://tasvideos.org/EmulatorResources/libTAS/GameCompatibility.html) page.

There is also initial support for Windows games through wine. Details are given below.

## Non-linux users

If you don't have a Linux system beforehand, there are several options that you can choose:
- Run libTAS using WSL 2 (Windows Subsystem for Linux) (preferred option, described below)
- Install a Linux container with Docker (described below)
- Install Linux on a virtual machine. Grab a virtualization software (e.g. VirtualBox) and a Linux distribution (e.g. Ubuntu). If you have a 64-bit computer, install a 64-bit Linux distribution, which will allow you to run both 32-bit and 64-bit games. Note for Ubuntu users that you need a recent version (17.10 minimum).

## Install

You can download the latest stable version of the software in the [Releases](https://github.com/clementgallet/libTAS/releases) page. The current dependencies are:

* `libc6`, `libgcc1`, `libstdc++6`
* `libqt5core5a`, `libqt5gui5`, `libqt5widgets5` with Qt version at least 5.6
* `libx11-6`, `libxcb1`, `libxcb-keysyms1`, `libxcb-xkb1`, `libxcb-cursor0`
* `ffmpeg`
* `libswresample2` or `libswresample3`, `libasound2`
* `libfontconfig1`, `libfreetype6`
* `liblz4-1`

Installing with the debian package will install all the required packages as well.

There is also an [automated build](https://ci.appveyor.com/project/clementgallet/libtas/build/artifacts) available for the last interim version, 64-bit with dual-arch.

## Building

An PKGBUILD is available for Arch Linux on the [AUR](https://aur.archlinux.org/packages/libtas/).

### Dependencies

You will need to download and install the following to build libTAS:

* Deb: `apt-get install build-essential automake pkg-config libx11-dev libx11-xcb-dev qtbase5-dev qt5-default libsdl2-dev libxcb1-dev libxcb-keysyms1-dev libxcb-xkb-dev libxcb-cursor-dev libxcb-randr0-dev libudev-dev libasound2-dev libavutil-dev libswresample-dev ffmpeg liblz4-dev`
* Arch: `pacman -S base-devel automake pkgconf qt5-base xcb-util-cursor alsa-lib ffmpeg lz4 sdl2`

To enable HUD on the game screen, you will also need:

* Deb: `apt-get install libfreetype6-dev libfontconfig1-dev`
* Arch: `pacman -S fontconfig freetype2`

### Cloning

    git clone https://github.com/clementgallet/libTAS.git
    cd libTAS

### Building

    ./build.sh

autoconf will detect the presence of these libraries and disable the corresponding features if necessary.
If you want to manually enable/disable a feature, you must add at the end of the `build.sh` command:

- `--disable-hud`: enable/disable displaying informations on the game screen

Be careful that you must compile your code in the same arch as the game. If you have an amd64 system and you only have access to a i386 game, you can cross-compile the code to i386 (see below).

### Install

    sudo make install

### Dual-arch

If you have an amd64 system and you want to run both i386 or amd64 games, you can install libTAS 32-bit library together with the amd64 build. To do that, you can build using `./build.sh --with-i386`, which will produce an additional `libtas32.so` (32-bit) library together with amd64 builds of `libTAS` GUI and `libtas.so` library.

You will need the 32-bit version of libraries `libX11`, `libX11-xcb`, `libasound`, `libavutil`, `libswresample`, `liblz4-1`, the 32-bit version of the headers for `libavutil`, `libswresample`, and also librairies `libfreetype` and `libfontconfig` for the HUD feature. You will also need your compiler to be able to cross-compile, by installing `g++-multilib` if using g++ compiler. When running a game, libTAS will choose automatically the right `libtas.so` library based on the game arch.

## Run

To run this program, you can use the program shortcut in your system menu, or open a terminal and enter:

    libTAS [game_executable_path [game_cmdline_arguments]]

You can type `libTAS -h` to have a description of the program options.

The program prompts a graphical user interface where you can start the game or change several options. Details of the different options are available [here](http://tasvideos.org/EmulatorResources/libTAS/MenuOptions.html)

There are a few things to take care before being able to run a game. You might want to look at the software [usage](http://tasvideos.org/EmulatorResources/libTAS/Usage.html).

Here are the default controls when the game has started:

- frame advancing, using the `V` key
- pause/play, using the `pause` key
- fast forward, using the `tab` key

Note: the game starts up **paused**.

## Run Windows games through wine

This is still experimental. To launch Windows games through wine, you first need to install wine on your system (it must be located somewhere in $PATH).

To get audio correctly handled, you need to open winetricks, then select "Select the default wineprefix", "Change settings" and check "sound=alsa".

In the game executable field, you must set the Windows `.exe` executable (both 32-bit and 64-bit are supported, given that you have the wine version for it). It will detect a Windows executable and launch wine with the right options.

There are still a lot of issues to fix:
- "Prevent writing to disk" feature does not work, because wine delegating to another process (wine server) the file handling (#250)
- Window focus/unfocus doesn't work sometimes and you cannot move the game window (#262). Try alt-tabbing until you get it back
- "Virtual Steam client" does not work, because we don't hook the loading of the `steam_api.dll` file (#264)
- Window repainting (Expose event) does not work, so if you pause the game and drag another window in front of it, it will overwrite what is displayed by the game

## Run on WSL 2

### Install

You need at least Windows 10 version 2004 to run WSL 2. libTAS does not work with WSL 1, because it is missing some mandatory components of the Linux system. Activate WSL 2 and install Ubuntu from the Windows Store, you can find plenty of documentation for that.

Once installed, you have a new application "Ubuntu" which opens a terminal. Download the latest "libtas_*_amd64.deb" file from the Releases. To locate the file from the Ubuntu terminal, you can access to Windows disk drives in  "/mnt/" ("/mnt/c" for the C: drive, etc.). Go to the directory which contains the downloaded file, and install the package using `sudo dpkg -i libtas_*_amd64.deb`.

By default, there is no display configured. You need to export the display using the following command:

```
export DISPLAY=$(awk '/nameserver / {print $2; exit}' /etc/resolv.conf 2>/dev/null):0
```

You can paste this command in the file `~/.bashrc` so that it will run each time you start Ubuntu.

We also need a X server to be able to display Linux windows. WSL 2 is supposed to not need an additional program for that, but I couldn't find how to do it. So you need [vcxsrv](https://sourceforge.net/projects/vcxsrv/). Once it is installed, we need to let it connect to WSL, and it was blocked by default by my Windows Defender firewall. You need to open the firewall configuration window, and authorize the program "vcxsrv" to communicate through the firewall (both private and public networks).

### Run

Launch the "XLaunch" program, which will launch the configuration for the X server. You need to change the "Display number" to 0 on the first screen, uncheck "Native opengl" and check "Disable access control" on the third screen.

From the Ubuntu terminal, enter `libTAS`. You should see the window showing.

## Run on Windows using Docker

One option if you have a Windows system is to run a Linux distribution inside a Docker container. This has the advantage of automating package installation and libTAS compilation. On Windows there are currently two possibilities for using Docker: [Docker for Windows](https://docs.docker.com/docker-for-windows) and [Docker Toolbox](https://docs.docker.com/toolbox/toolbox_install_windows/). There are some requirements for the former (Windows 10 64-bit Pro, Enterprise, or Education), so I will describe the installation process for the later (both should be really close, but I can't test the former).

### Install

- install [Docker Toolbox](https://docs.docker.com/toolbox/toolbox_install_windows/)
- download the [Dockerfile](https://github.com/clementgallet/libTAS/blob/master/Dockerfile) somewhere (don't rename the file!).
- open Docker Quickstart Terminal, and run the command `docker build -t libtas /c/path/to/Dockerfile/directory`. The path points to the directory containing the Dockerfile, not to the Dockerfile itself. Also, this terminal takes paths in Unix syntax: `C:\foo\bar` becomes `/c/foo/bar`. The Docker container build should start.
- prepare a directory that will contain your games. This directory must be somewhere inside `C:/Users/`, because it is shared by default by Virtualbox. If you want to share another folder, you can follow this [link](http://support.divio.com/en/articles/646695-how-to-use-a-directory-outside-c-users-with-docker-toolbox-docker-for-windows)
- you need a X11 server to be able to display applications from the Linux container. Install [vcxsrv](https://sourceforge.net/projects/vcxsrv/)

### Run

- launch Xlaunch, and check "Disable access control" in the Extra settings
- for the Linux container to find your X11 server, you need your ip address. Type `ipconfig` in the Docker terminal, and search for ipv4 field.
- to launch the container, run:

```
docker run -ti --rm -e DISPLAY="<ip>:0.0" -v /c/Users/path/to/shared/folder:/root/games libtas
```

You need to use the Unix syntax again for your shared folder path. Relative paths don't work here.

This should open a terminal in your container. You can then enter `libTAS` to launch the program. If everything goes right, you should see the libtas window appear. If it complains about not been able to open the display, check again your ipv4 field. You can also access to your shared files located inside `/root/games` path. Because of how Docker works, everything that is modified outside of `/root/games` will not be preserved on later instances of the container, so be sure to install and save everything inside that directory.

TODO: Figure out how to use Mesa OpenGL.

## Logo

Made by brunovalads.

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
