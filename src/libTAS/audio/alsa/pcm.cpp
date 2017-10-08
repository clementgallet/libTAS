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

#include "pcm.h"
#include "../../logging.h"
#include "../AudioContext.h"
#include "../AudioSource.h"
#include "../AudioBuffer.h"
#include <time.h> //nanosleep
#include "../../GlobalState.h"
#include "../../hook.h"
//#include "../../threadwrappers.h" // isMainThread()

namespace libtas {

static std::shared_ptr<AudioSource> sourceAlsa;
static int buffer_size = 4096;

DEFINE_ORIG_POINTER(snd_pcm_open);
DEFINE_ORIG_POINTER(snd_pcm_hw_params_sizeof);
DEFINE_ORIG_POINTER(snd_pcm_hw_params_any);
DEFINE_ORIG_POINTER(snd_pcm_hw_params_set_access);
DEFINE_ORIG_POINTER(snd_pcm_hw_params_set_format);
DEFINE_ORIG_POINTER(snd_pcm_hw_params_set_rate);
DEFINE_ORIG_POINTER(snd_pcm_hw_params_set_period_size_near);
DEFINE_ORIG_POINTER(snd_pcm_hw_params_set_buffer_size_near);
DEFINE_ORIG_POINTER(snd_pcm_hw_params_set_channels);
DEFINE_ORIG_POINTER(snd_pcm_hw_params);
DEFINE_ORIG_POINTER(snd_pcm_hw_params_malloc);
DEFINE_ORIG_POINTER(snd_pcm_hw_params_free);
DEFINE_ORIG_POINTER(snd_pcm_prepare);
DEFINE_ORIG_POINTER(snd_pcm_writei);
DEFINE_ORIG_POINTER(snd_pcm_readi);
DEFINE_ORIG_POINTER(snd_pcm_nonblock);
DEFINE_ORIG_POINTER(snd_pcm_close);

int snd_pcm_open(snd_pcm_t **pcm, const char *name, snd_pcm_stream_t stream, int mode)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE(snd_pcm_open, nullptr);
        return orig::snd_pcm_open(pcm, name, stream, mode);
    }

    DEBUGLOGCALL(LCF_SOUND);

    if (stream != SND_PCM_STREAM_PLAYBACK) {
        debuglog(LCF_SOUND | LCF_ERROR, "    Unsupported stream direction ", stream);
        return -1;
    }
    game_info.audio |= GameInfo::ALSA;
    game_info.tosend = true;

    std::lock_guard<std::mutex> lock(audiocontext.mutex);

    /* We create an empty buffer that holds the audio parameters. That way,
     * we can guess the parameters from a previous buffer when adding a new one.
     */
    int bufferId = audiocontext.createBuffer();
    auto buffer = audiocontext.getBuffer(bufferId);

    /* Push buffer in the source */
    int sourceId = audiocontext.createSource();
    sourceAlsa = audiocontext.getSource(sourceId);

    sourceAlsa->buffer_queue.push_back(buffer);
    sourceAlsa->source = AudioSource::SOURCE_STREAMING;

    return 0;
}

int snd_pcm_close(snd_pcm_t *pcm)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE(snd_pcm_close, nullptr);
        return orig::snd_pcm_close(pcm);
    }

    DEBUGLOGCALL(LCF_SOUND);
    return 0;
}

int snd_pcm_nonblock(snd_pcm_t *pcm, int nonblock)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE(snd_pcm_nonblock, nullptr);
        return orig::snd_pcm_nonblock(pcm, nonblock);
    }

    debuglog(LCF_SOUND, __func__, " call with ", nonblock==0?"block":((nonblock==1)?"nonblock":"abort"), " mode");
    return 0;
}

int snd_pcm_hw_params(snd_pcm_t *pcm, snd_pcm_hw_params_t *params)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE(snd_pcm_hw_params, nullptr);
        return orig::snd_pcm_hw_params(pcm, params);
    }

    DEBUGLOGCALL(LCF_SOUND);

    /* Update internal buffer parameters */
    auto buffer = sourceAlsa->buffer_queue[0];
    buffer->size = 0;
    buffer->update();

    /* snd_pcm_hw_params calls snd_pcm_prepare, so we start playing here */
    sourceAlsa->state = AudioSource::SOURCE_PLAYING;

    return 0;
}

