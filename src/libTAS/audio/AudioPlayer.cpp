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

#include "AudioPlayer.h"

#ifdef LIBTAS_ENABLE_SOUNDPLAYBACK

#include "../logging.h"
#include "../../shared/tasflags.h"

AudioPlayer audioplayer;

AudioPlayer::AudioPlayer(void)
{
    inited = false;
}

bool AudioPlayer::init(pa_sample_format_t format, int nbChannels, int frequency)
{
    debuglog(LCF_SOUND, "Init audio player");
    pa_sample_spec pa_ss;

    pa_ss.format = format;
    pa_ss.channels = nbChannels;
    pa_ss.rate = frequency;

    pa_s = pa_simple_new(NULL,  // Use the default server.
            "TAS game",         // Our application's name.
            PA_STREAM_PLAYBACK,
            NULL,               // Use the default device.
            "Audio",            // Description of our stream.
            &pa_ss,             // Our sample format.
            NULL,               // Use default channel map
            NULL,               // Use default buffering attributes.
            NULL                // Ignore error code.
            );

    if (pa_s == nullptr) {
        debuglog(LCF_SOUND | LCF_ERROR, "pa_simple_new() failed");
        return false;
    }

    return true;
}

bool AudioPlayer::play(AudioContext& ac)
{
    debuglog(LCF_SOUND, "Play an audio frame");
    if (!inited) {
        pa_sample_format_t format;
        if (ac.outBitDepth == 8)
            format = PA_SAMPLE_U8;
        if (ac.outBitDepth == 16)
            format = PA_SAMPLE_S16LE;
        if (!init(format, ac.outNbChannels, ac.outFrequency))
            return false;
        inited = true;
    }

    if (tasflags.fastforward)
        return true;

    if (pa_simple_write(pa_s, (void*)&ac.outSamples[0], ac.outBytes, NULL) < 0) {
        debuglog(LCF_SOUND | LCF_ERROR, "pa_simple_write() failed");
        return false;
    }

    return true;
}

#endif

