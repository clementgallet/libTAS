---
layout: page
title: FAQ
permalink: /faq/
---


#### I have a Windows system, how can I run libTAS?

If you have Windows 10 or newer, the easiest way is to use [WSL 2](../guides/wsl) (Windows Subsystem for Linux) to run libTAS. Otherwise, you can install a Linux distribution (e.g. Ubuntu) on a virtual machine (e.g. using VirtualBox).

#### Can I TAS Windows games with libTAS?

Short answer: no, unless it has a Linux release. Long answer: there's experimental support for run games through [Wine](https://github.com/clementgallet/libTAS#run-windows-games-through-wine). Very few games were tested, and there are a bunch of issues.

#### Can I TAS Steam games?

If Steam offers a Linux version, possibly yes. Some games from Steam are actually DRM-free, so there are no issues with them. Other games will need you to enable `Virtual Steam client` setting to work. Games heavily relying on Steam features (such as multiplayer games) won't work.

#### My Steam game complains about some missing library

Steam games are often dependent on old libraries from the steam-runtime, which are not available anymore
on recent systems. To fix that, it is possible to launch libTAS with the additional
libraries from steam-runtime using:

    ~/.steam/bin/steam-runtime/run.sh libTAS

#### Why does my game run horribly slow?

Unfortunately, for savestates to work, games must be run entirely on the CPU, without the GPU. It makes games run even more slowly as they contain heavy rendering. If you don't bother about savestates, you can uncheck `Video > Force software rendering`. If you do, several options are to lower the game resolution or disable rendering effects ingame. Also, fast-forward skips rendering.

#### Can libTAS use the real GPU and still support savestates?

Not robustly in the general case yet.

Using a real GPU while preserving savestates is much harder than CPU-only rendering, because a lot of graphics state lives in the driver and on the GPU itself. There is no stable cross-vendor way to snapshot and restore all of that state for arbitrary OpenGL/Vulkan games.

In theory, some approaches exist:

- low-level GPU state capture (very complex, backend and driver specific)
- dual rendering or copy-back schemes (possible, but expensive and hard to keep deterministic)
- API tracing and replay (the most practical direction, but still requires periodic checkpoints and strict synchronization)

So this feature is possible only in constrained or experimental setups today. For now, software rendering remains the most reliable way to guarantee savestate correctness.

#### Can I load a savestate after closing and restarting the game?

Usually no. libTAS savestates are designed to be restored in the same process lifetime.

The main issue is that windowed games depend on external state that is not fully reproducible after restart, especially the connection to the display server (X11/XCB/Wayland). After a restart, the connection, object identifiers, and driver-side graphics resources are different, while the savestate still contains pointers and IDs from the old session.

This is also why general checkpoint/restore projects such as CRIU explicitly do not support restoring many GUI applications.

In practice, restart-restore may only work in constrained setups (for example specific headless or isolated display environments), but it is not a generally robust feature for regular desktop games.

#### When starting a game, it says "Could not determine arch of file <path>"

Check that you specified the binary file for the game. Linux games often provide a script
to launch the game (e.g. `AxiomVerge`), but the actual binary is another file
(e.g. `AxiomVerge.bin.x86_64`).

#### I'm getting an error about `System.TypeInitializationException`

This is a known [Mono bug](https://github.com/mono/mono/issues/6752), it can be
fixed by launching libTAS like this: `term=XTERM libTAS`.

#### I'm getting an error "ERROR: We didn't find any regular TTF font !"

You need to install a TTF font so that the on-screen display works. If you are using a
Debian-based distribution (e.g. Ubuntu), you can execute `sudo apt install fonts-liberation`.

#### I'm getting an error when launching libTAS: "libQt5Core.so.5: cannont open shared object file: No such file or directory"

If you are running libTAS in WSL, it probably means that you have the version 1 installed. You can check by running the following command in a Windows terminal: `wsl --list --verbose`. If it prints `1`, then you must upgrade to version 2.
