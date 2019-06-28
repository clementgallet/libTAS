## [Unreleased]
### Added

* Both 32-bit and 64-bit libtas.so libraries can be installed, and the program
  will select the correct one depending on the game arch
* Hook Xinerama for games gathering the monitor resolution
* Detect Windows executable and transparently call wine on it
* Add a loadbranch hotkey to load the entire savestate movie in playback mode
* Prevent the game screen from appearing unresponsive when the game is paused
* Don't scroll input editor after manual scrolling, until current frame is not visible
* Prevent monitor resolution change
* Add border/title on fullscreen window
* Check that GUI and library git commit match

### Changed

* Don't require /proc/self/pagemap to exist for savestating
* Switch to autotools

### Fixed

* Don't assume heap segments have the same protection
* Handle when state saving failed
* Fix insert frame not working after delete frame

## [1.3.4] - 2019-04-08
### Added

* Add an extra row in the input editor to add frames and copy at the end
* Use appveyor for automated building
* Add support for SDL_UpdateWindowSurface() screen update
* Hook udev to populate joystick information
* Block access to pulseaudio, so that games fallback on alsa
* Drag analog inputs in the input editor
* Implement XI2 inputs
* Add some tooltips
* Handle wait timeouts
* Add an option to enforce determinism when events are pulled asynchronously
* Show recording/playback toggle in the OSD messages
* Store custom input names from the input editor in movies
* A thread creating a window is considered as the main thread
* Add shortcuts to some input editor options

### Changed

* Implement evdev/jsdev with pipes
* Remove time tracking all threads from movies/config
* Only offer to load a previous savestate movie if it's possible
* Remove the main thread condition in nondeterministic timer

### Fixed

* Fix SDL_BlitSurface hooking
* Fix a race condition on file handle list
* Fix evdev/jsdev hat inputs
* Enforce a minimum column size for analog inputs in the input editor
* Resize columns when it is needed
* Prevent zombie game processes

## [1.3.3] - 2019-02-08
### Added

* Implement more Xlib keyboard mapping functions
* When loading a savestate of previous game iteration, the movie is loaded
* Improve the support of virtual screen resolution for non-SDL games
* Prevent Xlib games to switch to fullscreen
* Implement an auto-restart feature
* Manually add audio latency for better playback
* Add a restart input
* Add an option to make a savestate each time the thread set has changed
* Lock inputs in the input editor

### Changed

* Take advantage of SDL2 dynamic linking mechanism for better SDL2 hooking
* Store files in $XDG_DATA_HOME and $XDG_CONFIG_HOME (usually ~/.local/share and ~/.config)

### Fixed

* Fix zeroing pages when incremental savestates feature is disabled
* Specifying a movie in commandline does not truncate the file anymore
* Fix a crash when renaming the frame column in input editor
* Fix behavior when SDL_JoystickOpen is called multiple times
* Print the correct char for common keys when typing text (non-SDL)
* Restore file offsets and close files when loading a savestate
* Fix a freeze in the input editor
* Fix a typo in SDL text input
* Make savefiles streams unbuffered to avoid data loss

## [1.3.2] - 2018-11-16
### Added

* Add locale setting
* Store md5 hash of game executable in movies
* Option to enable/disable dummy Steam client
* Implement OpenAL loop points
* Hook Xlib window resize
* Support for SDL1 rendering using surface blitting
* Mouse position recalibration to match game cursor position
* Add savestate slot to input editor
* Old autosaves are removed
* Autosave configuration window
* Desktop entry file
* ALSA mmap writing

### Changed

* Use system command for compressing/decompressing instead of tar/zlib libraries
* Implement more of the Steam API
* Register removed files as savefiles
* Rename program linTAS into libTAS

### Fixed

* Initialize audio sources that are reused
* Fix controller sliders
* Fix unsigned ints showed as signed in RAM Search
* Recreate texture/fbo when a new GL context is created
* Fix SDL1 keyboard layout
* Wait for threads to register
* Detect auto-repeat cases when detectable auto-repeat is not supported by the server
* Fix crash when loading a savestate on a non-draw frame
* End screen capture when SDL renderer is destroyed
* Register all connections to X server (for savestates) instead of just one
* Fix screen capture of SDL1 surface
* Autosave and exit save when movie is modified in the input editor
* Increment rerecord count if the movie was modified in the input editor

