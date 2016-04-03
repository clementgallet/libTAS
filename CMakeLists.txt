cmake_minimum_required(VERSION 3.0)

project(libTAS)

file(GLOB_RECURSE lib_sources src/libTAS/*)
file(GLOB_RECURSE lin_sources src/linTAS/*)
file(GLOB shared_sources src/shared/*)
file(GLOB external_sources src/external/*)

set(EXECUTABLE_OUTPUT_PATH bin/)
set(LIBRARY_OUTPUT_PATH bin/)
#set(CMAKE_CXX_STANDARD 11)

add_executable(linTAS ${lin_sources} ${shared_sources} ${external_sources})
add_library(TAS SHARED ${lib_sources} ${shared_sources} ${external_sources})

# Add some c++ requirements
target_compile_features(TAS PRIVATE cxx_auto_type cxx_nullptr cxx_range_for cxx_variadic_templates)
target_compile_features(linTAS PRIVATE cxx_auto_type cxx_range_for)

# Common flags
target_compile_options(TAS PUBLIC -g -fvisibility=hidden -Wall -Wextra -Wmissing-include-dirs -Wmissing-declarations -Wfloat-equal -Wundef -Wcast-align -Wredundant-decls -Winit-self -Wshadow)

# Add X11 library
find_package(X11 REQUIRED)
include_directories(${X11_X11_INCLUDE_DIRS})
target_link_libraries (linTAS ${X11_X11_LIB})
target_link_libraries (TAS ${X11_X11_LIB})

# Add optional features
find_package(PkgConfig REQUIRED)

# AV dumping
option(ENABLE_DUMPING "Enable AV dumping" ON)

pkg_check_modules(SWRESAMPLE libswresample)
pkg_check_modules(AVVIDEO libavcodec libswscale libavformat)
if (ENABLE_DUMPING AND SWRESAMPLE_FOUND AND AVVIDEO_FOUND)
    # Enable av dumping
    message(STATUS "AV dumping is enabled")
    target_include_directories(TAS PUBLIC ${SWRESAMPLE_INCLUDE_DIRS} ${AVVIDEO_INCLUDE_DIRS})
    link_directories(${SWRESAMPLE_LIBRARY_DIRS} ${AVVIDEO_LIBRARY_DIRS})
    target_link_libraries(TAS ${SWRESAMPLE_LIBRARIES} ${AVVIDEO_LIBRARIES})
    add_definitions(-DLIBTAS_ENABLE_AVDUMPING)
else()
    message(WARNING "AV dumping is disabled")
endif()

# Sound playback
option(ENABLE_SOUND "Enable sound playback" ON)

pkg_check_modules(PULSESIMPLE libpulse-simple)
if (ENABLE_SOUND AND SWRESAMPLE_FOUND AND PULSESIMPLE_FOUND)
    # Enable sound playback
    message(STATUS "Sound playback is enabled")
    target_include_directories(TAS PUBLIC ${SWRESAMPLE_INCLUDE_DIRS} ${PULSESIMPLE_INCLUDE_DIRS})
    target_link_libraries(TAS ${SWRESAMPLE_LIBRARIES} ${PULSESIMPLE_LIBRARIES})
    link_directories(${SWRESAMPLE_LIBRARY_DIRS} ${PULSESIMPLE_LIBRARY_DIRS})
    add_definitions(-DLIBTAS_ENABLE_SOUNDPLAYBACK)
else()
    message(WARNING "Sound playback is disabled")
endif()

# HUD
option(ENABLE_HUD "Enable HUD" OFF)

pkg_check_modules(FREETYPE freetype2)
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

