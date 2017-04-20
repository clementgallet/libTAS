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
extern "C" {
#include <libavcodec/avcodec.h> // for AVCodecID struct
}

class SharedConfig {
    public:
        SharedConfig();

        /* Is the game running or on pause */
        bool running;

        /* By how much is the speed reduced */
        int speed_divisor;

        /* Is fastforward enabled */
        bool fastforward;

        /* Log status */
        enum LogStatus {
            NO_LOGGING,
            LOGGING_TO_CONSOLE,
            LOGGING_TO_FILE
        };

        LogStatus logging_status;

        /* Which flags trigger a debug message */
        LogCategoryFlag includeFlags;

        /* Which flags prevent triggering a debug message */
        LogCategoryFlag excludeFlags;

        /* Are we dumping audio and video? */
        bool av_dumping;

        /* Framerate at which the game is running.
         * Set to 0 to use the nondeterministic timer
         * In that case, AV dumping is disabled.
         */
        unsigned int framerate;

        /* Are we recording and sending keyboard inputs to the game? */
        bool keyboard_support;

        /* Are we recording and sending mouse inputs to the game? */
        bool mouse_support;

        /* Number of SDL controllers to (virtually) plug in */
        int numControllers;

        /* Display frame count in the HUD */
        bool hud_framecount;

        /* Display inputs in the HUD */
        bool hud_inputs;

        /* Prevent the game to write into savefiles */
        bool prevent_savefiles;

        /** Sound config **/
        /* Bit depth of the buffer (usually 8 or 16) */
        int audio_bitdepth;

        /* Number of channels of the buffer */
        int audio_channels;

        /* Frequency of buffer in Hz */
        int audio_frequency;

        /* Mute audio */
        bool audio_mute;

        /* Encode config */
        AVCodecID video_codec;
        int video_bitrate;
        AVCodecID audio_codec;
        int audio_bitrate;

};

extern SharedConfig shared_config;

#endif
