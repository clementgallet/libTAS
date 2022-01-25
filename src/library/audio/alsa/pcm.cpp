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

#include "pcm.h"
#include "../../logging.h"
#include "../AudioContext.h"
#include "../AudioSource.h"
#include "../AudioBuffer.h"
#include <time.h> //nanosleep
#include "../../GlobalState.h"
#include "../../hook.h"
#include "../../DeterministicTimer.h"
#include "../../checkpoint/ThreadManager.h"

#include <stdint.h>

#define BUFFER_SIZE_MIN 2048

namespace libtas {

static int buffer_size = 4096; // in samples

DEFINE_ORIG_POINTER(snd_pcm_open)
DEFINE_ORIG_POINTER(snd_pcm_open_lconf)
DEFINE_ORIG_POINTER(snd_pcm_open_fallback)

DEFINE_ORIG_POINTER(snd_pcm_poll_descriptors_count)
DEFINE_ORIG_POINTER(snd_pcm_poll_descriptors)
DEFINE_ORIG_POINTER(snd_pcm_poll_descriptors_revents)

DEFINE_ORIG_POINTER(snd_pcm_info)
DEFINE_ORIG_POINTER(snd_pcm_sw_params_current)
DEFINE_ORIG_POINTER(snd_pcm_sw_params)
DEFINE_ORIG_POINTER(snd_pcm_hw_params_sizeof)
DEFINE_ORIG_POINTER(snd_pcm_hw_params_any)
DEFINE_ORIG_POINTER(snd_pcm_hw_params_current)
DEFINE_ORIG_POINTER(snd_pcm_hw_params_set_access)
DEFINE_ORIG_POINTER(snd_pcm_hw_params_get_access)
DEFINE_ORIG_POINTER(snd_pcm_hw_params_set_format)
DEFINE_ORIG_POINTER(snd_pcm_hw_params_get_format_mask)

DEFINE_ORIG_POINTER(snd_pcm_hw_params_set_rate)
DEFINE_ORIG_POINTER(snd_pcm_hw_params_set_rate_near)
DEFINE_ORIG_POINTER(snd_pcm_hw_params_set_rate_resample)
DEFINE_ORIG_POINTER(snd_pcm_hw_params_get_rate)
DEFINE_ORIG_POINTER(snd_pcm_hw_params_get_rate_min)
DEFINE_ORIG_POINTER(snd_pcm_hw_params_get_rate_max)

DEFINE_ORIG_POINTER(snd_pcm_hw_params_get_period_size)
DEFINE_ORIG_POINTER(snd_pcm_hw_params_get_period_time_min)
DEFINE_ORIG_POINTER(snd_pcm_hw_params_set_period_time_near)
DEFINE_ORIG_POINTER(snd_pcm_hw_params_set_period_size_near)
DEFINE_ORIG_POINTER(snd_pcm_hw_params_set_periods_near)
DEFINE_ORIG_POINTER(snd_pcm_hw_params_get_periods)

DEFINE_ORIG_POINTER(snd_pcm_hw_params_get_buffer_size)
DEFINE_ORIG_POINTER(snd_pcm_hw_params_get_buffer_size_min)
DEFINE_ORIG_POINTER(snd_pcm_hw_params_get_buffer_size_max)
DEFINE_ORIG_POINTER(snd_pcm_hw_params_get_buffer_time_max)
DEFINE_ORIG_POINTER(snd_pcm_hw_params_set_buffer_size_near)
DEFINE_ORIG_POINTER(snd_pcm_hw_params_set_buffer_time_near)

DEFINE_ORIG_POINTER(snd_pcm_hw_params_get_channels)
DEFINE_ORIG_POINTER(snd_pcm_hw_params_get_channels_min)
DEFINE_ORIG_POINTER(snd_pcm_hw_params_get_channels_max)
DEFINE_ORIG_POINTER(snd_pcm_hw_params_set_channels)
DEFINE_ORIG_POINTER(snd_pcm_hw_params)
DEFINE_ORIG_POINTER(snd_pcm_hw_params_malloc)
DEFINE_ORIG_POINTER(snd_pcm_hw_params_free)
DEFINE_ORIG_POINTER(snd_pcm_hw_params_copy)
DEFINE_ORIG_POINTER(snd_pcm_get_params)
DEFINE_ORIG_POINTER(snd_pcm_set_params)

DEFINE_ORIG_POINTER(snd_pcm_prepare)
DEFINE_ORIG_POINTER(snd_pcm_writei)
DEFINE_ORIG_POINTER(snd_pcm_readi)
DEFINE_ORIG_POINTER(snd_pcm_nonblock)
DEFINE_ORIG_POINTER(snd_pcm_close)
DEFINE_ORIG_POINTER(snd_pcm_recover)
DEFINE_ORIG_POINTER(snd_pcm_reset)
DEFINE_ORIG_POINTER(snd_pcm_status)

DEFINE_ORIG_POINTER(snd_pcm_mmap_begin)
DEFINE_ORIG_POINTER(snd_pcm_mmap_commit)

DEFINE_ORIG_POINTER(snd_pcm_start)
DEFINE_ORIG_POINTER(snd_pcm_drop)
DEFINE_ORIG_POINTER(snd_pcm_state)
DEFINE_ORIG_POINTER(snd_pcm_hw_params_can_pause)
DEFINE_ORIG_POINTER(snd_pcm_pause)
DEFINE_ORIG_POINTER(snd_pcm_resume)
DEFINE_ORIG_POINTER(snd_pcm_wait)
DEFINE_ORIG_POINTER(snd_pcm_delay)
DEFINE_ORIG_POINTER(snd_pcm_avail)
DEFINE_ORIG_POINTER(snd_pcm_avail_update)
DEFINE_ORIG_POINTER(snd_pcm_rewind)
DEFINE_ORIG_POINTER(snd_pcm_hw_params_test_rate)
DEFINE_ORIG_POINTER(snd_pcm_hw_params_test_format)
DEFINE_ORIG_POINTER(snd_pcm_hw_params_test_channels)

DEFINE_ORIG_POINTER(snd_pcm_sw_params_sizeof)
DEFINE_ORIG_POINTER(snd_pcm_sw_params_set_start_threshold)
DEFINE_ORIG_POINTER(snd_pcm_sw_params_set_stop_threshold)
DEFINE_ORIG_POINTER(snd_pcm_sw_params_set_avail_min)

DEFINE_ORIG_POINTER(snd_pcm_get_chmap)

DEFINE_ORIG_POINTER(snd_pcm_format_mask_malloc)
DEFINE_ORIG_POINTER(snd_pcm_format_mask_free)
DEFINE_ORIG_POINTER(snd_pcm_format_mask_test)

DEFINE_ORIG_POINTER(snd_pcm_bytes_to_frames)
DEFINE_ORIG_POINTER(snd_pcm_frames_to_bytes)

static int get_latency(snd_pcm_t *pcm);

static int last_source;

static bool block_mode = true;

int snd_pcm_open(snd_pcm_t **pcm, const char *name, snd_pcm_stream_t stream, int mode)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE_GLOBAL(snd_pcm_open);
        return orig::snd_pcm_open(pcm, name, stream, mode);
    }

    DEBUGLOGCALL(LCF_SOUND);

    if (shared_config.audio_disabled)
        return -1;

    if (stream != SND_PCM_STREAM_PLAYBACK) {
        debuglogstdio(LCF_SOUND | LCF_WARNING, "    Unsupported stream direction %d", stream);
        return -1;
    }

    if (mode == SND_PCM_NONBLOCK) {
        block_mode = false;
    }
    else {
        block_mode = true;
    }

    if (!(game_info.audio & GameInfo::ALSA)) {
        game_info.audio |= GameInfo::ALSA;
        game_info.tosend = true;
    }

    std::lock_guard<std::mutex> lock(audiocontext.mutex);

    /* We create an empty buffer that holds the audio parameters. That way,
     * we can guess the parameters from a previous buffer when adding a new one.
     */
    int bufferId = audiocontext.createBuffer();
    auto buffer = audiocontext.getBuffer(bufferId);

    /* Create a source and push buffer in the source */
    int sourceId = audiocontext.createSource();
    auto source = audiocontext.getSource(sourceId);

    source->buffer_queue.push_back(buffer);
    source->source = AudioSource::SOURCE_STREAMING_CONTINUOUS;

    /* Fill the source id in pcm, so that you can locate the source on future calls */
    *pcm = reinterpret_cast<snd_pcm_t*>(sourceId);
    last_source = sourceId;

    return 0;
}