int snd_pcm_prepare(snd_pcm_t *pcm)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE(snd_pcm_prepare, nullptr);
        return orig::snd_pcm_prepare(pcm);
    }

    DEBUGLOGCALL(LCF_SOUND);

    return 0;
}

static int get_latency()
{
    std::lock_guard<std::mutex> lock(audiocontext.mutex);
    return sourceAlsa->queueSize() - sourceAlsa->getPosition();
}

snd_pcm_sframes_t snd_pcm_writei(snd_pcm_t *pcm, const void *buffer, snd_pcm_uframes_t size)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE(snd_pcm_writei, nullptr);
        return orig::snd_pcm_writei(pcm, buffer, size);
    }

    debuglog(LCF_SOUND, __func__, " call with ", size, " frames");

    /* Blocking if latency is too high */
    do {
        struct timespec mssleep = {0, 1000*1000};
        NATIVECALL(nanosleep(&mssleep, NULL)); // Wait 10 ms before trying again
    } while (!is_exiting && get_latency() > buffer_size);

    if (is_exiting) return 0;

    std::lock_guard<std::mutex> lock(audiocontext.mutex);

    /* We try to reuse a buffer that has been processed from the source */
    std::shared_ptr<AudioBuffer> ab;
    if (sourceAlsa->nbQueueProcessed() > 0) {
        /* Removing first buffer */
        ab = sourceAlsa->buffer_queue[0];
        sourceAlsa->buffer_queue.erase(sourceAlsa->buffer_queue.begin());
        sourceAlsa->queue_index--;
    }
    else {
        /* Building a new buffer */
        int bufferId = audiocontext.createBuffer();
        ab = audiocontext.getBuffer(bufferId);

        /* Getting the parameters of the buffer from one of the queue */
        if (sourceAlsa->buffer_queue.empty()) {
            debuglog(LCF_SOUND | LCF_ERROR, "Empty queue, cannot guess buffer parameters");
            return -1;
        }

        auto ref = sourceAlsa->buffer_queue[0];
        ab->format = ref->format;
        ab->nbChannels = ref->nbChannels;
        ab->frequency = ref->frequency;
    }

    /* Filling buffer */
    ab->update(); // Compute alignSize
    ab->sampleSize = size;
    ab->size = size * ab->alignSize;
    ab->samples.clear();
    ab->samples.insert(ab->samples.end(), static_cast<const uint8_t*>(buffer), &(static_cast<const uint8_t*>(buffer))[ab->size]);

    sourceAlsa->buffer_queue.push_back(ab);

    return static_cast<snd_pcm_sframes_t>(size);
}

snd_pcm_sframes_t snd_pcm_readi(snd_pcm_t *pcm, void *buffer, snd_pcm_uframes_t size)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE(snd_pcm_readi, nullptr);
        return orig::snd_pcm_readi(pcm, buffer, size);
    }

    debuglog(LCF_SOUND, __func__, " call with ", size, " bytes");
    return static_cast<snd_pcm_sframes_t>(size);
}

int snd_pcm_hw_params_any(snd_pcm_t *pcm, snd_pcm_hw_params_t *params)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE(snd_pcm_hw_params_any, nullptr);
        return orig::snd_pcm_hw_params_any(pcm, params);
    }

    DEBUGLOGCALL(LCF_SOUND);
    return 0;
}

size_t snd_pcm_hw_params_sizeof(void)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE(snd_pcm_hw_params_sizeof, nullptr);
        return orig::snd_pcm_hw_params_sizeof();
    }

    DEBUGLOGCALL(LCF_SOUND);
    return 8;
}

int snd_pcm_hw_params_malloc(snd_pcm_hw_params_t **ptr)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE(snd_pcm_hw_params_malloc, nullptr);
        return orig::snd_pcm_hw_params_malloc(ptr);
    }

    DEBUGLOGCALL(LCF_SOUND);
    *ptr = reinterpret_cast<snd_pcm_hw_params_t*>(1); // Set a non-null value
    return 0;
}

