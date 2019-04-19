cmake_minimum_required(VERSION 3.1)

project(libtas)

set(PROGRAM_SOURCES
    src/program/AutoSave.cpp
    src/program/Config.cpp
    src/program/GameLoop.cpp
    src/program/KeyMapping.cpp
    src/program/main.cpp
    src/program/MovieFile.cpp
    src/program/utils.cpp
    src/program/ui/AnnotationsWindow.cpp
    src/program/ui/AutoSaveWindow.cpp
    src/program/ui/ControllerAxisWidget.cpp
    src/program/ui/ControllerTabWindow.cpp
    src/program/ui/ControllerWidget.cpp
    src/program/ui/EncodeWindow.cpp
    src/program/ui/ErrorChecking.cpp
    src/program/ui/ExecutableWindow.cpp
    src/program/ui/GameInfoWindow.cpp
    src/program/ui/InputEditorModel.cpp
    src/program/ui/InputEditorView.cpp
    src/program/ui/InputEditorWindow.cpp
    src/program/ui/InputWindow.cpp
    src/program/ui/KeyPressedDialog.cpp
    src/program/ui/MainWindow.cpp
    src/program/ui/OsdWindow.cpp
    src/program/ui/PointerScanModel.cpp
    src/program/ui/PointerScanWindow.cpp
    src/program/ui/RamSearchModel.cpp
    src/program/ui/RamSearchWindow.cpp
    src/program/ui/RamWatchEditWindow.cpp
    src/program/ui/RamWatchModel.cpp
    src/program/ui/RamWatchWindow.cpp
    src/program/ramsearch/IRamWatch.cpp
    src/program/ramsearch/IRamWatchDetailed.cpp
    src/program/ramsearch/MemSection.cpp
)

set(LIBRARY_SOURCES
    src/library/backtrace.cpp
    src/library/DeterministicTimer.cpp
    src/library/dlhook.cpp
    src/library/frame.cpp
    src/library/glibwrappers.cpp
    src/library/global.cpp
    src/library/GlobalState.cpp
    src/library/hook.cpp
    src/library/localewrappers.cpp
    src/library/logging.cpp
    src/library/main.cpp
    src/library/mallocwrappers.cpp
    src/library/NonDeterministicTimer.cpp
    src/library/openglwrappers.cpp
    src/library/pthreadwrappers.cpp
    src/library/randomwrappers.cpp
    src/library/ScreenCapture.cpp
    src/library/SDLEventQueue.cpp
    src/library/sdldisplay.cpp
    src/library/sdldynapi.cpp
    src/library/sdlevents.cpp
    src/library/sdlmain.cpp
    src/library/sdlmutex.cpp
    src/library/sdlrender.cpp
    src/library/sdlthreads.cpp
    src/library/sdltimer.cpp
    src/library/sdlversion.cpp
    src/library/sdlwindows.cpp
    src/library/signalwrappers.cpp
    src/library/sleepwrappers.cpp
    src/library/TimeHolder.cpp
    src/library/timewrappers.cpp
    src/library/tlswrappers.cpp
    src/library/Utils.cpp
    src/library/WindowTitle.cpp
    src/library/XlibEventQueue.cpp
    src/library/xdisplay.cpp
    src/library/xevents.cpp
    src/library/xf86vidmode.cpp
    src/library/xrandr.cpp
    src/library/xwindows.cpp
    src/library/audio/AudioBuffer.cpp
    src/library/audio/AudioContext.cpp
    src/library/audio/AudioPlayer.cpp
    src/library/audio/AudioSource.cpp
    src/library/audio/DecoderMSADPCM.cpp
    src/library/audio/alsa/control.cpp
    src/library/audio/alsa/pcm.cpp
    src/library/audio/fluidsynth/fluidsynth.cpp
    src/library/audio/fmod/fmod.cpp
    src/library/audio/openal/al.cpp
    src/library/audio/openal/alc.cpp
    src/library/audio/openal/efx.cpp
    src/library/audio/sdl/sdlaudio.cpp
    src/library/checkpoint/AltStack.cpp
    src/library/checkpoint/Checkpoint.cpp
    src/library/checkpoint/CustomSignals.cpp
    src/library/checkpoint/ProcMapsArea.cpp
    src/library/checkpoint/ProcSelfMaps.cpp
    src/library/checkpoint/ReservedMemory.cpp
    src/library/checkpoint/SaveState.cpp
    src/library/checkpoint/ThreadLocalStorage.cpp
    src/library/checkpoint/ThreadManager.cpp
    src/library/checkpoint/ThreadSync.cpp
    src/library/encoding/AVEncoder.cpp
    src/library/encoding/NutMuxer.cpp
    src/library/fileio/FileHandleList.cpp
    src/library/fileio/generaliowrappers.cpp
    src/library/fileio/posixiowrappers.cpp
    src/library/fileio/SaveFile.cpp
    src/library/fileio/SaveFileList.cpp
    src/library/fileio/stdiowrappers.cpp
    src/library/inputs/evdev.cpp
    src/library/inputs/inputevents.cpp
    src/library/inputs/inputs.cpp
    src/library/inputs/ioctl_joy.cpp
    src/library/inputs/jsdev.cpp
    src/library/inputs/keyboard_helper.cpp
    src/library/inputs/sdlgamecontroller.cpp
    src/library/inputs/sdlhaptic.cpp
    src/library/inputs/sdljoystick.cpp
    src/library/inputs/sdlkeyboard.cpp
    src/library/inputs/sdlkeyboardlayout.cpp
    src/library/inputs/sdlpointer.cpp
    src/library/inputs/sdltextinput.cpp
    src/library/inputs/udevwrappers.cpp
    src/library/inputs/xcbkeyboard.cpp
    src/library/inputs/xkeyboard.cpp
    src/library/inputs/xkeyboardlayout.cpp
    src/library/inputs/xinput.cpp
    src/library/inputs/xpointer.cpp
    src/library/renderhud/RenderHUD_GL.cpp
    src/library/renderhud/RenderHUD_SDL1.cpp
    src/library/renderhud/RenderHUD_SDL2_renderer.cpp
    src/library/renderhud/RenderHUD_SDL2_surface.cpp
    src/library/renderhud/RenderHUD.cpp
    src/library/renderhud/sdl_ttf.cpp
    src/library/renderhud/SurfaceARGB.cpp
    src/library/steam/isteamapps.cpp
    src/library/steam/isteamclient.cpp
    src/library/steam/isteamcontroller.cpp
    src/library/steam/isteamfriends.cpp
    src/library/steam/isteamremotestorage.cpp
    src/library/steam/isteamscreenshots.cpp
    src/library/steam/isteamugc.cpp
    src/library/steam/isteamuser.cpp
    src/library/steam/isteamuserstats.cpp
    src/library/steam/isteamutils.cpp
    src/library/steam/steamapi.cpp
    src/library/steam/steamapiinternal.cpp
    src/library/test/hooktest.cpp
)