int snd_pcm_open_lconf(snd_pcm_t **pcm, const char *name,
    		       snd_pcm_stream_t stream, int mode,
    		       snd_config_t *lconf)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE_GLOBAL(snd_pcm_open_lconf);
        return orig::snd_pcm_open_lconf(pcm, name, stream, mode, lconf);
    }

    DEBUGLOGCALL(LCF_SOUND);
    return snd_pcm_open(pcm, name, stream, mode);
}

int snd_pcm_open_fallback(snd_pcm_t **pcm, snd_config_t *root,
    			  const char *name, const char *orig_name,
    			  snd_pcm_stream_t stream, int mode)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE_GLOBAL(snd_pcm_open_fallback);
        return orig::snd_pcm_open_fallback(pcm, root, name, orig_name, stream, mode);
    }

    DEBUGLOGCALL(LCF_SOUND);
    return snd_pcm_open(pcm, name, stream, mode);
}

int snd_pcm_close(snd_pcm_t *pcm)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE_GLOBAL(snd_pcm_close);
        return orig::snd_pcm_close(pcm);
    }

    DEBUGLOGCALL(LCF_SOUND);
    std::lock_guard<std::mutex> lock(audiocontext.mutex);

    /* Delete source buffers and source */
    int sourceId = reinterpret_cast<intptr_t>(pcm);
    auto source = audiocontext.getSource(sourceId);

    if (source) {
        for (auto& buffer : source->buffer_queue)
            audiocontext.deleteBuffer(buffer->id);

        audiocontext.deleteSource(sourceId);
    }

    return 0;
}

int snd_pcm_poll_descriptors_count(snd_pcm_t *pcm)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE_GLOBAL(snd_pcm_poll_descriptors_count);
        return orig::snd_pcm_poll_descriptors_count(pcm);
    }

    DEBUGLOGCALL(LCF_SOUND);
    return 1;
}

int snd_pcm_poll_descriptors(snd_pcm_t *pcm, struct pollfd *pfds, unsigned int space)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE_GLOBAL(snd_pcm_poll_descriptors);
        return orig::snd_pcm_poll_descriptors(pcm, pfds, space);
    }

    DEBUGLOGCALL(LCF_SOUND);

    if (pfds) {
        /* Use a magic number to identify the fake ALSA fd, and use the last
         * field to store the source */
        pfds[0] = {0xa15a, POLLIN, static_cast<short int>(reinterpret_cast<intptr_t>(pcm))};
        return 1;
    }

    return 0;
}

int snd_pcm_poll_descriptors_revents(snd_pcm_t *pcm, struct pollfd *pfds, unsigned int nfds, unsigned short *revents)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE_GLOBAL(snd_pcm_poll_descriptors_revents);
        return orig::snd_pcm_poll_descriptors_revents(pcm, pfds, nfds, revents);
    }

    DEBUGLOGCALL(LCF_SOUND);

    /* We don't handle audio capture anyway, so there's only one possible value */
    if (revents)
        *revents = POLLOUT;

    return 0;
}

int snd_pcm_info(snd_pcm_t *pcm, snd_pcm_info_t *info)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE_GLOBAL(snd_pcm_info);
        return orig::snd_pcm_info(pcm, info);
    }

    DEBUGLOGCALL(LCF_SOUND);
    return 0;
}

int snd_pcm_nonblock(snd_pcm_t *pcm, int nonblock)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE_GLOBAL(snd_pcm_nonblock);
        return orig::snd_pcm_nonblock(pcm, nonblock);
    }

    debuglogstdio(LCF_SOUND, "%s call with %s mode", __func__, nonblock==0?"block":((nonblock==1)?"nonblock":"abort"));
    if (nonblock == 0) {
        block_mode = true;
    }
    else if (nonblock == 1) {
        block_mode = false;
    }

    return 0;
}

int snd_pcm_start(snd_pcm_t *pcm)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE_GLOBAL(snd_pcm_start);
        return orig::snd_pcm_start(pcm);
    }

    DEBUGLOGCALL(LCF_SOUND);
    int sourceId = reinterpret_cast<intptr_t>(pcm);
    auto source = audiocontext.getSource(sourceId);
    source->state = AudioSource::SOURCE_PLAYING;

    return 0;
}

int snd_pcm_drop(snd_pcm_t *pcm)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE_GLOBAL(snd_pcm_drop);
        return orig::snd_pcm_drop(pcm);
    }

    DEBUGLOGCALL(LCF_SOUND);
    int sourceId = reinterpret_cast<intptr_t>(pcm);
    auto source = audiocontext.getSource(sourceId);
    source->setPosition(source->queueSize());
    return 0;
}

int snd_pcm_hw_params_can_pause(const snd_pcm_hw_params_t *params)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE_GLOBAL(snd_pcm_hw_params_can_pause);
        return orig::snd_pcm_hw_params_can_pause(params);
    }

    DEBUGLOGCALL(LCF_SOUND);
    return 1;
}

