---
layout: page
title: libTAS options
permalink: /guides/options/
---

## Main Window

### Frames per second

You have to set an FPS value prior to running a game, which appears as two fields being the numerator and denominator of a fraction (e.g. `60` and `1` for 60 FPS). Many games accept any value and rely on the vsync parameter to either have to game running at fixed FPS depending on the screen framerate, or uncapped FPS. In this case, you can set the value you want, and libtas will simulate an activated vsync with the corresponding framerate.

Some games are build with a constant framerate in mind, and setting another FPS value will trigger abnormal results (like duplicate frames). In this case, it is possible to guess the intended game framerate by checking Runtime > Debug > Uncontrolled time. This will deactivate the deterministic timer, meaning that the game will access the real system time. Then, you can run the game and look at the obtained FPS value. This mode is not meant to be used for TASing, because it makes inputs recording non-deterministic.

### System Time

The System Time is the time that is accessed by the game. It can be set before starting the game to have different starting times. Because system time is often used as a seed to a random number generator, setting this value can affect the random behaviors in the game. This is why initial system time is stored in movie files. When the game has started, this value shows the current time that the game has access to.

## File

### Executable Options

When the game is required to be ran from a specific working directory, the `Run path` field must be set to that directory. Also, if the game is bundled with its own shared libraries, you probably want to add that library directory path to `Library path` field. Both fields can be guessed if the game is intended to be launched with a script file. You can look at the contents
of the script.

## Movie

### Annotations

Movies can require specific instructions to run, such as a specific game version, additional libraries, game options, libTAS options, etc. Please provide such instructions inside this field to help other people playing back your movie.

### Autosave

Opens a window to enable or disable autosaving. When enabled, the movie is saved when a certain time has passed, and when the user advanced a certain number of frames. Autosave movies are stored in `~/.libtas/movie/`. Also, old autosave movies are deleted as new ones are created.

### Auto-restart game (since v1.3.3)

When this option is enabled, if the game is closed while recording or playing a movie, then it is automatically restarted and the movie recording/playback is resumed. It works if the game closes by itself, or if the user pressed inputs that result in the game closing (like going to the menu and exit game). This does not work if the user closes the game window, because it is not an input inside the game, but an input of the window manager. In that case, you have to use the restart input, which sends a quit event to the game and makes libTAS restart the game. This input is like any input, it must be mapped in the input mapping configuration.

### Input Editor

Opens a window showing the inputs of the playing movie. The first column shows the slot of the savestate if there is one at that frame. The second column shows the frame number and each subsequent column indicates the value of each single input present in the movie. For binary inputs (keys, mouse buttons, controller buttons), each cell indicates if that input was pressed at the corresponding frame, and it is possible to toggle that input by left-clicking on it. For analog inputs (mouse coordinates, controller axes), each cell indicates the value of the input at the corresponding frame, and it is possible to modify the value by double-clicking on it. It is not possible to edit an input before the current frame (past inputs).

It is possible to change the label of an input, or to add an input column by right-clicking on a column header. Also, right-clicking on a cell provides several options to the user.

When the input editor is opened, the savestates work a bit differently. When in recording mode, if you load a savestate, it will load the entire movie even past the current frame of the savestate.

## Video

### Virtual screen resolution

This option simulates a monitor that only supports the specified resolution, so that games may conform with this setting and display at the specified resolution. This is useful to encode movies at a higher resolution than the monitor resolution. This doesn't work for all games.

### Force software rendering

This option forces the OpenGL device driver Mesa 3D to use its software implementation of OpenGL (llvmpipe) to run the game. While this makes the game much slower, it is usually required to be able to use savestates. Indeed, the state of the GPU cannot be easily accessed and stored into savestates by the tool, thus savestates are incomplete and may crash the game. When using Mesa's software implementation, all of the graphics pipeline is done by the CPU and can be stored in the savestate.

You must be using Mesa 3D to be able to use this option, which usually means that you must be using the free driver for your GPU (e.g. nouveau for nvidia GPUs or radeon for ATI/AMD CPUs)

### Toggle performance tweaks

When software rendering is on, you can check this to skip some steps in the rendering pipeline, which can give a small performance boost.

### On-screen Display (OSD)

Displays some information on the game screen such as framecount, inputs, notifications and ram watches. The placement of these texts can be modified, and can be displayed in the encode video.

### Variable framerate

Allows the user to change the framerate during the execution of the game. This must be checked
before starting the game.

## Runtime

### Time tracking

Some games expect, at some point, the time to advance, and wait in a loop, querying the time constantly until the time has advanced enough. However, we only advance time during a frame boundary, triggered by a screen display call. To avoid this softlock, we can let the time advance after a threshold number of calls from the same time-querying function. This option affects sync, but the game should run deterministically.

### Savestates

#### Incremental savestates

The Incremental savestates option allows taking advantage of the recent soft-dirty bit pushed by the CRIU project, which can track which memory pages have been written to. When the first savestate is triggered, a complete memory dump is performed (like a regular savestate), but the soft-dirty bit is cleared. If another savestate is performed, only the memory pages that were modified since are saved, leading to lightweight savestates. However, to be effective, this option requires the user to make the first savestate as late as possible. The tool detects at startup if this feature is available on the system, and disables the option if not.

