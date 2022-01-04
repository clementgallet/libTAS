/*
    Copyright 2015-2020 Clément Gallet <clement.gallet@ens-lyon.org>

    This file is part of libTAS.

    libTAS is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    libTAS is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with libTAS.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LIBTAS_SHAREDCONFIG_H_INCLUDED
#define LIBTAS_SHAREDCONFIG_H_INCLUDED

#include "lcf.h"
#include <cstdint>

struct __attribute__((packed, aligned(8))) SharedConfig {
    /* By how much is the speed reduced */
    int speed_divisor = 1;

    /* Fastforward mode */
    enum FastForwardMode {
        FF_SLEEP = 0x01, // Skips sleep calls
        FF_MIXING = 0x02, // Skips audio mixing
        FF_RENDERING = 0x04, // Skips all rendering
    };
    int fastforward_mode = FF_SLEEP | FF_MIXING;

    /* Recording status */
    enum RecStatus {
        NO_RECORDING,
        RECORDING_WRITE,
        RECORDING_READ // Read until end of movie or toggle
    };
    int recording = NO_RECORDING;

    /* Movie framecount */
    uint64_t movie_framecount = 0;

    /* Frame count at the start of the game. Used when game has restarted */
    uint64_t initial_framecount = 0;

    /* Log status */
    enum LogStatus {
        NO_LOGGING,
        LOGGING_TO_CONSOLE,
        LOGGING_TO_FILE
    };

    int logging_status = LOGGING_TO_CONSOLE;

    /* Which flags trigger a debug message */
    LogCategoryFlag includeFlags = LCF_ERROR | LCF_WARNING | LCF_INFO;

    /* Which flags prevent triggering a debug message */
    LogCategoryFlag excludeFlags = LCF_NONE;

    /* Framerate at which the game is running, as a fraction
     * Set to 0 to use the nondeterministic timer
     * In that case, AV dumping is disabled.
     */
    unsigned int framerate_num = 60;
    unsigned int framerate_den = 1;

    /* Number of SDL controllers to (virtually) plug in */
    int nb_controllers = 0;

    /* Log status */
    enum OSDFlags {
        OSD_FRAMECOUNT = 0x01,
        OSD_INPUTS = 0x02,
        OSD_MESSAGES = 0x04,
        OSD_RAMWATCHES = 0x08,
        OSD_LUA = 0x10
    };

    /* Elements to be displayed on the OSD */
    int osd = OSD_FRAMECOUNT | OSD_INPUTS | OSD_MESSAGES | OSD_RAMWATCHES | OSD_LUA;

    /* OSD text location */
    enum OSDLocation {
        OSD_LEFT = 0x01,
        OSD_HCENTER = 0x02,
        OSD_RIGHT = 0x04,
        OSD_TOP = 0x10,
        OSD_VCENTER = 0x20,
        OSD_BOTTOM = 0x40,
    };

    int osd_frame_location = OSD_LEFT | OSD_TOP;
    int osd_inputs_location = OSD_LEFT | OSD_BOTTOM;
    int osd_messages_location = OSD_RIGHT | OSD_BOTTOM;
    int osd_ramwatches_location = OSD_RIGHT | OSD_TOP;

    /** Sound config **/
    /* Bit depth of the buffer (usually 8 or 16) */
    int audio_bitdepth = 16;

    /* Number of channels of the buffer */
    int audio_channels = 2;

    /* Frequency of buffer in Hz */
    int audio_frequency = 44100;

    /* Encode config */
    int video_codec = 2; // x264rgb
    int video_bitrate = 4000;
    int video_framerate = 60; // used when variable framerate
    int audio_codec = 0;
    int audio_bitrate = 128;

    /* An enum indicating which time-getting function query the time */
    enum TimeCallType
    {
        TIMETYPE_UNTRACKED = -1,
        TIMETYPE_TIME = 0,
        TIMETYPE_GETTIMEOFDAY,
        TIMETYPE_CLOCK,
        TIMETYPE_CLOCKGETTIME,
        TIMETYPE_SDLGETTICKS,
        TIMETYPE_SDLGETPERFORMANCECOUNTER,
        TIMETYPE_GETTICKCOUNT,
        TIMETYPE_GETTICKCOUNT64,
        TIMETYPE_QUERYPERFORMANCECOUNTER,
        TIMETYPE_NUMTRACKEDTYPES
    };

    /* Limit for each time-getting method before time auto-advances to
     * avoid a freeze. Distinguish between main and secondary threads.
     */
    int main_gettimes_threshold[TIMETYPE_NUMTRACKEDTYPES] = {-1, -1, -1, -1, -1, -1, -1, -1, -1};
    int sec_gettimes_threshold[TIMETYPE_NUMTRACKEDTYPES] = {-1, -1, -1, -1, -1, -1, -1, -1, -1};

    /* Initial system time at game startup */
    /* We don't use struct timespec because it contains longs so the size
     * depends on the arch.*/
    int64_t initial_time_sec = 1;
    int64_t initial_time_nsec = 0;

    /* Virtual monitor resolution */
    int screen_width = 0;
    int screen_height = 0;

    /* Debug flags */
    enum DebugFlags {
        DEBUG_UNCONTROLLED_TIME = 0x01, // Using undeterministic timer
        DEBUG_NATIVE_EVENTS = 0x02, // Allow game to access real events
        DEBUG_MAIN_FIRST_THREAD = 0x04 // Keep main thread as first thread
    };

    int debug_state = 0;

    /* An enum indicating which lang are we enforcing */
    enum LocaleType
    {
        LOCALE_ENGLISH,
        LOCALE_JAPANESE,
        LOCALE_KOREAN,
        LOCALE_CHINESE_SIMPLIFIED,
        LOCALE_CHINESE_TRADITIONAL,
        LOCALE_SPANISH,
        LOCALE_GERMAN,
        LOCALE_FRENCH,
        LOCALE_ITALIAN,
        LOCALE_NATIVE
    };

    int locale = LOCALE_ENGLISH;

    /* An enum indicating which wait are we doing */
    enum AsyncType
    {
        ASYNC_JSDEV = 0x01,
        ASYNC_EVDEV = 0x02,
        ASYNC_XEVENTS_BEG = 0x04,
        ASYNC_XEVENTS_END = 0x08,
        ASYNC_SDLEVENTS_BEG = 0x10,
        ASYNC_SDLEVENTS_END = 0x20,
    };

    /* Wait for specific events to be processed in a separate thread before starting frame */
    int async_events = 0;

    /* An enum indicating which wait are we doing */
    enum WaitType
    {
        WAIT_NATIVE,
        WAIT_INFINITE,
        WAIT_FULL_INFINITE,
        WAIT_FINITE,
        WAIT_FULL,
        NO_WAIT,
    };

    /* How are we handling waits */
    int wait_timeout = WAIT_NATIVE;

    /* An enum indicating enabled game-specific timing settings */
    enum GameSpecificTiming
    {
        GC_TIMING_CELESTE = 0x01,
    };

    /* Game-specific timing settings */
    int game_specific_timing = 0;

    /* An enum indicating enabled game-specific timing settings */
    enum GameSpecificSync
    {
        GC_SYNC_CELESTE = 0x01,
        GC_SYNC_WITNESS = 0x02,
    };

    /* Game-specific timing settings */
    int game_specific_sync = 0;

    /* An enum indicating several savestate options */
    enum SaveStateFlags
    {
        SS_INCREMENTAL = 0x01, /* Using incremental savestates */
        SS_RAM = 0x02, /* Storing savestates in RAM */
        SS_BACKTRACK = 0x04, /* Saving a backtrack savestate each time a thread is created/destroyed */
        SS_COMPRESSED = 0x08, /* Compress savestates */
        SS_PRESENT = 0x10, /* Skip unmapped pages */
        SS_FORK = 0x20, /* Use a forked process to save the state */
    };

    /* Savestate settings */
    int savestate_settings = SS_COMPRESSED;

    /* Stacktrace hash to advance time */
    uint64_t busy_loop_hash = 0;

    /* Is the game running or on pause */
    bool running = false;

    /* Is fastforward enabled */
    bool fastforward = false;

    /* Are we dumping audio and video? */
    bool av_dumping = false;

    /* Are we recording and sending mouse inputs to the game? */
    bool mouse_support = true;

    /* Are we sending absolute or relative mouse movements */
    bool mouse_mode_relative = false;

    /* Are preventing the game from warping the cursor */
    bool mouse_prevent_warp = false;

    /* Display OSD in the video encode */
    bool osd_encode = false;

    /* Use a backup of savefiles in memory, which leaves the original
     * savefiles unmodified and save the content in savestates */
    bool prevent_savefiles = true;

    /* Write back the content of backup savefiles into their file when the game
     * quits. This is necessary if the game is restarted.
     */
    bool write_savefiles_on_exit = false;

    /* Mute audio */
    bool audio_mute = true;

    /* Prevent audio device from being initialized */
    bool audio_disabled = false;

    /* Recycle threads when they terminate */
    bool recycle_threads = false;

    /* Simulates a virtual Steam client */
    bool virtual_steam = false;

    /* Force Mesa software OpenGL driver */
    bool opengl_soft = true;

    /* Enable OpenGL performance tweaks */
    bool opengl_performance = false;

    /* Tries to detect busy loops and advance time */
    bool busyloop_detection = false;

    /* User can modify the framerate during the game execution */
    bool variable_framerate = false;

    /* Send stack traces of all time calls to libTAS program */
    bool time_trace = false;

    /* Call raise(SIGINT) in libtas::init */
    bool sigint_upon_launch = false;
};

#endif
