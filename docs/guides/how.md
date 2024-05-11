---
layout: page
title: How does it work
permalink: /guides/how/
---

This page describes the technical details of the implementation, issues and limitations of this tool.

* TOC
{:toc}

## Structure

TODO

## Hooking

The method used to inject code into the game is the `LD_PRELOAD` trick. When launching
a program, the dynamic linker loads the required libraries into the address space,
then it resolves symbols. When multiple libraries offer the same symbol, it chooses 
the one that was loaded first. So by preloading the libtas library, their symbols
will be chosen. This simple method allows us to intercept all functions that are
loaded on startup, and we have access to the original functions by using
`dl_sym()` with the `RTLD_NEXT` special value to get the next occurrence of a
symbol in the loading order.

We also hook `dlopen()` and `dlsym()` to intercept functions that are dynamically
loaded.

## Frames

The concept of frame is inherent to TASing, because we need to split the game
execution into an enumerable number of time steps to be able to feed inputs.
For game engines, the definition of frames is well defined because there
is usually an engine loop consisting of gathering inputs, running one engine step, 
and rendering.

Because libTAS is game-agnostic, we need to define when the current frame has 
ended, and the most convenient way was when a rendering occurs.

So we hook every function that draws on the game window, on both high-level 
(e.g. `SDL_GL_SwapWindow()`, `SDL_RenderPresent()`) and low-level APIs (e.g. 
`glXSwapBuffers()`, `vkQueuePresentKHR()`). There, we perform all the code that
needs to run every frame, and communication with the libTAS program. This includes:

* Advancing time and frame count
* Processing and mixing audio sources
* Saving the game window pixels for HUD display, encoding, and for restoring the
  game window after a state load
* Getting and processing inputs
* Optionally wait to sync between multiple threads

### Limitations

This definition of a frame has several limitations:

#### Non-draw frames

Some games advance time by sleeping. When the amount of sleep reaches the 
length of a frame, we need to trigger the end of the current frame, so that 
we can keep up with the framecount. This is a special frame called a non-draw
frame. These frames can be seen in the input editor in red. They are similar
to lag frames known in consoles (where the computation of one frame takes more
than one frame), but a still different in several ways:

* non-draw frames can still read inputs
* non-draw frames can be caused by a variety of reasons:
    - incorrect framerate
    - correct framerate, but games doing strict comparisons, or because of 
      floating-point imprecision
    - time-tracking feature
    - games managing refresh rate manually, for example when playing back a 30-fps
      video in a 60-fps game
    - certain wait settings with games that wait for threads to finish
* non-draw frames may inconsistent. They may be consistent within one system, but
  inconsistent across different systems due to the environment (e.g. libc version, 
  GPU driver)

#### No one-to-one equivalence

Sometime there is no one-to-one equivalence between one frame as define by the 
game and one draw call.

Some modern engines make a distinction between physics frames and visual frames.
While the physics runs at a fixed framerate (e.g. 50 fps for some Unity games), 
they can render at any framerate depending on the monitor refresh rate and the
vsync option. This is not really an issue for libTAS, but it is usually good to
know when designing a TAS for such games.

The much more problematic case is when games refresh multiple times the game 
window during a single frame, usually when they refresh only some portions of the
screen that were "dirty". Also, they can refresh the game window extra times 
due to external factors like moving the mouse pointer.

All these cases are very common for games that are encapsulated into GUI 
toolkits (e.g. GTK, Qt, ), that are designed to build general-purpose graphical
applications. As such, the general structure is very different from games, which
makes them incompatible with libTAS. Issues not limited to:

* Messy rendering
* Multiple nested windows, which leads to difficulties to differentiate the
  in-game window from the rest, as well as inputs and focus issues
* Highly multithreaded

We mostly see the emulators falling in this kind of category, but thankfully, 
many of them have a no-GUI version that use a simple window (e.g. using SDL).

Emulators with GUI versions only

* Adobe Flash Player: GTK