void snd_pcm_hw_params_free(snd_pcm_hw_params_t *obj)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE(snd_pcm_hw_params_free, nullptr);
        return orig::snd_pcm_hw_params_free(obj);
    }

    DEBUGLOGCALL(LCF_SOUND);
}

int snd_pcm_hw_params_set_access(snd_pcm_t *pcm, snd_pcm_hw_params_t *params, snd_pcm_access_t _access)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE(snd_pcm_hw_params_set_access, nullptr);
        return orig::snd_pcm_hw_params_set_access(pcm, params, _access);
    }

    debuglog(LCF_SOUND, __func__, " call with access ", _access);
    if (_access != SND_PCM_ACCESS_RW_INTERLEAVED) {
        debuglog(LCF_SOUND, LCF_ERROR, "    Unsupported access!");
    }
    return 0;
}

int snd_pcm_hw_params_set_format(snd_pcm_t *pcm, snd_pcm_hw_params_t *params, snd_pcm_format_t val)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE(snd_pcm_hw_params_set_format, nullptr);
        return orig::snd_pcm_hw_params_set_format(pcm, params, val);
    }

    debuglog(LCF_SOUND, __func__, " call with format ", val);

    auto buffer = sourceAlsa->buffer_queue[0];

    switch(val) {
        case SND_PCM_FORMAT_U8:
            buffer->format = AudioBuffer::SAMPLE_FMT_U8;
            break;
        case SND_PCM_FORMAT_S16_LE:
            buffer->format = AudioBuffer::SAMPLE_FMT_S16;
            break;
        case SND_PCM_FORMAT_S32_LE:
            buffer->format = AudioBuffer::SAMPLE_FMT_S32;
            break;
        case SND_PCM_FORMAT_FLOAT_LE:
            buffer->format = AudioBuffer::SAMPLE_FMT_FLT;
            break;
        default:
            debuglog(LCF_SOUND | LCF_ERROR, "    Unsupported audio format");
            return -1;
    }

    return 0;
}

int snd_pcm_hw_params_set_channels(snd_pcm_t *pcm, snd_pcm_hw_params_t *params, unsigned int val)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE(snd_pcm_hw_params_set_channels, nullptr);
        return orig::snd_pcm_hw_params_set_channels(pcm, params, val);
    }

    debuglog(LCF_SOUND, __func__, " call with channels ", val);

    auto buffer = sourceAlsa->buffer_queue[0];
    buffer->nbChannels = val;

    return 0;
}

int snd_pcm_hw_params_set_rate(snd_pcm_t *pcm, snd_pcm_hw_params_t *params, unsigned int val, int dir)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE(snd_pcm_hw_params_set_rate, nullptr);
        return orig::snd_pcm_hw_params_set_rate(pcm, params, val, dir);
    }

    debuglog(LCF_SOUND, __func__, " call with rate ", val, " and dir ", dir);

    auto buffer = sourceAlsa->buffer_queue[0];
    buffer->frequency = val;

    return 0;
}

int snd_pcm_hw_params_set_period_size_near(snd_pcm_t *pcm, snd_pcm_hw_params_t *params, snd_pcm_uframes_t *val, int *dir)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE(snd_pcm_hw_params_set_period_size_near, nullptr);
        return orig::snd_pcm_hw_params_set_period_size_near(pcm, params, val, dir);
    }

    debuglog(LCF_SOUND, __func__, " call with period size ", *val, " and dir ", dir?*dir:-2);
    return 0;
}

int snd_pcm_hw_params_set_buffer_size_near(snd_pcm_t *pcm, snd_pcm_hw_params_t *params, snd_pcm_uframes_t *val)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE(snd_pcm_hw_params_set_buffer_size_near, nullptr);
        return orig::snd_pcm_hw_params_set_buffer_size_near(pcm, params, val);
    }

    debuglog(LCF_SOUND, __func__, " call with buffer size ", *val);
    buffer_size = *val;
    return 0;
}

}