#### Store savestates in RAM

This option is good for users that don't have an SSD, but no check is performed if there is enough remaining memory when savestating.

#### Backtrack savestate

This option makes the game save a state each time savestates are invalidated by thread creation or destruction. This state can be loaded with F10 (by default). It allows users to get back to the earliest frame that is available.

#### Compressed savestates

Compressed savestates save time especially for slow hard-drive disks. This option is recommended.

#### Skip unmapped pages

Save savestate space by not saving unmapped memory pages, but some games crash
because of this. If unsure, leave unchecked.

#### Fork to save states

Use the `fork()` feature to create a copy of the game process that will save its
memory, so you can resume the game immediately.

### Prevent writing to disk

This option aims to prevent the game from saving its savefiles on disk. This is useful to keep the same state of the game whenever you load a savestate or you quit and restart the game. To enable that, libTAS detects if the game opens a regular file in write mode, and instead opens a virtual file in memory with a copy of the content of the actual file. The game does not notice it and uses regular file commands (e.g. read, write, seek) on it. Because this virtual file is in memory, it is saved inside savestates and is recovered when loading a savestate. Also, when the game is closed, all modifications to the virtual file are lost. This option may cause some games to crash, if they are doing uncommon operations with savefiles, or if the tool incorrectly detected savefiles.

### Recycle threads

One current limitation of the savestates implementation is that loading a savestate won't recreated threads that have exited since the savestate was done. It makes loading impossible in common cases like between levels, and the user will be forced to restart the entire movie because they cannot load any savestate. We can work around this limitation by recycling threads. When a game thread exits, it turns into a wait mode instead, and the next time the game creates a new thread, no thread is actually created and the thread function is passed to this waiting thread. Thanks to this, savestates are much more likely to be possible.

However, some games will crash when this option is checked (e.g. recent Mono games) because thread-local storage is not completely supported.

### Virtual Steam client

When enabled, it will simulate a dummy Steam client in case games want to connect to Steam. This is mandatory for games that require Steam to be opened, because libTAS does not work with Steam. The implementation of this dummy client is not complete, so it won't work with all games.

### Debug

#### Uncontrolled time

This option makes the game access the real system time. It is useful to see if a softlocked game can run fine with this option. It also gives the user a good idea of the native framerate of the game. Checking this option will cause desyncs in input recording/playback.

#### Native events

This option makes the game access the real event system. It is useful to debug games that don't take inputs from the user. Checking this option may make input recording not work.

## Tools

### Configure Encode

Video encode is configured in this window. The program offers some audio and video codecs as dropdown menus, which fill the corresponding FFmpeg options, but you can manually modify the options to be sent to FFmpeg. Also, the container is determined from the file extension (e.g. naming your video encode.mkv will use the Matroska container).

### Ram Search

There is a rudimentary Ram Search feature implemented. To start a new search, you need to:

- Check the memory regions you want to include. In the most common case, you will want to only include Heap and Anon RW Mapping which are the two regions of dynamically allocated read/write memory.
- Choose Unknown/Previous Value if you don't want to filter the results, or Specific Value if you want to filter base on a comparison to a specific value. In that case, fill the value and the operator above. Be careful that the allocated memory can be very big in some games, so whenever possible, filter the results for a new search. Also, try to filter the value 0 because a non-negligible part of memory contains zeros. If too many results are retained, the program will crash
- Choose the type of variable to search for, and how it will be displayed
- Press New 

Then, for more filtering, set the appropriate parameters and press Search. If you find a result that you want to keep, you can select the row and press Add Watch. You will be able to save the address and apply a label. Then, the saved addresses are available in the Ram Watch menu.

### Ram Watch

A simple window to look at the ram watches. It is also possible to scan for pointers to a specific address. Because most memory is allocated dynamically, the location of a particular value that you are looking for will probably change for every game execution. To be able to keep track of this value, you can search for a chain of pointers that starts from a static address (which never change) to the actual value location. It will look like pointer -> offset -> pointer -> offset -> pointer -> offset -> location.

The pointer scan feature will scan for pointer chains to a specific address. Selecting a ram watch and pressing the button will open a new window. You can then set the maximum number of pointers in the chain and the maximum value of offsets. The Search will take some time, but all the layout is stored so you can search again with different parameters or other addresses in a very short time, as long as you don't frame advance. Then, you can store results in the Ram Watch.

## Input

### Mouse relative mode

Set the mouse position relative to the center of the screen. For exemple, if you move the cursor
3 pixels right of the center of the screen, it will send to the game as if you moved
your cursor 3 pixels to the right from the last frame. This is useful for first-person
controls, together with the next feature.

### Warp mouse to center each frame

This feature mimics what is used by first-person games so that the pointer does
not go past the game screen.

### Prevent games from warping the mouse cursor

If checked, libTAS will prevent games from warping the mouse cursor (but will still register
the warp internally).

### Recalibrate mouse position

Some games move their cursor using the relative movement of the mouse, so there will be an offset between the system mouse cursor and the game cursor. To align both cursors, after choosing this option, click on the game cursor.
