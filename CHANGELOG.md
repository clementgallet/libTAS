## [Unreleased]
### Added
- Xlib cursor and pointer warping functions
- Force FMOD to output to ALSA, and removed PulseAudio hooks
- Kill button to terminate a hanging game process
- Hook SDL text input
- Copy/cut/paste options in the input editor

### Changed
- Remove all ffmpeg API, and use a pipe to send video/audio using nut muxer

### Fixed
- Fix OpenAL deadlocks
- Change frame sleep logic to get fps closer to target fps
- Auto-repeat bug, which forced to disable xlib auto-repeat during the game execution
- Encode the first frame when starting encode before the game is launched
- Encode non-draw frames at game startup as blank frames
- Add encode segment number in encode filename

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
