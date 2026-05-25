/*
    Copyright 2015-2026 Clément Gallet <clement.gallet@ens-lyon.org>

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

#include "AudioPlayerAlsa.h"
#include "AudioContext.h"

#include "logging.h"
#include "global.h" // Global::shared_config
#include "GlobalState.h"

namespace libtas {

snd_pcm_t *AudioPlayerAlsa::phandle;
AudioPlayerAlsa::APStatus AudioPlayerAlsa::status = STATUS_UNINIT;
std::vector<uint8_t> AudioPlayerAlsa::silence;

AudioPlayerAlsa::APStatus AudioPlayerAlsa::init(const AudioContext& ac)
{
    LOG(LL_DEBUG, LCF_SOUND, "Init audio player");

    
    if (!ac.isInited())
        return STATUS_UNINIT;
    
    /* Format parameters */
    snd_pcm_format_t format;
    switch (ac.format) {
        case AudioBuffer::SAMPLE_FMT_U8:
            format = SND_PCM_FORMAT_U8;
            break;
        case AudioBuffer::SAMPLE_FMT_S16:
            format = SND_PCM_FORMAT_S16_LE;
            break;
        case AudioBuffer::SAMPLE_FMT_S32:
            format = SND_PCM_FORMAT_S32_LE;
            break;
        case AudioBuffer::SAMPLE_FMT_FLT:
            format = SND_PCM_FORMAT_FLOAT_LE;
            break;
        case AudioBuffer::SAMPLE_FMT_DBL:
            format = SND_PCM_FORMAT_FLOAT64_LE;
            break;
        default:
            LOG(LL_ERROR, LCF_SOUND, "Unsupported ALSA output for format %d", ac.format);
            return STATUS_ERROR;
    }

    /* Build a 50 ms silence buffer */
    int sil_bytes = static_cast<int>(0.05 * ac.frequency) * ac.bytes_per_sample;

    silence.assign(sil_bytes, AudioBuffer::formatToSilenceByte(ac.format));

    GlobalNative gn;

    if (snd_pcm_open(&phandle, "default", SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK) < 0) {
        LOG(LL_ERROR, LCF_SOUND, "  Cannot open default audio device");
        return STATUS_ERROR;
    }

    snd_pcm_hw_params_t *hw_params;

    if (snd_pcm_hw_params_malloc(&hw_params) < 0) {
        LOG(LL_ERROR, LCF_SOUND, "  snd_pcm_hw_params_malloc failed");
        return STATUS_ERROR;
    }

    if (snd_pcm_hw_params_any(phandle, hw_params) < 0) {
        LOG(LL_ERROR, LCF_SOUND, "  snd_pcm_hw_params_any failed");
        return STATUS_ERROR;
    }

    if (snd_pcm_hw_params_set_access(phandle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED) < 0) {
        LOG(LL_ERROR, LCF_SOUND, "  snd_pcm_hw_params_set_access failed");
        return STATUS_ERROR;
    }

    if (snd_pcm_hw_params_set_format(phandle, hw_params, format) < 0) {
        LOG(LL_ERROR, LCF_SOUND, "  snd_pcm_hw_params_set_format failed");
        return STATUS_ERROR;
    }

    int dir = 0;
    if (snd_pcm_hw_params_set_rate(phandle, hw_params, ac.frequency, dir) < 0) {
        LOG(LL_ERROR, LCF_SOUND, "  snd_pcm_hw_params_set_rate failed");
        return STATUS_ERROR;
    }

    if (snd_pcm_hw_params_set_channels(phandle, hw_params, ac.channels) < 0) {
        LOG(LL_ERROR, LCF_SOUND, "  snd_pcm_hw_params_set_channels failed (%d)", ac.channels);
        return STATUS_ERROR;
    }

    snd_pcm_uframes_t buffer_size = (Global::shared_config.initial_framerate_num>0)?(2*ac.frequency*Global::shared_config.initial_framerate_den/Global::shared_config.initial_framerate_num):(2*ac.frequency/30);
    LOG(LL_DEBUG, LCF_SOUND, "  Buffer size is %d", buffer_size);
    if (snd_pcm_hw_params_set_buffer_size_near(phandle, hw_params, &buffer_size) < 0) {
        LOG(LL_ERROR, LCF_SOUND, "  snd_pcm_hw_params_set_rate_near failed");
        return STATUS_ERROR;
    }
    LOG(LL_DEBUG, LCF_SOUND, "  new buffer size is %d", buffer_size);

    if (snd_pcm_hw_params(phandle, hw_params) < 0) {
        LOG(LL_ERROR, LCF_SOUND, "  snd_pcm_hw_params failed");
        return STATUS_ERROR;
    }

    if (snd_pcm_prepare(phandle) < 0) {
        LOG(LL_ERROR, LCF_SOUND, "  snd_pcm_prepare failed");
        return STATUS_ERROR;
    }

    snd_pcm_hw_params_free(hw_params);

    return STATUS_OK;
}

bool AudioPlayerAlsa::play(const AudioContext& ac)
{
    if (status == STATUS_UNINIT)
        status = init(ac);

    if (status == STATUS_UNINIT)
        return true;

    if (status == STATUS_ERROR)
        return false;

    if (Global::shared_config.fastforward)
        return true;

    LOG(LL_DEBUG, LCF_SOUND, "Play an audio frame");
    int err;
    {
        GlobalNative gn;
        err = snd_pcm_writei(phandle, ac.samples_data.data(), ac.samples_size);
    }
    if (err < 0) {
        if (err == -EPIPE) {
            LOG(LL_DEBUG, LCF_SOUND, "  Underrun");
            {
                GlobalNative gn;
                err = snd_pcm_prepare(phandle);
            }
            if (err < 0) {
                LOG(LL_ERROR, LCF_SOUND, "  Can't recovery from underrun, prepare failed: %s", snd_strerror(err));
                return false;
            }
            else {
                {
                    GlobalNative gn;
                    /* Send silence bytes first */
                    snd_pcm_writei(phandle, silence.data(), silence.size()/ac.bytes_per_sample);
                    snd_pcm_writei(phandle, ac.samples_data.data(), ac.samples_size);
                }
            }
        }
        else if (err == -EAGAIN) {
            LOG(LL_DEBUG, LCF_SOUND, "  Skip frame to catch up");            
            return false;
        }
        else {
            LOG(LL_ERROR, LCF_SOUND, "  snd_pcm_writei() failed: %s", snd_strerror (err));
            return false;
        }
    }

    return true;
}

void AudioPlayerAlsa::close()
{
    if (status == STATUS_OK) {
        MYASSERT(snd_pcm_close(phandle) == 0)
        status = STATUS_UNINIT;
    }
}

}