int snd_pcm_pause(snd_pcm_t *pcm, int enable)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE_GLOBAL(snd_pcm_pause);
        return orig::snd_pcm_pause(pcm, enable);
    }

    DEBUGLOGCALL(LCF_SOUND);

    int sourceId = reinterpret_cast<intptr_t>(pcm);
    auto source = audiocontext.getSource(sourceId);
    if (enable)
        source->state = AudioSource::SOURCE_PAUSED;
    else
        source->state = AudioSource::SOURCE_PLAYING;

    return 0;
}

snd_pcm_state_t snd_pcm_state(snd_pcm_t *pcm)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE_GLOBAL(snd_pcm_state);
        return orig::snd_pcm_state(pcm);
    }

    DEBUGLOGCALL(LCF_SOUND);

    int sourceId = reinterpret_cast<intptr_t>(pcm);
    auto source = audiocontext.getSource(sourceId);
    switch (source->state) {
        case AudioSource::SOURCE_INITIAL:
            return SND_PCM_STATE_OPEN;
        case AudioSource::SOURCE_PREPARED:
            return SND_PCM_STATE_PREPARED;
        case AudioSource::SOURCE_PLAYING:
            return SND_PCM_STATE_RUNNING;
        case AudioSource::SOURCE_PAUSED:
            return SND_PCM_STATE_PAUSED;
        case AudioSource::SOURCE_STOPPED:
            return SND_PCM_STATE_XRUN;
        case AudioSource::SOURCE_UNDERRUN:
            return SND_PCM_STATE_XRUN;
    }

    return SND_PCM_STATE_OPEN;
}

int snd_pcm_resume(snd_pcm_t *pcm)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE_GLOBAL(snd_pcm_resume);
        return orig::snd_pcm_resume(pcm);
    }

    DEBUGLOGCALL(LCF_SOUND);
    int sourceId = reinterpret_cast<intptr_t>(pcm);
    auto source = audiocontext.getSource(sourceId);
    source->state = AudioSource::SOURCE_PLAYING;

    return 0;
}

int snd_pcm_wait(snd_pcm_t *pcm, int timeout)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE_GLOBAL(snd_pcm_wait);
        return orig::snd_pcm_wait(pcm, timeout);
    }
    debuglogstdio(LCF_SOUND, "%s called with timeout %d", __func__, timeout);

    /* Check for no more available samples */
    int delta_ms = -1;
    if ((buffer_size - get_latency(pcm)) <= 0) {
        /* Wait for timeout or available samples */
        TimeHolder initial_time = detTimer.getTicks();
        do {
            struct timespec mssleep = {0, 1000*1000};
            NATIVECALL(nanosleep(&mssleep, NULL)); // Wait 1 ms before trying again
            TimeHolder delta_time = detTimer.getTicks();
            delta_time -= initial_time;
            delta_ms = delta_time.tv_sec * 1000 + delta_time.tv_nsec / 1000000;
        } while (!is_exiting && (get_latency(pcm) >= buffer_size) && ((timeout < 0) || (delta_ms < timeout)));
    }

    if ((buffer_size - get_latency(pcm)) > 0)
        return 1;

    /* Timeout */
    return 0;
}

int snd_pcm_delay(snd_pcm_t *pcm, snd_pcm_sframes_t *delayp)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE_GLOBAL(snd_pcm_delay);
        return orig::snd_pcm_delay(pcm, delayp);
    }

    DEBUGLOGCALL(LCF_SOUND);
    *delayp = get_latency(pcm);
    debuglogstdio(LCF_SOUND, "   return %d", *delayp);
    return 0;
}

snd_pcm_sframes_t snd_pcm_avail(snd_pcm_t *pcm)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE_GLOBAL(snd_pcm_avail);
        return orig::snd_pcm_avail(pcm);
    }

    DEBUGLOGCALL(LCF_SOUND);
    snd_pcm_sframes_t avail = buffer_size - get_latency(pcm);
    if (avail<0)
        avail = 0;
    debuglogstdio(LCF_SOUND, "   return %d", avail);
    return avail;
}

snd_pcm_sframes_t snd_pcm_avail_update(snd_pcm_t *pcm)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE_GLOBAL(snd_pcm_avail_update);
        return orig::snd_pcm_avail_update(pcm);
    }

    DEBUGLOGCALL(LCF_SOUND);
    snd_pcm_sframes_t avail = buffer_size - get_latency(pcm);
    if (avail<0)
        avail = 0;
    debuglogstdio(LCF_SOUND, "   return %d", avail);
    return avail;
}

snd_pcm_sframes_t snd_pcm_rewind(snd_pcm_t *pcm, snd_pcm_uframes_t frames)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE_GLOBAL(snd_pcm_rewind);
        return orig::snd_pcm_rewind(pcm, frames);
    }

    DEBUGLOGCALL(LCF_SOUND);
    int sourceId = reinterpret_cast<intptr_t>(pcm);
    auto source = audiocontext.getSource(sourceId);
    snd_pcm_uframes_t pos = source->getPosition();
    if (frames <= pos) {
        source->setPosition(pos - frames);
        return 0;
    }
    return -1;
}

int snd_pcm_recover(snd_pcm_t *pcm, int err, int silent)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE_GLOBAL(snd_pcm_recover);
        return orig::snd_pcm_recover(pcm, err, silent);
    }

    DEBUGLOGCALL(LCF_SOUND);

    if (err == -EPIPE) {
        int sourceId = reinterpret_cast<intptr_t>(pcm);
        auto source = audiocontext.getSource(sourceId);

        if (source->state == AudioSource::SOURCE_UNDERRUN)
            source->state = AudioSource::SOURCE_PREPARED;
        
        return 0;
    }
    
    return err;
}

int snd_pcm_reset(snd_pcm_t *pcm)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE_GLOBAL(snd_pcm_reset);
        return orig::snd_pcm_reset(pcm);
    }

    int sourceId = reinterpret_cast<intptr_t>(pcm);
    auto source = audiocontext.getSource(sourceId);
    source->setPosition(source->queueSize());
    DEBUGLOGCALL(LCF_SOUND);
    return 0;
}

int snd_pcm_status(snd_pcm_t *pcm, snd_pcm_status_t *status)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE_GLOBAL(snd_pcm_status);
        return orig::snd_pcm_status(pcm, status);
    }

    // int sourceId = reinterpret_cast<intptr_t>(pcm);
    // auto source = audiocontext.getSource(sourceId);
    // source->setPosition(source->queueSize());
    DEBUGLOGCALL(LCF_SOUND);
    return 0;
}

