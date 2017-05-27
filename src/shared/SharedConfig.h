/*
    Copyright 2015-2016 Clément Gallet <clement.gallet@ens-lyon.org>

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
#ifdef LIBTAS_ENABLE_AVDUMPING
extern "C" {
#include <libavcodec/avcodec.h> // for AVCodecID struct
}
#endif

class SharedConfig {
    public:
        /* Is the game running or on pause */
        bool running = false;

        /* By how much is the speed reduced */
        int speed_divisor = 1;

        /* Is fastforward enabled */
        bool fastforward = false;

        /* Log status */
        enum LogStatus {
            NO_LOGGING,
            LOGGING_TO_CONSOLE,
            LOGGING_TO_FILE
        };

        LogStatus logging_status = LOGGING_TO_CONSOLE;

        /* Which flags trigger a debug message */
        LogCategoryFlag includeFlags = LCF_ALL;

        /* Which flags prevent triggering a debug message */
        LogCategoryFlag excludeFlags = LCF_NONE;

        /* Are we dumping audio and video? */
        bool av_dumping = false;

        /* Framerate at which the game is running.
         * Set to 0 to use the nondeterministic timer
         * In that case, AV dumping is disabled.
         */
        unsigned int framerate = 60;

        /* Are we recording and sending keyboard inputs to the game? */
        bool keyboard_support = true;

        /* Are we recording and sending mouse inputs to the game? */
        bool mouse_support = true;

        /* Number of SDL controllers to (virtually) plug in */
        int numControllers = 0;

        /* Display frame count in the HUD */
        bool hud_framecount = true;

        /* Display inputs in the HUD */
        bool hud_inputs = true;

        /* Display HUD in the video encode */
        bool hud_encode = false;

        /* Prevent the game to write into savefiles */
        bool prevent_savefiles = true;

        /** Sound config **/
        /* Bit depth of the buffer (usually 8 or 16) */
        int audio_bitdepth = 16;

        /* Number of channels of the buffer */
        int audio_channels = 2;

        /* Frequency of buffer in Hz */
        int audio_frequency = 44100;

        /* Mute audio */
        bool audio_mute = false;

#ifdef LIBTAS_ENABLE_AVDUMPING
        /* Encode config */
        AVCodecID video_codec = AV_CODEC_ID_H264;
        int video_bitrate = 4000000;
        AVCodecID audio_codec = AV_CODEC_ID_VORBIS;
        int audio_bitrate = 128000;
#endif

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

};

extern SharedConfig shared_config;

#endif