## [1.3.1] - 2018-08-30
### Added
- Xlib cursor and pointer warping functions
- Force FMOD to output to ALSA, and removed PulseAudio hooks
- Kill button to terminate a hanging game process
- Hook SDL text input
- Copy/cut/paste options in the input editor
- Start implementing Steam API to support some Steam games
- Support pthread keys for recycled threads, add add an option to disable thread recycling
- Movies can store annotations

### Changed
- Remove all ffmpeg API, and use a pipe to send video/audio using nut muxer

### Fixed
- Fix OpenAL deadlocks
- Change frame sleep logic to get fps closer to target fps
- Auto-repeat bug, which forced to disable xlib auto-repeat during the game execution
- Encode the first frame when starting encode before the game is launched
- Encode non-draw frames at game startup as blank frames
- Add encode segment number in encode filename
- Memory region permission is set when loading a savestate
- Fix some pointers overflow on 32-bit arch
- Audio buffers are cleared before calling the callback (SDL1 Audio)

## [1.3.0] - 2018-07-30
### Added
- Analog inputs in input editor, which are also editable
- Incremental savestates
- Store savestates in disk or RAM
- Simulates a system with english language
- Display ram watches on game screen
- Add pointers to ram watches
- Add a pointer scanning window to search for chains of indirection to a specific address
- Values can be poked into a game address
- Fast-forward skip modes
- Insert frames and truncate inputs in input editor

### Changed
- Some default fonts are chosen in priority for OSD
- Better savefile handling

### Fixed
- Set the repeat attribute in SDL2 key events, fixing keyboard inputs in SWD2
- Support thread exit with recycled threads
- Fix frame boundary trigger when game sleeps at startup, may affect sync

## [1.2.0] - 2018-05-28
### Added
- Input editor
- Recording mode when input editor is opened does not truncate the movie
- Recycle threads so that savestates always see the same number of threads
- OSD element can be positioned by the user
- Add messages on the OSD
- Add a status bar on the main GUI window

### Changed
- Offer fractional instead of integer framerate. This allows games like VVVVVV which uses 34ms frame interval to run at the exact speed without additional non-draw frames
- The whole movie is stored when savestating, not just the movie until the current frame. This way, users don't loose future inputs when using recording mode with input editor opened

### Fixed
- Don't add an extra frame when user stops the game
- Remove a memory leak caused by allocated keycode-keysym mapping
- Fix garbage inputs caused by uninitialized inputs when game does not have focus
- Always simulate the game window being on top-left corner so that games using global mouse coords do not desync
- Savestates work with audio not muted
- Change the way of hooking dlfoo functions because it broke on glibc 2.27 (now using `_dl_sym` to get the real dlsym)
- Fix the time increase at frame boundary when the game had some sleep calls, so that the time remains a multiple of the framerate increment
- Fix games using SDL mixer by replacing calls to `SDL_MixAudio` by `SDL_MixAudioFormat`, the former needed an audio device opened
- Always make logical and screen coords match on SDL2

## [1.1.1] - 2018-03-25
### Added
- Simulate a specific monitor resolution.
- Improve fastforward by skipping OpenGL draw functions.
- Initialize allocated memory with zeros, fixing desyncs in some games.
- Add author field in movie files.
- Add an option to pause at a specific frame.

### Changed
- Don't rely on the keyboard layout to determine keycodes from keysyms when feeding inputs to the game. Now using a standard qwerty keyboard layout.
- Changed the way to store the game screen when using OpenGL (using framebuffers and renderbuffers). Now the screen capture stays inside the GPU, except when doing an encode.

### Fixed
- Always display the last frame of a movie during a fast-forward, so that a TAS can be more easily resumed.
- Movie file browser does not ask for overwrite confirmation anymore.
- Add an extra movie frame corresponding to the game startup, so that frame count and system time now match. Old movies will desync, the user needs to add one blank frame at the beginning.
- Using both frame-advance and fast-forward now produces a faster frame-advance running speed.

## [1.1.0] - 2018-02-25
### Added
- Add full controller inputs, including analog inputs.

### Changed
- Switch from FLTK to Qt.
- Change the config file format and movie file format to use QSettings instead
of Fl_Preferences.

### Fixed
- Use stack_t type to fix a compile error on recent glibc.
- Fix a freeze when loading a savestate after a thread has been destroyed and
immediately created, in which case pthread library recycle the same id. Now
the state cannot be loaded in this case.
- Releasing the frame advance key with another key now correctly stops the
frame advance auto-repeat feature.

## [1.0.0] - 2018-02-09

Initial release.
