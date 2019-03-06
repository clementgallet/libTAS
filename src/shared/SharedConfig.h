/*
    Copyright 2015-2018 Clément Gallet <clement.gallet@ens-lyon.org>

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
#include <time.h>

struct SharedConfig {
    /* Is the game running or on pause */
    bool running = false;

    /* By how much is the speed reduced */
    int speed_divisor = 1;

    /* Is fastforward enabled */
    bool fastforward = false;

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
    unsigned long movie_framecount = 0;

    /* Frame count at the start of the game. Used when game has restarted */
    unsigned long initial_framecount = 0;

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

    /* Are we dumping audio and video? */
    bool av_dumping = false;

    /* Framerate at which the game is running, as a fraction
     * Set to 0 to use the nondeterministic timer
     * In that case, AV dumping is disabled.
     */
    unsigned int framerate_num = 60;
    unsigned int framerate_den = 1;

    /* Are we recording and sending keyboard inputs to the game? */
    bool keyboard_support = true;

    /* Are we recording and sending mouse inputs to the game? */
    bool mouse_support = true;

    /* Number of SDL controllers to (virtually) plug in */
    int nb_controllers = 0;

    /* Log status */
    enum OSDFlags {
        OSD_FRAMECOUNT = 0x01,
        OSD_INPUTS = 0x02,
        OSD_MESSAGES = 0x04,
        OSD_RAMWATCHES = 0x08
    };

    /* Elements to be displayed on the OSD */
    int osd = OSD_FRAMECOUNT | OSD_INPUTS | OSD_MESSAGES | OSD_RAMWATCHES;

    /* Display OSD in the video encode */
    bool osd_encode = false;

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

    /* Use a backup of savefiles in memory, which leaves the original
     * savefiles unmodified and save the content in savestates */
    bool prevent_savefiles = true;

    /* Write back the content of backup savefiles into their file when the game
     * quits. This is necessary if the game is restarted.
     */
    bool write_savefiles_on_exit = false;

    /** Sound config **/
    /* Bit depth of the buffer (usually 8 or 16) */
    int audio_bitdepth = 16;

    /* Number of channels of the buffer */
    int audio_channels = 2;

    /* Frequency of buffer in Hz */
    int audio_frequency = 44100;

    /* Mute audio */
    bool audio_mute = true;

    /* Encode config */
    int video_codec = 0;
    int video_bitrate = 4000;
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
        TIMETYPE_NUMTRACKEDTYPES
    };

    /* Limit for each time-getting method before time auto-advances to
     * avoid a freeze. Distinguish between main and secondary threads.
     */
    int main_gettimes_threshold[TIMETYPE_NUMTRACKEDTYPES] = {-1, -1, -1, -1, -1, -1};
    int sec_gettimes_threshold[TIMETYPE_NUMTRACKEDTYPES] = {-1, -1, -1, -1, -1, -1};

    bool save_screenpixels = true;

    /* Initial system time at game startup */
    struct timespec initial_time = {1, 0};

    /* Virtual monitor resolution */
    int screen_width = 0;
    int screen_height = 0;

    /* Using incremental savestates */
    bool incremental_savestates = true;

    /* Storing savestates in RAM */
    bool savestates_in_ram = false;

    /* Saving a backtrack savestate each time a thread is created/destroyed */
    bool backtrack_savestate = true;

    /* Debug flags */
    enum DebugFlags {
        DEBUG_UNCONTROLLED_TIME = 0x01, // Using undeterministic timer
        DEBUG_NATIVE_EVENTS = 0x02, // Allow game to access real events
    };

    int debug_state = 0;

    bool recycle_threads = true;

    /* An enum indicating which lang are we enforcing */
    enum LocaleType
    {
        LOCALE_ENGLISH,
        LOCALE_JAPANESE,
        LOCALE_KOREAN,
        LOCALE_CHINESE,
        LOCALE_SPANISH,
        LOCALE_GERMAN,
        LOCALE_FRENCH,
        LOCALE_ITALIAN,
        LOCALE_NATIVE
    };

    int locale = LOCALE_ENGLISH;

    /* Simulates a virtual Steam client */
    bool virtual_steam = false;

    /* Force Mesa software OpenGL driver */
    bool opengl_soft = true;

    /* An enum indicating which wait are we doing */
    enum AsyncType
    {
        ASYNC_JSDEV = 0x01,
        ASYNC_EVDEV = 0x02,
        ASYNC_XEVENTS = 0x04,
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
    };

    /* How are we handling waits */
    int wait_timeout = WAIT_NATIVE;

    /* Require vsync or sleep to advance the frame counter */
    bool require_vsync = false;

};

#endif
