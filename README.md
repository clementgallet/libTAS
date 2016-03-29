# libTAS

GNU/Linux software to (hopefully) give TAS tools to games. Code orginates from [SuperMeatBoyTaser](https://github.com/DeathlyDeep/SuperMeatBoyTaser).

# Compile

The current dependancies so far are:

- Xlib

To enable video dumping, you will need:

- libavcodec
- libavformat
- libavutil
- libswscale

To enable audio dumping or audio playback, you will need:

- libavresample

To enable audio playback, you will also need:

- libpulse-simple

To enable HUD on top of the game screen, you will need:

- libfreetype

Be careful that you must compile your code in the same arch as the game. Type `make 32bit` or `make 64bit`.

# Run

This program supports for now only games based on the SDL library. You must give the path to the SDL lib generally bundled with the game.

```./run.sh gameexecutable [game_commandline_arguments]```

For now, what you can do is:

- frame advancing, using the `V` key
- pause/play, using the `pause` key
- fast forward, using the `tab` key
- record and playback inputs
- dump the video

