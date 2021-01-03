---
layout: page
title: FAQ
permalink: /faq/
---


#### I have a Windows system, how can I run libTAS?

If you have Windows 10, the easiest way is to use [WSL 2](/guides/wsl) (Windows Subsystem for Linux) to run libTAS. Otherwise, you can install a Linux distribution (e.g. Ubuntu) on a virtual machine (e.g. using VirtualBox).

#### Can I TAS Windows games with libTAS?

Short answer: no, unless it has a Linux release. Long answer: there's experimental support for run games through [Wine](https://github.com/clementgallet/libTAS#run-windows-games-through-wine). Very few games were tested, and there are a bunch of issues.

#### Can I TAS Steam games?

If Steam offers a Linux version, possibly yes. Some games from Steam are actually drm-free, so there are no issues with them. Other games will need you to enable `Virtual Steam client` setting to work. Games heavily relying on Steam features (such as multiplayer games) won't work.

#### My Steam game complains about some missing library

Steam games are often dependent on old libraries from the steam-runtime, which are not available anymore
on recent systems. To fix that, it is possible to launch libTAS with the additional
libraries from steam-runtime using:

    ~/.steam/bin/steam-runtime/run.sh

#### Why does my game run horribly slow?

Unfortunately, for savestates to work, games must be run entire on the CPU, without the GPU. It makes games run even more slowly as they contain heavy rendering. If you don't bother about savestates, you can uncheck `Video > Force software rendering`. If you do, several options are to lower the game resolution or disable rendering effects ingame. Also, fast-forward skips rendering.