Emulators with both GUI and no-GUI versions:

* Yuzu (Switch): Qt and SDL2
* PPSSPP (PSP): Qt and SDL2
* Ryujinx (Switch): GTK3 and SDL2
* Citra (3DS): Qt and SDL2

## Communication

TODO

## Determinism

### Uninitialized memory

TODO

### Threading

TODO

### External factors

TODO

cputime, urandom, pid, opengl extensions

## Time

### Timer

In an uncontrolled environment on a PC, operations take a variable amount of time,
and games often query for the system time for various reasons. We need a manage
ourselves how time is advancing, and returns that time whenever the game queries
for the current time. For this reason we need to manage a deterministic timer.

For deterministic replay, we need to advance time only at specific cases:

#### Frame boundary

The main operation that increases the deterministic timer is when reaching the
end of a frame. At the beginning of the frame boundary, we increase the deterministic
timer by the exact value of the frame length (inverse of the framerate set by
the user). Moreover, if the timer was increased during the current frame by
another mean, the timer is only increases by the difference, so that we always
have: `current_length = framecount * frame_length`. This way of increasing the
deterministic timer also matches the behaviour of games that are rendering with
vsync on. The vsync feature would pause the game when refreshing the screen so
that it matches the refresh rate of the screen.

When, for some reason, the deterministic timer was increased by more than a
frame length before the end of frame was reached, then the current frame is
ended immediately, resulting in a non-draw frame (see Frames).

#### Sleep

Some games are not handling time that using the vsync feature, but through
manually sleeping to pause the game. In that case, we must support sleep calls
(e.g. `usleep()`, `nanosleep()`) and advance the deterministic timer by the
amount. Otherwise, some games may softlock because they expect time to advance
after performing a sleep call. However, we only want to handle sleep calls from
the thread that is in charge of the game engine, and not sleep calls that are
commonly used by other threads to wait for something. So there are currently 3
settings available to handle sleep calls:

* Never advance time: more conservative but can softlock
* Advance time on main thread: default, more accurate but may cause extra non-draw
  frames and more rarely desyncs
* Always advance time: will desync

#### Wait

Some modern game engines will not use sleep calls to handle time, because they
cannot be interrupted easily. For engines that use threads extensively, they
prefer to sleep using wait calls with a timeout (e.g. `pthread_cond_timedwait()`,
`poll()`, `select()`). Those functions are used for a lot of other cases, so it
is hard to know when they are used for time handling. By default, wait calls
are ignored, but for specific games, there are other options that can advance
time. Whatever option is used, only calls from the main thread may advance time.

#### Time-tracking

For some games, no sleep or wait functions are used, but instead they use some
sort of spin-wait. To advance time, some game engines will continuously query 
for the current time (e.g. `clock_gettime()`, `gettimeofday()`) and do some
yielding in between. This is the main reason of softlocking in libtas, because
it does not fall in the cases above and are hard to identify.

The current solution is to count the number of calls of a 
time-querying function within a single frame, and advance the deterministic
timer when the count reached a threshold. While it fixes most softlocks, it still
raises several issues:

* several parameters are arbitrary: the threshold number, the time increment
* a low threshold may trigger extra non-draw frames, a high number may
  significantly slow the execution speed
* games may desync with this option, when for example the main thread spin-wait
  while waiting for another thread to finish, which causes a variable number
  of time-querying calls

### Time types

There are two classes of time that is queried from the game:

* realtime: a value that represents the actual time since a set value (usually
    Unix epoch of 1970-01-01). The conversion between this value and the current
    time is sensible to timezone configuration, and can move backward due to
    daylight saving time. This time can be set by the user to any value before
    the game is executed (and even during game execution), so that's why libTAS
    offers users to modify the initial value as well as changing the value during
    execution. libTAS enforces a default timezone of UTC.
    
* monotonic time: a value that represents increasing time from an arbitrary value
    (e.g. beginning of the game execution). This value is guaranteed to always
    increase, so it cannot be modified by the user. libTAS still offers to
    change the starting value, as it may be used by games for seeding their 
    pseudo-random generator.