set(SHARED_SOURCES
    src/shared/AllInputs.cpp
    src/shared/SingleInput.cpp
    src/shared/sockethelpers.cpp
)

set(EXECUTABLE_OUTPUT_PATH ./)
set(LIBRARY_OUTPUT_PATH ./)
#set(CMAKE_CXX_STANDARD 11)
set(CMAKE_INSTALL_RPATH "$ORIGIN")
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_AUTOMOC ON)

add_executable(libTAS ${PROGRAM_SOURCES} ${SHARED_SOURCES})
add_library(tas SHARED ${LIBRARY_SOURCES} ${SHARED_SOURCES})

# Add some c++ requirements
target_compile_features(tas PRIVATE cxx_auto_type cxx_nullptr cxx_range_for cxx_variadic_templates)
target_compile_features(libTAS PRIVATE cxx_auto_type cxx_range_for)

# Common debug flags
target_compile_options(tas PUBLIC -fvisibility=hidden)
target_compile_options(libTAS PUBLIC -Wno-float-equal)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}  -Wall -Wextra -Wmissing-include-dirs -Wmissing-declarations -Wfloat-equal -Wundef -Wcast-align -Winit-self -Wshadow -Wno-unused-parameter -Wno-missing-field-initializers")

# Set optimization flags for release
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG -fweb -flto=1 -fno-stack-protector")
set(CMAKE_CFLAGS_RELEASE "-O2 -DNDEBUG -fweb -flto=1 -fno-stack-protector")

# Add Qt5 lib
find_package(Qt5Widgets REQUIRED)
if (Qt5Widgets_FOUND)
    if (Qt5Widgets_VERSION VERSION_LESS 5.6.0)
        message(FATAL_ERROR "Minimum supported Qt5 version is 5.6.0")
    endif()
else()
    message(SEND_ERROR "The Qt5Widgets library could not be found!")
