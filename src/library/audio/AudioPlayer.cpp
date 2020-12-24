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

#include "AudioPlayer.h"

#include "../logging.h"
#include "../global.h" // shared_config
#include "../GlobalState.h"
//#include "../hook.h"

namespace libtas {

snd_pcm_t *AudioPlayer::phandle;
int AudioPlayer::inited = 0;
std::vector<char> AudioPlayer::silence;

bool AudioPlayer::init(snd_pcm_format_t format, int nbChannels, unsigned int frequency)
{
    debuglogstdio(LCF_SOUND, "Init audio player");

    GlobalNative gn;

    if (snd_pcm_open(&phandle, "default", SND_PCM_STREAM_PLAYBACK, 0) < 0) {
        debuglogstdio(LCF_SOUND | LCF_ERROR, "  Cannot open default audio device");
        return false;
    }

    snd_pcm_hw_params_t *hw_params;

    if (snd_pcm_hw_params_malloc(&hw_params) < 0) {
        debuglogstdio(LCF_SOUND | LCF_ERROR, "  snd_pcm_hw_params_malloc failed");
        return false;
    }

    if (snd_pcm_hw_params_any(phandle, hw_params) < 0) {
        debuglogstdio(LCF_SOUND | LCF_ERROR, "  snd_pcm_hw_params_any failed");
        return false;
    }

    if (snd_pcm_hw_params_set_access(phandle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED) < 0) {
        debuglogstdio(LCF_SOUND | LCF_ERROR, "  snd_pcm_hw_params_set_access failed");
        return false;
    }

    if (snd_pcm_hw_params_set_format(phandle, hw_params, format) < 0) {
        debuglogstdio(LCF_SOUND | LCF_ERROR, "  snd_pcm_hw_params_set_format failed");
        return false;
    }

    int dir = 0;
    if (snd_pcm_hw_params_set_rate(phandle, hw_params, frequency, dir) < 0) {
        debuglogstdio(LCF_SOUND | LCF_ERROR, "  snd_pcm_hw_params_set_rate failed");
        return false;
    }

    if (snd_pcm_hw_params_set_channels(phandle, hw_params, nbChannels) < 0) {
        debuglogstdio(LCF_SOUND | LCF_ERROR, "  snd_pcm_hw_params_set_channels failed (%d)", nbChannels);
        return false;
    }

    snd_pcm_uframes_t buffer_size = (shared_config.framerate_num>0)?(2*frequency*shared_config.framerate_den/shared_config.framerate_num):(2*frequency/30);
    debuglogstdio(LCF_SOUND, "  Buffer size is %d", buffer_size);
    if (snd_pcm_hw_params_set_buffer_size_near(phandle, hw_params, &buffer_size) < 0) {
        debuglogstdio(LCF_SOUND | LCF_ERROR, "  snd_pcm_hw_params_set_rate_near failed");
        return false;
    }
    debuglogstdio(LCF_SOUND, "  new buffer size is %d", buffer_size);

    if (snd_pcm_hw_params(phandle, hw_params) < 0) {
        debuglogstdio(LCF_SOUND | LCF_ERROR, "  snd_pcm_hw_params failed");
        return false;
    }

    if (snd_pcm_prepare(phandle) < 0) {
        debuglogstdio(LCF_SOUND | LCF_ERROR, "  snd_pcm_prepare failed");
        return false;
    }

    snd_pcm_hw_params_free(hw_params);

    return true;
}

bool AudioPlayer::play(AudioContext& ac)
{
    if (inited == 0) {
        snd_pcm_format_t format;
        if (ac.outBitDepth == 8)
            format = SND_PCM_FORMAT_U8;
        if (ac.outBitDepth == 16)
            format = SND_PCM_FORMAT_S16_LE;
        if (!init(format, ac.outNbChannels, static_cast<unsigned int>(ac.outFrequency))) {
            inited = -1;
            return false;
        }

        /* Build a 50 ms silence buffer */
        int sil_bytes = static_cast<int>(0.05 * ac.outFrequency) * ac.outAlignSize;

        if (ac.outBitDepth == 8) {
            silence.assign(sil_bytes, -128);
        }
        if (ac.outBitDepth == 16) {
            silence.assign(sil_bytes, 0x00);
        }
        
        inited = 1;
    }

    if (inited == -1)
        return false;

    if (shared_config.fastforward)
        return true;

    debuglogstdio(LCF_SOUND, "Play an audio frame");
    int err;
    {
        GlobalNative gn;
        err = snd_pcm_writei(phandle, ac.outSamples.data(), ac.outNbSamples);
    }
	if (err < 0) {
		if (err == -EPIPE) {
			debuglogstdio(LCF_SOUND, "  Underrun");
            {
                GlobalNative gn;
	            err = snd_pcm_prepare(phandle);
            }
			if (err < 0) {
				debuglogstdio(LCF_SOUND | LCF_ERROR, "  Can't recovery from underrun, prepare failed: %s", snd_strerror(err));
			    return false;
            }
            else {
                {
                    GlobalNative gn;
                    /* Send silence bytes first */
                    snd_pcm_writei(phandle, silence.data(), silence.size()/ac.outAlignSize);
                    snd_pcm_writei(phandle, ac.outSamples.data(), ac.outNbSamples);
                }
            }
		}
		else {
			debuglogstdio(LCF_SOUND | LCF_ERROR, "  snd_pcm_writei() failed: %s", snd_strerror (err));
			return false;
		}
	}

    return true;
}

void AudioPlayer::close()
{
    if (inited == 1) {
        MYASSERT(snd_pcm_close(phandle) == 0)
        inited = 0;
    }
}

}