int snd_pcm_hw_params(snd_pcm_t *pcm, snd_pcm_hw_params_t *params)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE_GLOBAL(snd_pcm_hw_params);
        return orig::snd_pcm_hw_params(pcm, params);
    }

    DEBUGLOGCALL(LCF_SOUND);

    /* Update internal buffer parameters */
    int sourceId = reinterpret_cast<intptr_t>(pcm);
    auto source = audiocontext.getSource(sourceId);
    auto buffer = source->buffer_queue[0];
    buffer->size = 0;
    buffer->update();

    /* snd_pcm_hw_params calls snd_pcm_prepare, so we start playing here */
    source->state = AudioSource::SOURCE_PREPARED;

    return 0;
}

int snd_pcm_hw_params_current(snd_pcm_t *pcm, snd_pcm_hw_params_t *params)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE_GLOBAL(snd_pcm_hw_params_current);
        return orig::snd_pcm_hw_params_current(pcm, params);
    }

    DEBUGLOGCALL(LCF_SOUND);
    return 0;
}

int snd_pcm_sw_params_current(snd_pcm_t *pcm, snd_pcm_sw_params_t *params)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE_GLOBAL(snd_pcm_sw_params_current);
        return orig::snd_pcm_sw_params_current(pcm, params);
    }

    DEBUGLOGCALL(LCF_SOUND);
    return 0;
}

int snd_pcm_sw_params(snd_pcm_t *pcm, snd_pcm_sw_params_t *params)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE_GLOBAL(snd_pcm_sw_params);
        return orig::snd_pcm_sw_params(pcm, params);
    }

    DEBUGLOGCALL(LCF_SOUND);

    /* Update internal buffer parameters */
    int sourceId = reinterpret_cast<intptr_t>(pcm);
    auto source = audiocontext.getSource(sourceId);
    auto buffer = source->buffer_queue[0];
    buffer->size = 0;
    buffer->update();

    /* snd_pcm_sw_params calls snd_pcm_prepare, so we start playing here */
    source->state = AudioSource::SOURCE_PREPARED;

    return 0;
}

int snd_pcm_prepare(snd_pcm_t *pcm)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE_GLOBAL(snd_pcm_prepare);
        return orig::snd_pcm_prepare(pcm);
    }

    DEBUGLOGCALL(LCF_SOUND);
    int sourceId = reinterpret_cast<intptr_t>(pcm);
    auto source = audiocontext.getSource(sourceId);
    if ((source->state == AudioSource::SOURCE_INITIAL) ||
        (source->state == AudioSource::SOURCE_UNDERRUN) ||
        (source->state == AudioSource::SOURCE_STOPPED))
        source->state = AudioSource::SOURCE_PREPARED;

    return 0;
}

static int get_latency(snd_pcm_t *pcm)
{
    std::lock_guard<std::mutex> lock(audiocontext.mutex);
    int sourceId = reinterpret_cast<intptr_t>(pcm);
    auto source = audiocontext.getSource(sourceId);
    return source->queueSize() - source->getPosition();
}

snd_pcm_sframes_t snd_pcm_writei(snd_pcm_t *pcm, const void *buffer, snd_pcm_uframes_t size)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE_GLOBAL(snd_pcm_writei);
        return orig::snd_pcm_writei(pcm, buffer, size);
    }

    debuglogstdio(LCF_SOUND, "snd_pcm_writei call with %d frames and pcm %p", size, pcm);

    /* Fill audio thread id */
    audiocontext.audio_thread = ThreadManager::getThreadId();
    int sourceId = reinterpret_cast<intptr_t>(pcm);
    auto source = audiocontext.getSource(sourceId);

    if (source->state == AudioSource::SOURCE_PREPARED) {
        /* Start playback */
        source->state = AudioSource::SOURCE_PLAYING;
    }

    if (source->state == AudioSource::SOURCE_UNDERRUN) {
        /* Underrun */
        return -EPIPE;
    }

    if (source->state != AudioSource::SOURCE_PLAYING) {
        return -EBADFD;
    }

    /* Blocking or return if latency is too high. Blocking should be done until
     * all frames can be written. */
    if (block_mode) {
        struct timespec mssleep = {0, 1000*1000};
        while (!is_exiting && ((get_latency(pcm) + static_cast<int>(size)) > buffer_size)) {
            NATIVECALL(nanosleep(&mssleep, NULL)); // Wait 1 ms before trying again
        }

        if (is_exiting) return 0;
    }
    else if (get_latency(pcm) >= buffer_size) {
        return -EAGAIN;
    }

    /* Only write a portion of the buffer if no room for the whole buffer */
    if ((get_latency(pcm)+static_cast<int>(size)) > buffer_size) {
        size = buffer_size - get_latency(pcm);
    }

    if (size == 0)
        return 0;

    std::lock_guard<std::mutex> lock(audiocontext.mutex);

    /* We try to reuse a buffer that has been processed from the source */
    std::shared_ptr<AudioBuffer> ab;
    if (source->nbQueueProcessed() > 0) {
        /* Removing first buffer */
        ab = source->buffer_queue[0];
        source->buffer_queue.erase(source->buffer_queue.begin());
        source->queue_index--;
    }
    else {
        /* Building a new buffer */
        int bufferId = audiocontext.createBuffer();
        ab = audiocontext.getBuffer(bufferId);

        /* Getting the parameters of the buffer from one of the queue */
        if (source->buffer_queue.empty()) {
            debuglogstdio(LCF_SOUND | LCF_ERROR, "Empty queue, cannot guess buffer parameters");
            return -1;
        }

        auto ref = source->buffer_queue[0];
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

    source->buffer_queue.push_back(ab);

    return static_cast<snd_pcm_sframes_t>(size);
}

snd_pcm_sframes_t snd_pcm_readi(snd_pcm_t *pcm, void *buffer, snd_pcm_uframes_t size)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE_GLOBAL(snd_pcm_readi);
        return orig::snd_pcm_readi(pcm, buffer, size);
    }

    debuglogstdio(LCF_SOUND, "%s call with %d bytes", __func__, size);
    return static_cast<snd_pcm_sframes_t>(size);
}

/* Pointer to buffer used for mmap writing */
std::shared_ptr<AudioBuffer> mmap_ab;

