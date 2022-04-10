---
layout: home
---

libTAS is a software for Linux that provides TAS tools to games, such as
frame advance, inputs recording, savestates. It is not a Linux emulator as
games are running natively on the user system, but it creates an intermediate
layer between the game and the operating system, feeding the game with
altered data (such as inputs, system time). We can call such tool a
**trans-layer** - an API translation layer. It tries to make the game running
deterministically, although it is still an issue when dealing with
multithreaded games. This layer connects to an external program to provide
a graphical interface to the user, with tools such as input editor or RAM
watch/search.

# Download

* [Stable v1.4.3 version, deb package](https://github.com/clementgallet/libTAS/releases/tag/v1.4.3)
* [Latest build, deb package (64-bit only)](https://ci.appveyor.com/project/clementgallet/libtas/build/artifacts)



 