The deterministic timer can return both time types, it just keeps internally the
difference between the two, which changes only when realtime is modified during
game execution.

## Inputs and events

TODO

## Savestates

Savestate is one of the core features of a TASing tool, and the most sensible one.
The procedure consists of three steps: preparing for savestate, the actual save or
load, and resuming execution.

### Preparation

One thread of the game has the role of doing all the work. It is not an extra
thread but the thread that triggered the frame boundary. The preparation for
saving and loading is mostly identical.

1. **Acquire locks**. Knowing that other threads are still running, they might be
    doing some work incompatible with savestating, such as creating new threads.
    So we must protect those calls.
1. **Stop the audio playback**. Playbacking real audio if libTAS is unmuted may
    leave undetermined state of the audio driver (e.g. samples left in audio buffer),
    so it's best to stop audio playback. Even with this, some audio drivers will
    still mess up the savestating process, so it is advised to leave the audio
    muted when TASing
1. **Check if saving/loading is possible**. For saving, it used to check for disk
    space, but it was causing several issues. First, the savestate size cannot
    be estimated beforehand, only an upper bound (which may be **much** higher).
    Also, VMs may grow the disk capacity on demand, so the check would fail to
    find enough space even if the procedure would work correctly.
    
    For loading, it checks for a perfect match between the actual set of threads
    and the set that was present when saving was performed (stored in savestate
    header). This is the main constraint on state loading, that should be fixable
    with enough work.
1. **Save original altstack state**. More on that later
1. **Lock and sync X server connections**. We want to control as much as
    possible the interactions with the rest of the system, so we must empty what
    is pending between the game and the X server, and prevent more events to be
    generated. The connections themselves to the X server cannot be saved or
    restored, so we try to keep them at the same state everytime.
1. **Suspend threads**. A signal is sent to all the other threads to run a handler
    that will make them suspend. We try to use an uncommon signal that won't be used
    by the game (currently `SIGXFSZ`). Inside the handler, each thread saves its
    thread local storage data in memory, as well as its registers (using `getcontext()`).
    They then signal the main thread that they are suspended, and wait for the
    signal to be resumed.
1. **Disable urandom handler**. Because we set up a signal handler to refill our
    replacement for `/dev/urandom`, that may trigger during the savestate process,
    we disable it
1. **Handle opened files**. When saving, we must store offsets of opened file
    descriptors, because they are not part of the process memory, so they must
    be saved in memory. Also, pipe contents are not part of the process memory
    as well, so we empty all pipes into memory.
    
    When loading, we must close all files that were not opened when the savestate
    was performed.
1. **Setup custom altstack**. During the savestate process, we need to dump all
    memory, including the stack. This is a problem because the stack is used to 
    run the savestate code. A solution is to use another location in memory for
    the stack that will be used exclusively for running the savestate code.
    This is existing feature called altstack: when a process is signaled and a
    handler is executed, it can run in a special stack instead of the original
    one. This is used originally to be able to run code when a program has its
    stack corrupted or full. We use this at our advantage by setting the altstack
    to a memory region that will be skipped from saving.
1. **Start the savestate process**
    The altstack feature being triggered by a signal, we registered at the game
    startup a handler for another signal (currently `SIGSYS`), and we call it
    using `raise()`.

Now that everything is set up, we can start the actual saving or restoring of
the memory process. To get the memory layout, we look at the file `/proc/self/pagemap`
which contains each memory segment allocated by the game.

### Saving

We first save the state header containing the list of all running threads. Then,
we open two files that will store: one for all the memory segment metadata, and
another one for the memory segment data. We don't store the data of all memory
segments, some of them are skipped:

* Special segments (`[vsyscall]`, `[vectors]`, `[vvar]`, `[vdso]`)
* Our reserved segment that is used for the altstack (as well as other stuff that
    we want them to be unaffected by savestates)