int snd_pcm_mmap_begin(snd_pcm_t *pcm, const snd_pcm_channel_area_t **areas, snd_pcm_uframes_t *offset, snd_pcm_uframes_t *frames)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE_GLOBAL(snd_pcm_mmap_begin);
        return orig::snd_pcm_mmap_begin(pcm, areas, offset, frames);
    }

    debuglogstdio(LCF_SOUND, "%s call with %d frames", __func__, *frames);

    /* Getting the available samples, don't return more frames than that */
    snd_pcm_sframes_t avail = buffer_size - get_latency(pcm);
    if (avail < 0)
        avail = 0;

    if (static_cast<snd_pcm_sframes_t>(*frames) > avail)
        *frames = avail;

    debuglogstdio(LCF_SOUND, "  returning %d frames", *frames);

    /* We should lock the audio mutex until snd_pcm_mmap_commit() is called,
     * but for some reason, FTL doesn't call snd_pcm_mmap_commit() the first
     * time, resulting in a deadlock. So for now we only lock inside this
     * function.
     */
    // audiocontext.mutex.lock();
    std::lock_guard<std::mutex> lock(audiocontext.mutex);

    int sourceId = reinterpret_cast<intptr_t>(pcm);
    auto source = audiocontext.getSource(sourceId);

    /* We try to reuse a buffer that has been processed from the source */
    if (source->nbQueueProcessed() > 0) {
        /* Removing first buffer */
        mmap_ab = source->buffer_queue[0];
        source->buffer_queue.erase(source->buffer_queue.begin());
        source->queue_index--;
    }
    else {
        /* Building a new buffer */
        int bufferId = audiocontext.createBuffer();
        mmap_ab = audiocontext.getBuffer(bufferId);

        /* Getting the parameters of the buffer from one of the queue */
        if (source->buffer_queue.empty()) {
            debuglogstdio(LCF_SOUND | LCF_ERROR, "Empty queue, cannot guess buffer parameters");
            return -1;
        }

        auto ref = source->buffer_queue[0];
        mmap_ab->format = ref->format;
        mmap_ab->nbChannels = ref->nbChannels;
        mmap_ab->frequency = ref->frequency;
    }

    /* Configuring the buffer */
    mmap_ab->update(); // Compute alignSize
    mmap_ab->sampleSize = *frames;
    mmap_ab->size = *frames * mmap_ab->alignSize;
    mmap_ab->samples.resize(mmap_ab->size);

    /* Fill the area info */
    static snd_pcm_channel_area_t my_areas[2];
    my_areas[0].addr = mmap_ab->samples.data();
    my_areas[0].first = 0;
    my_areas[0].step = mmap_ab->alignSize * 8; // in bits

    my_areas[1].addr = mmap_ab->samples.data();
    my_areas[1].first = mmap_ab->bitDepth; // in bits
    my_areas[1].step = mmap_ab->alignSize * 8; // in bits

    *areas = my_areas;
    *offset = 0;
    return 0;
}

snd_pcm_sframes_t snd_pcm_mmap_commit(snd_pcm_t *pcm, snd_pcm_uframes_t offset, snd_pcm_uframes_t frames)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE_GLOBAL(snd_pcm_mmap_commit);
        return orig::snd_pcm_mmap_commit(pcm, offset, frames);
    }

    /* Push the mmap buffer to the source */
    int sourceId = reinterpret_cast<intptr_t>(pcm);
    auto source = audiocontext.getSource(sourceId);
    source->buffer_queue.push_back(mmap_ab);

    /* We should unlock the audio mutex here, but we don't (see above comment) */
    // audiocontext.mutex.unlock();

    debuglogstdio(LCF_SOUND, "%s call with frames %d", __func__, frames);
    return frames;
}


int snd_pcm_hw_params_any(snd_pcm_t *pcm, snd_pcm_hw_params_t *params)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE_GLOBAL(snd_pcm_hw_params_any);
        return orig::snd_pcm_hw_params_any(pcm, params);
    }

    params = reinterpret_cast<snd_pcm_hw_params_t*>(pcm);
    DEBUGLOGCALL(LCF_SOUND);
    return 0;
}

size_t snd_pcm_hw_params_sizeof(void)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE_GLOBAL(snd_pcm_hw_params_sizeof);
        return orig::snd_pcm_hw_params_sizeof();
    }

    DEBUGLOGCALL(LCF_SOUND);
    return 8;
}

int snd_pcm_hw_params_malloc(snd_pcm_hw_params_t **ptr)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE_GLOBAL(snd_pcm_hw_params_malloc);
        return orig::snd_pcm_hw_params_malloc(ptr);
    }

    DEBUGLOGCALL(LCF_SOUND);
    *ptr = reinterpret_cast<snd_pcm_hw_params_t*>(1); // Set a non-null value
    return 0;
}

void snd_pcm_hw_params_free(snd_pcm_hw_params_t *obj)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE_GLOBAL(snd_pcm_hw_params_free);
        return orig::snd_pcm_hw_params_free(obj);
    }

    DEBUGLOGCALL(LCF_SOUND);
}

void snd_pcm_hw_params_copy(snd_pcm_hw_params_t *dst, const snd_pcm_hw_params_t *src)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE_GLOBAL(snd_pcm_hw_params_copy);
        return orig::snd_pcm_hw_params_copy(dst, src);
    }

    DEBUGLOGCALL(LCF_SOUND);
}

static snd_pcm_access_t current_access = SND_PCM_ACCESS_RW_INTERLEAVED;
int snd_pcm_hw_params_set_access(snd_pcm_t *pcm, snd_pcm_hw_params_t *params, snd_pcm_access_t access)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE_GLOBAL(snd_pcm_hw_params_set_access);
        return orig::snd_pcm_hw_params_set_access(pcm, params, access);
    }

    debuglogstdio(LCF_SOUND, "%s call with access %d", __func__, access);
    if ((access != SND_PCM_ACCESS_RW_INTERLEAVED) && (access != SND_PCM_ACCESS_MMAP_INTERLEAVED)) {
        debuglogstdio(LCF_SOUND | LCF_ERROR, "    Unsupported access %d", access);
    }
    current_access = access;
    return 0;
}

int snd_pcm_hw_params_set_format(snd_pcm_t *pcm, snd_pcm_hw_params_t *params, snd_pcm_format_t val)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE_GLOBAL(snd_pcm_hw_params_set_format);
        return orig::snd_pcm_hw_params_set_format(pcm, params, val);
    }

    debuglogstdio(LCF_SOUND, "%s call with format %d", __func__, val);

    int sourceId = reinterpret_cast<intptr_t>(pcm);
    auto source = audiocontext.getSource(sourceId);
    auto buffer = source->buffer_queue[0];

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
            debuglogstdio(LCF_SOUND | LCF_ERROR, "    Unsupported audio format");
            return -1;
    }

    return 0;
}

void snd_pcm_hw_params_get_format_mask(snd_pcm_hw_params_t *params, snd_pcm_format_mask_t *mask)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE_GLOBAL(snd_pcm_hw_params_get_format_mask);
        return orig::snd_pcm_hw_params_get_format_mask(params, mask);
    }

    DEBUGLOGCALL(LCF_SOUND);
}

int snd_pcm_hw_params_get_channels(const snd_pcm_hw_params_t *params, unsigned int *val)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE_GLOBAL(snd_pcm_hw_params_get_channels);
        return orig::snd_pcm_hw_params_get_channels(params, val);
    }

    DEBUGLOGCALL(LCF_SOUND);

    /* We don't have the pcm parameter here, so using the last opened source */
    int sourceId = last_source;
    auto source = audiocontext.getSource(sourceId);
    auto buffer = source->buffer_queue[0];
    *val = buffer->nbChannels;

    return 0;
}

