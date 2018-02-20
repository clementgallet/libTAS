cmake_minimum_required(VERSION 3.1)

project(libTAS)
# From here : http://stackoverflow.com/questions/28250081/when-should-i-rerun-cmake
# Note: We do not recommend using GLOB to collect a list of source files from your source tree.
# If no CMakeLists.txt file changes when a source is added or removed,
# then the generated build system cannot know when to ask CMake to regenerate.
file(GLOB_RECURSE lib_sources src/libTAS/*)
file(GLOB_RECURSE lin_sources src/linTAS/*)
file(GLOB shared_sources src/shared/*)
file(GLOB external_sources src/external/*)

set(EXECUTABLE_OUTPUT_PATH ./)
set(LIBRARY_OUTPUT_PATH ./)
#set(CMAKE_CXX_STANDARD 11)
set(CMAKE_INSTALL_RPATH "$ORIGIN")
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_AUTOMOC ON)

# Include FLTK path
set(FLTK_SKIP_OPENGL TRUE)
set(FLTK_SKIP_FORMS TRUE)
set(FLTK_SKIP_IMAGES TRUE)
set(FLTK_SKIP_FLUID TRUE)
find_package(FLTK REQUIRED NO_MODULE)
include_directories(${FLTK_INCLUDE_DIRS})

add_executable(linTAS ${lin_sources} ${shared_sources} ${external_sources})
add_library(TAS SHARED ${lib_sources} ${shared_sources} ${external_sources})

# Add some c++ requirements
target_compile_features(TAS PRIVATE cxx_auto_type cxx_nullptr cxx_range_for cxx_variadic_templates)
target_compile_features(linTAS PRIVATE cxx_auto_type cxx_range_for)

# Common debug flags
target_compile_options(TAS PUBLIC -fvisibility=hidden)
target_compile_options(linTAS PUBLIC -Wno-float-equal)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}  -Wall -Wextra -Wmissing-include-dirs -Wmissing-declarations -Wfloat-equal -Wundef -Wcast-align -Winit-self -Wshadow -Wno-unused-parameter -Wno-missing-field-initializers")
#set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2")

# Add librt for shm_open
#find_library(RT_LIB rt)
#target_link_libraries (linTAS ${RT_LIB})

#target_link_libraries(TAS ${CMAKE_DL_LIBS})

# Add Qt5 lib
find_package(Qt5Widgets)
target_link_libraries (linTAS Qt5::Widgets)

# Add TAR lib
find_library(TAR_LIB tar)
target_link_libraries(linTAS ${TAR_LIB})

# Add pthread
find_package(Threads REQUIRED)
target_link_libraries(linTAS Threads::Threads)
target_link_libraries(TAS Threads::Threads)

# Add XCB libraries
find_package(ECM REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_MODULE_PATH})

find_package(XCB COMPONENTS XCB KEYSYMS XKB REQUIRED)
include_directories(${XCB_INCLUDE_DIRS})
target_link_libraries (linTAS ${XCB_LIBRARIES})

# Add X11 library
find_package(X11 REQUIRED)
include_directories(${X11_X11_INCLUDE_DIRS})
target_link_libraries (linTAS ${X11_X11_LIB})
target_link_libraries (TAS ${X11_X11_LIB})

# Add FLTK library
target_link_libraries(linTAS fltk)

# Add zlib library
find_package(ZLIB REQUIRED)
include_directories(${ZLIB_INCLUDE_DIRS})
target_link_libraries(linTAS ${ZLIB_LIBRARIES})

# Add optional features
find_package(PkgConfig REQUIRED)

# Check for SDL2 headers
pkg_check_modules(SDL2 REQUIRED sdl2)
target_include_directories(TAS PUBLIC ${SDL2_INCLUDE_DIRS})

# AV dumping
option(ENABLE_DUMPING "Enable AV dumping" ON)

pkg_check_modules(SWRESAMPLE libswresample)
pkg_check_modules(AVVIDEO libavcodec>=57.48.101 libswscale libavformat libavutil)
if (ENABLE_DUMPING AND SWRESAMPLE_FOUND AND AVVIDEO_FOUND)
    # Enable av dumping
    message(STATUS "AV dumping is enabled")
    target_include_directories(TAS PUBLIC ${SWRESAMPLE_INCLUDE_DIRS} ${AVVIDEO_INCLUDE_DIRS})
    link_directories(${SWRESAMPLE_LIBRARY_DIRS} ${AVVIDEO_LIBRARY_DIRS})
    target_link_libraries(TAS ${SWRESAMPLE_LIBRARIES} ${AVVIDEO_LIBRARIES})
    target_link_libraries(linTAS ${AVVIDEO_LIBRARIES})
    add_definitions(-DLIBTAS_ENABLE_AVDUMPING)
else()
    message(WARNING "AV dumping is disabled")
endif()

# Sound playback
option(ENABLE_SOUND "Enable sound playback" ON)

pkg_check_modules(ALSA alsa)
if (ENABLE_SOUND AND SWRESAMPLE_FOUND AND ALSA_FOUND)
    # Enable sound playback
    message(STATUS "Sound playback is enabled")
    target_include_directories(TAS PUBLIC ${SWRESAMPLE_INCLUDE_DIRS} ${ALSA_INCLUDE_DIRS})
    target_link_libraries(TAS ${SWRESAMPLE_LIBRARIES} ${ALSA_LIBRARIES})
    link_directories(${SWRESAMPLE_LIBRARY_DIRS} ${ALSA_LIBRARY_DIRS})
    add_definitions(-DLIBTAS_ENABLE_SOUNDPLAYBACK)
else()
    message(WARNING "Sound playback is disabled")
endif()

# HUD
option(ENABLE_HUD "Enable HUD" ON)

pkg_check_modules(FREETYPE freetype2 fontconfig)
if (ENABLE_HUD AND FREETYPE_FOUND)
    # Enable HUD
    message(STATUS "HUD is enabled")
    target_include_directories(TAS PUBLIC ${FREETYPE_INCLUDE_DIRS})
    target_link_libraries(TAS ${FREETYPE_LIBRARIES})
    link_directories(${FREETYPE_LIBRARY_DIRS})
    add_definitions(-DLIBTAS_ENABLE_HUD)
else()
    message(WARNING "HUD is disabled")
endif()

# FILEIO HOOKING
option(ENABLE_FILEIO_HOOKING "Enable file IO hooking" ON)
if (ENABLE_FILEIO_HOOKING)
    # Enable file IO hooking
    message(STATUS "File IO hooking is enabled")
    add_definitions(-DLIBTAS_ENABLE_FILEIO_HOOKING)
else()
    message(WARNING "File IO hooking is disabled")
endif()

install(TARGETS linTAS TAS DESTINATION bin)