* Segments that can't be given read/write permission
* File-segments that don't have write protection
* Shared memory segments (this is definitively a TODO to handle this)

Also, many memory pages contain only zeros, so it is faster to detect those and
store them in metadata instead of blindly write to data.

Note: a possible improvement involves getting access to the page frame number (PFN)
(which requires `CAP_SYS_ADMIN`, basically root). With this information, we
could look at flags from each PFN by looking into `/proc/kpageflags`. One flag
is `ZERO_PAGE`, which immediatly tells us that the page contains zeros only.
 
We also save the `ucontext` passed into the signal handler, so that it can be
restored after state loading for restoring registers.

### Loading

Before restoring the memory, we must backup some information about the current
connections to the X server, because after state loading, it will break the continuity
of the `request` and `last_request_read` parameters that are supposed to always
increase. If we don't backup and restore those parameters, we would get a X server error.

Before restoring the savestate memory, we must make the actual and saved memory
layout to match. So we iteratively browse through each memory segment, and call
`munmap()`, `mremap()` or `mmap()` to reconstruct the saved memory layout.

For file-mapped segments, we try to map again the file if it is still present.
For the heap segment, we use `brk()` instead.

As a note, even if the memory segment was marked as skipped during the saving
process, we still need to reconstruct its memory layout.

When the memory layout matches the saved one, we can restore the memory of each
segment.

At the end, we restore the X connections parameters as well as the `ucontext`.

### Resuming

When resuming, all operations are recovered in the inverse way they were
prepared. Because of the very nature of state loading,
the code path for resuming a state saving and a state loading is identical,
because it is just as if we returned from a state saving.

So we used a special variable `restoreInProgress` that we filled in the signal
handler so that we can distinguish between resuming from state loading and state
saving.

After state loading, these extra steps need to be performed:

* When other threads are resumed, they need to recover their TLS and registers
    contained in the savestate (using `setcontext()`)
* The game sends the current framecount and time to libTAS program, so that it
    can display immediately the updated information
* The screen is redrawed to show the updated game screen after state loading

### Features

#### Store savestates in RAM

Instead of opening real files, we use the `SYS_memfd_create` syscall to create
a file in memory that provides a file descriptor. We can then use the same code,
and only retain file descriptors instead of file paths, to support savestates in
memory.

#### Compressed savestates

When saving a memory page, we feed the memory through an lz4 stream compression
function, and we also need to save the compressed size of each memory page.
For efficiency, we indicate the compression algorithm when we have multiple
consecutive memory pages to save, which increases the compression ratio.

Unfortunately, we cannot easily compress one continuous range of multiple memory
pages, because we need to be able to fetch and decompress any individual page
when loading a state.

#### Incremental savestates

This option allows taking advantage of the recent soft-dirty bit pushed by the
CRIU project, which can track which memory pages have been written to. When the
first savestate is triggered, a complete memory dump is performed (like a regular
savestate), and the soft-dirty bit is cleared. When another savestate is performed,
the soft-dirty bit is checked for each individual memory page, and only those where
the bit is checked are saved.

When loading a savestate, we can use the soft-dirty bit to skip loading memory
pages that were untouched.

#### Skip unmapped pages

One of the page flag when reading `/proc/pid/pagemap` is the `present` flag,
which indicate if a memory page has been committed in memory. Indeed, when
allocating memory, pages are not backed by real memory until they are written to.

So we can skip saving the content of those memory pages.

Using this option has historically caused crashes in some games, so it is turned
off by default, but should be safe mostly.

#### Fork to save states

Use the `fork()` feature to create a copy of the game process that will save its
memory. It works efficiently thanks to the very convenient copy-on-write
mechanism, where the forked process did not receive a copy of all the memory, but
shares its memory with the parent process, until either one modifies the memory.

So the forked process can save its memory while the game is resumed, with just
a slight overhead of the memory pages being copied.

## Video

TODO

## Audio

TODO

## Wine

TODO

## Steam

TODO

## File IO

TODO