int snd_pcm_hw_params_get_channels_min(const snd_pcm_hw_params_t *params, unsigned int *val)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE_GLOBAL(snd_pcm_hw_params_get_channels_min);
        return orig::snd_pcm_hw_params_get_channels_min(params, val);
    }

    DEBUGLOGCALL(LCF_SOUND);

    *val = 1;
    return 0;
}

int snd_pcm_hw_params_get_channels_max(const snd_pcm_hw_params_t *params, unsigned int *val)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE_GLOBAL(snd_pcm_hw_params_get_channels_max);
        return orig::snd_pcm_hw_params_get_channels_max(params, val);
    }

    DEBUGLOGCALL(LCF_SOUND);

    *val = 2;
    return 0;
}

int snd_pcm_hw_params_set_channels(snd_pcm_t *pcm, snd_pcm_hw_params_t *params, unsigned int val)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE_GLOBAL(snd_pcm_hw_params_set_channels);
        return orig::snd_pcm_hw_params_set_channels(pcm, params, val);
    }

    debuglogstdio(LCF_SOUND, "%s call with channels %d", __func__, val);

    int sourceId = reinterpret_cast<intptr_t>(pcm);
    auto source = audiocontext.getSource(sourceId);
    auto buffer = source->buffer_queue[0];
    buffer->nbChannels = val;

    return 0;
}

int snd_pcm_hw_params_set_rate(snd_pcm_t *pcm, snd_pcm_hw_params_t *params, unsigned int val, int dir)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE_GLOBAL(snd_pcm_hw_params_set_rate);
        return orig::snd_pcm_hw_params_set_rate(pcm, params, val, dir);
    }

    debuglogstdio(LCF_SOUND, "%s call with rate %d and dir %d", __func__, val, dir);

    int sourceId = reinterpret_cast<intptr_t>(pcm);
    auto source = audiocontext.getSource(sourceId);
    auto buffer = source->buffer_queue[0];
    buffer->frequency = val;

    return 0;
}

int snd_pcm_hw_params_set_rate_near(snd_pcm_t *pcm, snd_pcm_hw_params_t *params, unsigned int *val, int *dir)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE_GLOBAL(snd_pcm_hw_params_set_rate_near);
        return orig::snd_pcm_hw_params_set_rate_near(pcm, params, val, dir);
    }

    debuglogstdio(LCF_SOUND, "%s call with rate %d", __func__, *val);

    int sourceId = reinterpret_cast<intptr_t>(pcm);
    auto source = audiocontext.getSource(sourceId);
    auto buffer = source->buffer_queue[0];
    buffer->frequency = *val;

    return 0;
}

int snd_pcm_hw_params_set_rate_resample(snd_pcm_t *pcm, snd_pcm_hw_params_t *params, unsigned int val)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE_GLOBAL(snd_pcm_hw_params_set_rate_resample);
        return orig::snd_pcm_hw_params_set_rate_resample(pcm, params, val);
    }

    debuglogstdio(LCF_SOUND, "%s call with val %d", __func__, val);

    /* Not sure what should we do here */
    return 0;
}

int snd_pcm_hw_params_get_rate(const snd_pcm_hw_params_t *params, unsigned int *val, int *dir)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE_GLOBAL(snd_pcm_hw_params_get_rate);
        return orig::snd_pcm_hw_params_get_rate(params, val, dir);
    }

    DEBUGLOGCALL(LCF_SOUND);

    /* We don't have the pcm parameter here, so using the last opened source */
    int sourceId = last_source;
    auto source = audiocontext.getSource(sourceId);
    auto buffer = source->buffer_queue[0];
    if (buffer->frequency != 0) {
        *val = buffer->frequency;
        return 0;
    }

    return -1;
}

int snd_pcm_hw_params_get_rate_min(const snd_pcm_hw_params_t *params, unsigned int *val, int *dir)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE_GLOBAL(snd_pcm_hw_params_get_rate_min);
        return orig::snd_pcm_hw_params_get_rate_min(params, val, dir);
    }

    DEBUGLOGCALL(LCF_SOUND);
    *val = 11025;
    return 0;
}

int snd_pcm_hw_params_get_rate_max(const snd_pcm_hw_params_t *params, unsigned int *val, int *dir)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE_GLOBAL(snd_pcm_hw_params_get_rate_max);
        return orig::snd_pcm_hw_params_get_rate_max(params, val, dir);
    }

    DEBUGLOGCALL(LCF_SOUND);
    *val = 48000;
    return 0;
}


static int periods = 2;

int snd_pcm_hw_params_get_period_size(const snd_pcm_hw_params_t *params, snd_pcm_uframes_t *frames, int *dir)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE_GLOBAL(snd_pcm_hw_params_get_period_size);
        return orig::snd_pcm_hw_params_get_period_size(params, frames, dir);
    }

    DEBUGLOGCALL(LCF_SOUND);
    *frames = buffer_size / periods;

    return 0;
}

int snd_pcm_hw_params_get_period_time_min(const snd_pcm_hw_params_t *params, unsigned int *val, int *dir)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE_GLOBAL(snd_pcm_hw_params_get_period_time_min);
        return orig::snd_pcm_hw_params_get_period_time_min(params, val, dir);
    }

    DEBUGLOGCALL(LCF_SOUND);
    *val = 0;

    return 0;
}

int snd_pcm_hw_params_set_period_time_near(snd_pcm_t *pcm, snd_pcm_hw_params_t *params, unsigned int *val, int *dir)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE_GLOBAL(snd_pcm_hw_params_set_period_time_near);
        return orig::snd_pcm_hw_params_set_period_time_near(pcm, params, val, dir);
    }
    debuglogstdio(LCF_SOUND, "%s call with period time %d us and dir %d", __func__, *val, dir?*dir:-2);

    int sourceId = reinterpret_cast<intptr_t>(pcm);
    auto source = audiocontext.getSource(sourceId);
    auto buffer = source->buffer_queue[0];

    if (buffer->frequency != 0) {
        unsigned int period_size = static_cast<uint64_t>(*val) * buffer->frequency / 1000000;
        periods = buffer_size / period_size;
        /* Buffer size should be a multiple of period size, so we return a corrected value */
        /* TODO: support dir! */
        *val = 1000000 * (buffer_size / periods) / buffer->frequency;
        debuglogstdio(LCF_SOUND, "   returns period time of %d us", *val);
    }
    else {
        debuglogstdio(LCF_SOUND | LCF_ERROR, "   set period time without specifying sample rate");
    }    
    return 0;
}

