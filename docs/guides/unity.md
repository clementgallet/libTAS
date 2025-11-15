---
layout: page
title: Unity games
permalink: /guides/unity/
---

Unity engine being one of the most used game engine, and having Linux support,
a lot of work has been performed to support Unity games in libTAS. Running 
games in libTAS has been possible for a long time, but movies desynced very 
easily.

Here is an overview of what is required to run Unity games in libTAS.

## Porting

If the game does not have a Linux version, you may be able to port from a 
Windows or (even better) Mac version. ikuyo has made a nice [documentation](https://apps.microsoft.com/detail/9pdxgncfsczv)
about it.

There is also the [Unify](https://github.com/0xf4b1/unify) project to
automatically port games.

## Running

Running Unity games in libTAS does not require specific options. For determinism,
however, you will need to add `-force-gfx-direct` to the command-line options
field. libTAS tries to automatically detect Unity games and add this option, 
but it may failed, so it's better to add it manually.

For recent games that will default to using Vulkan, you may want to 
enforce OpenGL which is more stable, by adding `-force-opengl` to command-line
options as well.


## Determinism

### Debug version

Unity engine has been made more and more optimized by heavily relying on 
multi-threading. This made games highly non-deterministic sadly. To overcome this,
libTAS hooks a bunch of internal Unity functions that handle the job system, the
asynchronous reads and preloading operations. The goal is to make all those 
systems sequential: they still rely on threads, but their execution is performed
sequentially in a defined order.

To be able to hook internal functions, the main hooking mechanism is helpless. 
We need to rely on patching code functions, meaning that we need to know the 
location of those functions in the game memory. To do that, we rely on a nice
offer from Unity: they distribute a version of each Unity runtime with debug 
symbols! By replacing the runtime program with the version with symbols, we can
still run the game and get the location of the functions we want.

So, the procedure is the following:
* Obtain the runtime version: this is performed by libTAS when running the game 
  (look at what is displayed on the terminal), if it detected the game as using
  Unity engine. Alternatively, you can run this command: `strings /path/to/game/data/Resources/unity_builtin_extra | head -n 1`
* Download the Linux runtime for this version. You can find those on the 
  following sources depending on the version:
    - [versions 5.3.0+]: the [Official Unity archive](https://unity.com/fr/releases/editor/archive) 
      contains what you want. Download the file `̀Component installers` > 
      `macOS` > `Linux Build Support`, which can be extracted on Linux.
      For recent Unity version, you will need to pick the correct one between
      `Linux Build Support (IL2CPP)` and `Linux Build Support (Mono)`. You can 
      see easily if your game is compiled with IL2CPP, by looking if there is a
      `il2cpp` folder inside the game `Data` directory.
    - [versions 5.1.0 - 2018]: This [source archive](https://discussions.unity.com/t/unity-on-linux-release-notes-and-known-issues/595006)
    - [versions 4]: This [source archive](https://discussions.unity.com/t/early-unity-versions-downloads/927331)
* Extract the archive and grab the runtime, which should be located inside
  `Data/PlaybackEngines/linuxstandalonesupport/Variations/`. You will be interested in
  `linux[32|64]_[withgfx|player]_development_[mono|il2cpp]` directory.
* For newer Unity versions (~2019+), copy file `UnityPlayer_s.debug` to the game
  directory. This will be used by libTAS to find symbols (**do not rename**). For
  old Unity versions, copy `LinuxPlayer` executable to the game directory and
  use it as your new game executable. You will need to rename this file like the
  original executable (minus the extension which does not matter), for the game
  to work correctly.

If all goes well, libTAS should find the function addresses it wants, and print
each one on the terminal at the game startup.
  
If the Unity version is not available, e.g. the devs are using a custom build 
(hi Silksong), there is still an alternative to locate functions, using function
signatures. One function signature will only be working on a small set of Unity
versions, so these need to be found and added manually.

### Features

Options to enable or disable deterministic behaviour for Unity features are 
located inside `Settings` > `Game-specific`. They must be enabled before the 
game is launched to be effective.

#### Jobs

[Unity jobs](https://docs.unity3d.com/6000.0/Documentation/Manual/job-system-overview.html) 
are small tasks that are designed to be run in parallel, with possible
dependencies. To enforce determinism, when one job is scheduled, it immediately 
waits for the job to complete. When multiple independent jobs are scheduled (in 
a `parallel for` design), we schedule each job in order and wait for one job to
finish before scheduling the next.

#### Preload operations

When tasks can last more than one frame, Unity uses a Preloader that performs 
async operations (scene loading, assets loading, etc.). The lifetime of an async
operation is roughly:

* the (usually main) thread pushes the operation into the preloader queue
* the preloader thread pulls the operation with highest priority, and calls `op->Perform()`
* once per frame, the main thread calls `op->IntegrateTimeSliced()`
* if the operation is completed (`op->IsDone()`), it calls `op->IntegrateMainThread()`
  and optionally `op->InvokeCoroutine()`. Then the operation is pulled

To enforce determinism, each time an operation is pushed, we wait for the preloader 
thread to finish calling `op->Perform()`.

#### Async reads

Some files are read inside another thread, to not block the main thread.
To enforce determinism, we swap it with the sync variation of file read, that
exists in every version of Unity

### Visualization

Jobs and preload operations can be seen from the OSD in `Debug` > `Unity`.
They can be seen also in the profiler (in `Debug` > `Profiler`).
