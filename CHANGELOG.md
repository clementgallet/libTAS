## [Unreleased]
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