int snd_pcm_hw_params_set_period_size_near(snd_pcm_t *pcm, snd_pcm_hw_params_t *params, snd_pcm_uframes_t *val, int *dir)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE_GLOBAL(snd_pcm_hw_params_set_period_size_near);
        return orig::snd_pcm_hw_params_set_period_size_near(pcm, params, val, dir);
    }
    debuglogstdio(LCF_SOUND, "%s call with period size %d and dir %d", __func__, *val, dir?*dir:-2);
    
    periods = buffer_size / *val;
    /* Buffer size should be a multiple of period size, so we return a corrected value */
    /* TODO: support dir! */
    *val = buffer_size / periods;

    debuglogstdio(LCF_SOUND, "   returning size %d ", *val);
    return 0;
}

int snd_pcm_hw_params_set_periods_near(snd_pcm_t *pcm, snd_pcm_hw_params_t *params, unsigned int *val, int *dir)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE_GLOBAL(snd_pcm_hw_params_set_periods_near);
        return orig::snd_pcm_hw_params_set_periods_near(pcm, params, val, dir);
    }

    debuglogstdio(LCF_SOUND, "%s call with period %d and dir %d", __func__, *val, dir?*dir:-2);
    periods = *val;
    return 0;
}

int snd_pcm_hw_params_get_periods(const snd_pcm_hw_params_t *params, unsigned int *val, int *dir)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE_GLOBAL(snd_pcm_hw_params_get_periods);
        return orig::snd_pcm_hw_params_get_periods(params, val, dir);
    }

    DEBUGLOGCALL(LCF_SOUND);
    *val = periods;
    return 0;
}

int snd_pcm_hw_params_get_buffer_size(const snd_pcm_hw_params_t *params, snd_pcm_uframes_t *val)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE_GLOBAL(snd_pcm_hw_params_get_buffer_size);
        return orig::snd_pcm_hw_params_get_buffer_size(params, val);
    }

    DEBUGLOGCALL(LCF_SOUND);
    *val = buffer_size;
    return 0;
}

int snd_pcm_hw_params_get_buffer_size_min(const snd_pcm_hw_params_t *params, snd_pcm_uframes_t *val)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE_GLOBAL(snd_pcm_hw_params_get_buffer_size_min);
        return orig::snd_pcm_hw_params_get_buffer_size_min(params, val);
    }

    DEBUGLOGCALL(LCF_SOUND);
    *val = BUFFER_SIZE_MIN;
    return 0;
}

int snd_pcm_hw_params_get_buffer_size_max(const snd_pcm_hw_params_t *params, snd_pcm_uframes_t *val)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE_GLOBAL(snd_pcm_hw_params_get_buffer_size_max);
        return orig::snd_pcm_hw_params_get_buffer_size_max(params, val);
    }

    DEBUGLOGCALL(LCF_SOUND);
    *val = 4096;
    return 0;
}

int snd_pcm_hw_params_get_buffer_time_max(const snd_pcm_hw_params_t *params, unsigned int *val, int *dir)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE_GLOBAL(snd_pcm_hw_params_get_buffer_time_max);
        return orig::snd_pcm_hw_params_get_buffer_time_max(params, val, dir);
    }

    DEBUGLOGCALL(LCF_SOUND);

    /* We don't have the pcm parameter here, so using the last opened source */
    int sourceId = last_source;
    auto source = audiocontext.getSource(sourceId);
    auto buffer = source->buffer_queue[0];

    /* The next operation can overflow using 32-bit ints */
    *val = static_cast<uint64_t>(buffer_size) * 1000000 / buffer->frequency;
    return 0;
}

int snd_pcm_hw_params_set_buffer_size_near(snd_pcm_t *pcm, snd_pcm_hw_params_t *params, snd_pcm_uframes_t *val)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE_GLOBAL(snd_pcm_hw_params_set_buffer_size_near);
        return orig::snd_pcm_hw_params_set_buffer_size_near(pcm, params, val);
    }

    debuglogstdio(LCF_SOUND, "%s call with buffer size %d", __func__, *val);
    if (*val < BUFFER_SIZE_MIN) {
        debuglogstdio(LCF_SOUND | LCF_WARNING, "Buffer size is too low, raising to %d", BUFFER_SIZE_MIN);
        *val = BUFFER_SIZE_MIN;
    }
    buffer_size = *val;
    return 0;
}

int snd_pcm_hw_params_set_buffer_time_near(snd_pcm_t *pcm, snd_pcm_hw_params_t *params, unsigned int *val, int *dir)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE_GLOBAL(snd_pcm_hw_params_set_buffer_time_near);
        return orig::snd_pcm_hw_params_set_buffer_time_near(pcm, params, val, dir);
    }

    debuglogstdio(LCF_SOUND, "%s call with buffer time %d", __func__, *val);

    int sourceId = reinterpret_cast<intptr_t>(pcm);
    auto source = audiocontext.getSource(sourceId);
    auto buffer = source->buffer_queue[0];

    /* Special case for 0, return the default value */
    if (*val == 0) {
        *val = static_cast<uint64_t>(buffer_size) * 1000000 / buffer->frequency;
        return 0;
    }

    if (buffer->frequency != 0) {
        buffer_size = static_cast<uint64_t>(*val) * buffer->frequency / 1000000;
        if (buffer_size < BUFFER_SIZE_MIN) {
            buffer_size = BUFFER_SIZE_MIN;

            *val = 1000000 * buffer_size / buffer->frequency;
            debuglogstdio(LCF_SOUND | LCF_WARNING, "Buffer time is too low, raising to %d us", *val);
        }
    }
    else {
        debuglogstdio(LCF_SOUND | LCF_ERROR, "   set buffer time without specifying sample rate");
    }    

    return 0;
}

int snd_pcm_hw_params_test_rate(snd_pcm_t *pcm, snd_pcm_hw_params_t *params, unsigned int val, int dir)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE_GLOBAL(snd_pcm_hw_params_test_rate);
        return orig::snd_pcm_hw_params_test_rate(pcm, params, val, dir);
    }

    debuglogstdio(LCF_SOUND, "%s call with val %d", __func__, val);
    return 0;
}

int snd_pcm_hw_params_test_format(snd_pcm_t *pcm, snd_pcm_hw_params_t *params, snd_pcm_format_t val)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE_GLOBAL(snd_pcm_hw_params_test_format);
        return orig::snd_pcm_hw_params_test_format(pcm, params, val);
    }

    debuglogstdio(LCF_SOUND, "%s call with val %d", __func__, val);
    return 0;
}

int snd_pcm_hw_params_test_channels(snd_pcm_t *pcm, snd_pcm_hw_params_t *params, unsigned int val)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE_GLOBAL(snd_pcm_hw_params_test_channels);
        return orig::snd_pcm_hw_params_test_channels(pcm, params, val);
    }

    debuglogstdio(LCF_SOUND, "%s call with val %d", __func__, val);
    return 0;
}