endif(Qt5Widgets_FOUND)
target_link_libraries (libTAS Qt5::Widgets)

# Add pthread
find_package(Threads REQUIRED)
target_link_libraries(libTAS Threads::Threads)
target_link_libraries(tas Threads::Threads)

# Add XCB libraries
find_package(ECM REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_MODULE_PATH})

find_package(XCB COMPONENTS XCB KEYSYMS XKB CURSOR REQUIRED)
include_directories(${XCB_INCLUDE_DIRS})
target_link_libraries (libTAS ${XCB_LIBRARIES})

# Add X11_XCB library
find_package(X11_XCB REQUIRED)
include_directories(${X11_XCB_INCLUDE_DIR})
target_link_libraries (tas ${X11_XCB_LIBRARIES})

# Add X11 library
find_package(X11 REQUIRED)
include_directories(${X11_X11_INCLUDE_DIRS})
target_link_libraries (libTAS ${X11_X11_LIB})
target_link_libraries (tas ${X11_X11_LIB})

# Add xf86vidmode wrapper if the extension is present
if (X11_xf86vmode_FOUND)
    add_definitions(-DLIBTAS_HAS_XF86VMODE)
endif()

# Add xrandr wrapper if the extension is present
if (X11_Xrandr_FOUND)
    add_definitions(-DLIBTAS_HAS_XRANDR)
endif()

# Add xrandr wrapper if the extension is present
if (X11_Xi_FOUND)
    add_definitions(-DLIBTAS_HAS_XINPUT)
endif()

# Add optional features
find_package(PkgConfig REQUIRED)

# Check for SDL2 headers
pkg_check_modules(SDL2 REQUIRED sdl2)
target_include_directories(tas PUBLIC ${SDL2_INCLUDE_DIRS})

# Check for ffmpeg
find_program(FFMPEG ffmpeg)
if (NOT FFMPEG)
    message(WARNING "ffmpeg was not found. Encoding won't work")
endif()

# Interim
option(INTERIM_VERSION "Interim version" ON)
if (INTERIM_VERSION)

    # Get the latest abbreviated commit hash of the working branch
    execute_process(
        COMMAND git log -1 --format=%h
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_COMMIT_HASH
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    # Add commit number as macro
    message(STATUS "Building interim commit " ${GIT_COMMIT_HASH})
    add_definitions(-DLIBTAS_INTERIM_COMMIT="${GIT_COMMIT_HASH}")

    # Add build date as macro
    string(TIMESTAMP TODAY "%Y-%m-%d")
    add_definitions(-DLIBTAS_INTERIM_DATE="${TODAY}")

endif()

# Sound playback
pkg_check_modules(ALSA REQUIRED alsa)
target_include_directories(tas PUBLIC ${ALSA_INCLUDE_DIRS})
target_link_libraries(tas ${ALSA_LIBRARIES})
link_directories(${ALSA_LIBRARY_DIRS})

pkg_check_modules(SWRESAMPLE REQUIRED libswresample)
target_include_directories(tas PUBLIC ${SWRESAMPLE_INCLUDE_DIRS})
target_link_libraries(tas ${SWRESAMPLE_LIBRARIES})
link_directories(${SWRESAMPLE_LIBRARY_DIRS})

# HUD
option(ENABLE_HUD "Enable HUD" ON)

pkg_check_modules(FREETYPE freetype2 fontconfig)
if (ENABLE_HUD AND FREETYPE_FOUND)
    # Enable HUD
    message(STATUS "HUD is enabled")
    target_include_directories(tas PUBLIC ${FREETYPE_INCLUDE_DIRS} ${FONTCONFIG_INCLUDE_DIRS})
    target_link_libraries(tas ${FREETYPE_LIBRARIES} ${FONTCONFIG_LIBRARIES})
    link_directories(${FREETYPE_LIBRARY_DIRS} ${FONTCONFIG_LIBRARY_DIRS})
    add_definitions(-DLIBTAS_ENABLE_HUD)
else()
    message(WARNING "HUD is disabled")
endif()

#add helper script to build directory.
configure_file(libTasConcat.sh ./libTasConcat.sh COPYONLY)

# Install program and library
install(TARGETS libTAS tas DESTINATION bin)

# Install dump segment automerge helper script
install(FILES libTasConcat.sh DESTINATION bin)

# Install desktop entry
install(FILES libTAS.desktop DESTINATION share/applications)
