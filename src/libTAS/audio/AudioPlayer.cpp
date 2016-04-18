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
#include "../ThreadState.h"

AudioPlayer audioplayer;

AudioPlayer::AudioPlayer(void)
{
    if (snd_pcm_open(&phandle, "default", SND_PCM_STREAM_PLAYBACK, 0) < 0) {
        debuglog(LCF_SOUND, "  Cannot open default audio device");
    }

    inited = false;
}

AudioPlayer::~AudioPlayer(void)
{
    snd_pcm_close(phandle);
}

bool AudioPlayer::init(snd_pcm_format_t format, int nbChannels, unsigned int frequency)
{
    debuglog(LCF_SOUND, "Init audio player");

    snd_pcm_hw_params_t *hw_params;

    if (snd_pcm_hw_params_malloc(&hw_params) < 0) {
        debuglog(LCF_SOUND | LCF_ERROR, "  snd_pcm_hw_params_malloc failed");
        return false;
    }

    if (snd_pcm_hw_params_any(phandle, hw_params) < 0) {
        debuglog(LCF_SOUND | LCF_ERROR, "  snd_pcm_hw_params_any failed");
        return false;
    }

    if (snd_pcm_hw_params_set_access(phandle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED) < 0) {
        debuglog(LCF_SOUND | LCF_ERROR, "  snd_pcm_hw_params_set_access failed");
        return false;
    }

    if (snd_pcm_hw_params_set_format(phandle, hw_params, format) < 0) {
        debuglog(LCF_SOUND | LCF_ERROR, "  snd_pcm_hw_params_set_format failed");
        return false;
    }

    int dir;
    if (snd_pcm_hw_params_set_rate_near(phandle, hw_params, &frequency, &dir) < 0) {
        debuglog(LCF_SOUND | LCF_ERROR, "  snd_pcm_hw_params_set_rate_near failed");
        return false;
    }

    snd_pcm_uframes_t buffer_size = 2 * frequency / ((tasflags.framerate>0)?tasflags.framerate:30);
    debuglog(LCF_SOUND, "  Buffer size is ", buffer_size);
    if (snd_pcm_hw_params_set_buffer_size_near(phandle, hw_params, &buffer_size) < 0) {
        debuglog(LCF_SOUND | LCF_ERROR, "  snd_pcm_hw_params_set_rate_near failed");
        return false;
    }

    if (snd_pcm_hw_params_set_channels(phandle, hw_params, nbChannels) < 0) {
        debuglog(LCF_SOUND | LCF_ERROR, "  snd_pcm_hw_params_set_channels failed");
        return false;
    }

    if (snd_pcm_hw_params(phandle, hw_params) < 0) {
        debuglog(LCF_SOUND | LCF_ERROR, "  snd_pcm_hw_params failed");
        return false;
    }

    snd_pcm_hw_params_free(hw_params);

    if (snd_pcm_prepare(phandle) < 0) {
        debuglog(LCF_SOUND | LCF_ERROR, "  snd_pcm_prepare failed");
        return false;
    }

    return true;
}

bool AudioPlayer::play(AudioContext& ac)
{
    if (!inited) {
        snd_pcm_format_t format;
        if (ac.outBitDepth == 8)
            format = SND_PCM_FORMAT_U8;
        if (ac.outBitDepth == 16)
            format = SND_PCM_FORMAT_S16_LE;
        if (!init(format, ac.outNbChannels, (unsigned int)ac.outFrequency))
            return false;
        inited = true;
    }

    if (tasflags.fastforward)
        return true;

    debuglog(LCF_SOUND, "Play an audio frame");
    threadState.setOwnCode(true);
    int err = snd_pcm_writei(phandle, ac.outSamples.data(), ac.outNbSamples);
    threadState.setOwnCode(false);
	if (err < 0) {
		if (err == -EPIPE) {
			debuglog(LCF_SOUND, "  Underrun");
            threadState.setOwnCode(true);
			err = snd_pcm_prepare(phandle);
            threadState.setOwnCode(false);
			if (err < 0) {
				debuglog(LCF_SOUND | LCF_ERROR, "  Can't recovery from underrun, prepare failed: ", snd_strerror(err));
			    return false;
            }
            else {
                threadState.setOwnCode(true);
                snd_pcm_writei(phandle, ac.outSamples.data(), ac.outNbSamples);
                threadState.setOwnCode(false);
            }
		}
		else {
			debuglog(LCF_SOUND | LCF_ERROR, "  snd_pcm_writei() failed: ", snd_strerror (err));
			return false;
		}
	}

    return true;
}

#endif