int snd_pcm_hw_params_get_access(const snd_pcm_hw_params_t *params, snd_pcm_access_t *access)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE_GLOBAL(snd_pcm_hw_params_get_access);
        return orig::snd_pcm_hw_params_get_access(params, access);
    }

    *access = current_access;
    DEBUGLOGCALL(LCF_SOUND);
    return 0;
}

int snd_pcm_get_params(snd_pcm_t *pcm, snd_pcm_uframes_t *bs, snd_pcm_uframes_t *ps)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE_GLOBAL(snd_pcm_get_params);
        return orig::snd_pcm_get_params(pcm, bs, ps);
    }

    DEBUGLOGCALL(LCF_SOUND);

    /* TODO: does not support multiple pcms */
    if (bs) *bs = buffer_size;
    if (ps) *ps = buffer_size / periods;

    return 0;
}

int snd_pcm_set_params(snd_pcm_t *pcm, snd_pcm_format_t format, snd_pcm_access_t access, unsigned int channels, unsigned int rate, int soft_resample, unsigned int latency)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE_GLOBAL(snd_pcm_set_params);
        return orig::snd_pcm_set_params(pcm, format, access, channels, rate, soft_resample, latency);
    }

    DEBUGLOGCALL(LCF_SOUND);

    int sourceId = reinterpret_cast<intptr_t>(pcm);
    auto source = audiocontext.getSource(sourceId);
    auto buffer = source->buffer_queue[0];

    switch(format) {
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
            debuglogstdio(LCF_SOUND | LCF_ERROR, "    Unsupported audio format");
            return -1;
    }

    if ((access != SND_PCM_ACCESS_RW_INTERLEAVED) && (access != SND_PCM_ACCESS_MMAP_INTERLEAVED)) {
        debuglogstdio(LCF_SOUND | LCF_ERROR, "    Unsupported access %d", access);
    }
    current_access = access;

    buffer->nbChannels = channels;
    buffer->frequency = rate;

    /* Special case for 0, return the default value */
    if ((latency != 0) && (rate != 0)) {
        buffer_size = static_cast<uint64_t>(latency) * rate / 1000000;
    }

    return 0;
}

size_t snd_pcm_sw_params_sizeof(void)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE_GLOBAL(snd_pcm_sw_params_sizeof);
        return orig::snd_pcm_sw_params_sizeof();
    }

    DEBUGLOGCALL(LCF_SOUND);
    return 8;
}


int snd_pcm_sw_params_set_start_threshold(snd_pcm_t *pcm, snd_pcm_sw_params_t *params, snd_pcm_uframes_t val)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE_GLOBAL(snd_pcm_sw_params_set_start_threshold);
        return orig::snd_pcm_sw_params_set_start_threshold(pcm, params, val);
    }

    debuglogstdio(LCF_SOUND, "%s call with start threshold %d", __func__, val);
    return 0;
}

int snd_pcm_sw_params_set_stop_threshold(snd_pcm_t *pcm, snd_pcm_sw_params_t *params, snd_pcm_uframes_t val)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE_GLOBAL(snd_pcm_sw_params_set_stop_threshold);
        return orig::snd_pcm_sw_params_set_stop_threshold(pcm, params, val);
    }

    debuglogstdio(LCF_SOUND, "%s call with stop threshold %d", __func__, val);
    return 0;
}

int snd_pcm_sw_params_set_avail_min(snd_pcm_t *pcm, snd_pcm_sw_params_t *params, snd_pcm_uframes_t val)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE_GLOBAL(snd_pcm_sw_params_set_avail_min);
        return orig::snd_pcm_sw_params_set_avail_min(pcm, params, val);
    }

    debuglogstdio(LCF_SOUND, "%s call with val %d", __func__, val);
    return 0;
}

snd_pcm_chmap_t *snd_pcm_get_chmap(snd_pcm_t *pcm)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE_GLOBAL(snd_pcm_get_chmap);
        return orig::snd_pcm_get_chmap(pcm);
    }

    DEBUGLOGCALL(LCF_SOUND);
    int channels = 2; // TODO!!
    snd_pcm_chmap_t *map = static_cast<snd_pcm_chmap_t*>(malloc(sizeof(int) * (channels + 1)));
    map->channels = channels;
    map->pos[0] = SND_CHMAP_FL;
    map->pos[1] = SND_CHMAP_FR;

    return map;
}

int snd_pcm_format_mask_malloc(snd_pcm_format_mask_t **ptr)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE_GLOBAL(snd_pcm_format_mask_malloc);
        return orig::snd_pcm_format_mask_malloc(ptr);
    }

    DEBUGLOGCALL(LCF_SOUND);
    *ptr = reinterpret_cast<snd_pcm_format_mask_t*>(1); // Set a non-null value
    return 0;
}

void snd_pcm_format_mask_free(snd_pcm_format_mask_t *obj)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE_GLOBAL(snd_pcm_format_mask_free);
        return orig::snd_pcm_format_mask_free(obj);
    }

    DEBUGLOGCALL(LCF_SOUND);
}

int snd_pcm_format_mask_test(const snd_pcm_format_mask_t *mask, snd_pcm_format_t val)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE_GLOBAL(snd_pcm_format_mask_test);
        return orig::snd_pcm_format_mask_test(mask, val);
    }

    DEBUGLOGCALL(LCF_SOUND);
    if ((val == SND_PCM_FORMAT_U8) ||
        (val == SND_PCM_FORMAT_S16_LE) ||
        (val == SND_PCM_FORMAT_S32_LE) ||
        (val == SND_PCM_FORMAT_FLOAT_LE))
        return 1;

    return 0;
}

snd_pcm_sframes_t snd_pcm_bytes_to_frames(snd_pcm_t *pcm, ssize_t bytes)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE_GLOBAL(snd_pcm_bytes_to_frames);
        return orig::snd_pcm_bytes_to_frames(pcm, bytes);
    }

    debuglogstdio(LCF_SOUND, "%s called with bytes %d", __func__, bytes);
    int sourceId = reinterpret_cast<intptr_t>(pcm);
    auto source = audiocontext.getSource(sourceId);
    auto buffer = source->buffer_queue[0];
    return bytes / buffer->alignSize;
}

ssize_t snd_pcm_frames_to_bytes(snd_pcm_t *pcm, snd_pcm_sframes_t frames)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE_GLOBAL(snd_pcm_format_mask_free);
        return orig::snd_pcm_frames_to_bytes(pcm, frames);
    }

    debuglogstdio(LCF_SOUND, "%s called with frames %d", __func__, frames);
    int sourceId = reinterpret_cast<intptr_t>(pcm);
    auto source = audiocontext.getSource(sourceId);
    auto buffer = source->buffer_queue[0];
    return buffer->alignSize * frames;
}


}
