if BUILD32LIBONLY
bin_PROGRAMS =
else
bin_PROGRAMS = libtas.so
endif

libtas_so_SOURCES = \
    backtrace.cpp \
    BusyLoopDetection.cpp \
    DeterministicTimer.cpp \
    dlhook.cpp \
    eglwrappers.cpp \
    frame.cpp \
    GameHacks.cpp \
    glibwrappers.cpp \
    global.cpp \
    glxwrappers.cpp \
    GlobalState.cpp \
    hook.cpp \
    hookpatch.cpp \
    localewrappers.cpp \
    logging.cpp \
    main.cpp \
    mallocwrappers.cpp \
    NonDeterministicTimer.cpp \
    openglwrappers.cpp \
    pthreadwrappers.cpp \
    randomwrappers.cpp \
    ScreenCapture.cpp \
    signalwrappers.cpp \
    sleepwrappers.cpp \
    Stack.cpp \
    systemwrappers.cpp \
    TimeHolder.cpp \
    timewrappers.cpp \
    tlswrappers.cpp \
    Utils.cpp \
    vdpauwrappers.cpp \
    vulkanwrappers.cpp \
    waitwrappers.cpp \
    WindowTitle.cpp \
    audio/AudioBuffer.cpp \
    audio/AudioContext.cpp \
    audio/AudioConverterSwr.cpp \
    audio/AudioPlayerAlsa.cpp \
    audio/AudioSource.cpp \
    audio/DecoderMSADPCM.cpp \
    audio/alsa/control.cpp \
    audio/alsa/pcm.cpp \
    audio/cubeb/cubeb.cpp \
    audio/fluidsynth/fluidsynth.cpp \
    audio/fmod/fmod.cpp \
    audio/openal/al.cpp \
    audio/openal/alc.cpp \
    audio/openal/efx.cpp \
    audio/sdl/sdlaudio.cpp \
    checkpoint/AltStack.cpp \
    checkpoint/Checkpoint.cpp \
    checkpoint/MemArea.cpp \
    checkpoint/ProcSelfMaps.cpp \
    checkpoint/ReservedMemory.cpp \
    checkpoint/SaveState.cpp \
    checkpoint/SaveStateManager.cpp \
    checkpoint/ThreadLocalStorage.cpp \
    checkpoint/ThreadManager.cpp \
    checkpoint/ThreadSync.cpp \
    encoding/AVEncoder.cpp \
    encoding/NutMuxer.cpp \
    fileio/dirwrappers.cpp \
    fileio/FileHandleList.cpp \
    fileio/generaliowrappers.cpp \
    fileio/posixiowrappers.cpp \
    fileio/SaveFile.cpp \
    fileio/SaveFileList.cpp \
    fileio/stdiowrappers.cpp \
    fileio/URandom.cpp \
    inputs/evdev.cpp \
    inputs/inputevents.cpp \
    inputs/inputs.cpp \
    inputs/ioctl_joy.cpp \
    inputs/jsdev.cpp \
    inputs/keyboard_helper.cpp \
    inputs/sdlgamecontroller.cpp \
    inputs/sdlhaptic.cpp \
    inputs/sdljoystick.cpp \
    inputs/sdlkeyboard.cpp \
    inputs/sdlkeyboardlayout.cpp \
    inputs/sdlpointer.cpp \
    inputs/sdltextinput.cpp \
    inputs/udevwrappers.cpp \
    inputs/winekeyboardlayout.cpp \
    inputs/xcbkeyboard.cpp \
    inputs/xcbpointer.cpp \
    inputs/xkeyboard.cpp \
    inputs/xkeyboardlayout.cpp \
    inputs/xinput.cpp \
    inputs/xpointer.cpp \
    renderhud/RenderHUD_GL.cpp \
    renderhud/RenderHUD_SDL1.cpp \
    renderhud/RenderHUD_SDL2_renderer.cpp \
    renderhud/RenderHUD_SDL2_surface.cpp \
    renderhud/RenderHUD_VDPAU.cpp \
    renderhud/RenderHUD_XShm.cpp \
    renderhud/RenderHUD.cpp \
    renderhud/RenderHUD_Base_Linux.cpp \
    renderhud/sdl_ttf.cpp \
    renderhud/SurfaceARGB.cpp \
    renderhud/SurfaceXImage.cpp \
    sdl/SDLEventQueue.cpp \
    sdl/sdldisplay.cpp \
    sdl/sdldynapi.cpp \
    sdl/sdlevents.cpp \
    sdl/sdlmain.cpp \
    sdl/sdlmutex.cpp \
    sdl/sdlrender.cpp \
    sdl/sdlthreads.cpp \
    sdl/sdltime.cpp \
    sdl/sdltimer.cpp \
    sdl/sdlversion.cpp \
    sdl/sdlwindows.cpp \
    steam/CCallback.cpp \
    steam/CCallbackManager.cpp \
    steam/isteamapps.cpp \
    steam/isteamclient/isteamclient.cpp \
    steam/isteamclient/isteamclient006.cpp \
    steam/isteamclient/isteamclient012.cpp \
    steam/isteamclient/isteamclient014.cpp \
    steam/isteamclient/isteamclient016.cpp \
    steam/isteamclient/isteamclient017.cpp \
    steam/isteamclient/isteamclient020.cpp \
    steam/isteamcontroller.cpp \
    steam/isteamfriends.cpp \
    steam/isteamgamecoordinator.cpp \
    steam/isteamgameserver.cpp \
    steam/isteamhttp.cpp \
    steam/isteammatchmaking.cpp \
    steam/isteamnetworking.cpp \
    steam/isteamremotestorage/isteamremotestorage.cpp \
    steam/isteamremotestorage/isteamremotestorage001.cpp \
    steam/isteamremotestorage/isteamremotestorage012.cpp \
    steam/isteamremotestorage/isteamremotestorage013.cpp \
    steam/isteamremotestorage/isteamremotestorage014.cpp \
    steam/isteamscreenshots.cpp \
    steam/isteamugc.cpp \
    steam/isteamuser.cpp \
    steam/isteamuserstats.cpp \
    steam/isteamutils.cpp \
    steam/steamapi.cpp \
    steam/steamapiinternal.cpp \
    steam/steamgameserver.cpp \
    wine/winehook.cpp \
    wine/wined3d.cpp \
    wine/user32.cpp \
    wine/kernel32.cpp \
    xcb/XcbEventQueue.cpp \
    xcb/XcbEventQueueList.cpp \
    xcb/xcbconnection.cpp \
    xcb/xcbevents.cpp \
    xcb/xcbwindows.cpp \
    xlib/XlibEventQueue.cpp \
    xlib/XlibEventQueueList.cpp \
    xlib/xatom.cpp \
    xlib/xdisplay.cpp \
    xlib/xevents.cpp \
    xlib/xf86vidmode.cpp \
    xlib/xinerama.cpp \
    xlib/xrandr.cpp \
    xlib/xshm.cpp \
    xlib/xwindows.cpp \
    ../shared/AllInputs.cpp \
    ../shared/SingleInput.cpp \
    ../shared/sockethelpers.cpp \
    ../external/lz4.cpp \
    ../external/elfhacks.cpp	

libtas_so_CXXFLAGS = $(LIBSWRESAMPLE_CFLAGS) -fPIC -shared -fvisibility=hidden -fno-stack-protector
libtas_so_CXXFLAGS += -DSOCKET_LOG
libtas_so_LDFLAGS = -shared
libtas_so_LDADD = $(LIBRARY_LIBS)

if BUILD32LIB
bin_PROGRAMS += libtas32.so
libtas32_so_SOURCES = $(libtas_so_SOURCES)
libtas32_so_CXXFLAGS = -m32 $(libtas_so_CXXFLAGS)
libtas32_so_LDFLAGS = $(libtas_so_LDFLAGS)
libtas32_so_LDADD = $(LIBRARY32_LIBS)
endif
