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

#include "pulseaudiosimple.h"
#include "../logging.h"
#include "AudioContext.h"
#include "AudioSource.h"
#include "AudioBuffer.h"
#include <time.h> //nanosleep

namespace libtas {

    static std::shared_ptr<AudioSource> sourcePulse;

    /** Create a new connection to the server. */
pa_simple* pa_simple_new(
    const char *,               /**< Server name, or NULL for default */
    const char *,               /**< A descriptive name for this client (application name, ...) */
    pa_stream_direction_t dir,  /**< Open this stream for recording or playback? */
    const char *dev,            /**< Sink (resp. source) name, or NULL for default */
    const char *,               /**< A descriptive name for this stream (application name, song title, ...) */
    const pa_sample_spec *ss,   /**< The sample type to use */
    const pa_channel_map *,     /**< The channel map to use, or NULL for default */
    const pa_buffer_attr *,     /**< Buffering attributes, or NULL for default */
    int *error                  /**< A pointer where the error code is stored when the routine returns NULL. It is OK to pass NULL here. */
) {
    DEBUGLOGCALL(LCF_SOUND);
    if (dir != PA_STREAM_PLAYBACK) {
        /* This is not a playback stream, return */
        return nullptr;
    }

    game_info.audio |= GameInfo::PULSEAUDIO;
    game_info.tosend = true;

    /* We create an empty buffer that holds the audio parameters. That way,
     * we can guess the parameters from a previous buffer when adding a new one.
     */
    int bufferId = audiocontext.createBuffer();
    auto buffer = audiocontext.getBuffer(bufferId);

    buffer->frequency = ss->rate;

    switch(ss->format) {
        case PA_SAMPLE_U8:
            buffer->format = AudioBuffer::SAMPLE_FMT_U8;
            break;
        case PA_SAMPLE_S16LE:
            buffer->format = AudioBuffer::SAMPLE_FMT_S16;
            break;
        case PA_SAMPLE_S32LE:
            buffer->format = AudioBuffer::SAMPLE_FMT_S32;
            break;
        case PA_SAMPLE_FLOAT32LE:
            buffer->format = AudioBuffer::SAMPLE_FMT_FLT;
            break;
        default:
            debuglog(LCF_SOUND | LCF_ERROR, "Unsupported audio format");
            return nullptr;
    }

    buffer->nbChannels = ss->channels;

    debuglog(LCF_SOUND, "    Format ",buffer->bitDepth," bits");
    debuglog(LCF_SOUND, "    Frequency ",buffer->frequency, " Hz");
    debuglog(LCF_SOUND, "    Channels ",buffer->nbChannels);

    buffer->size = 0;
    buffer->update();

    /* Push buffers in a source */
    int sourceId = audiocontext.createSource();
    sourcePulse = audiocontext.getSource(sourceId);

    sourcePulse->buffer_queue.push_back(buffer);
    sourcePulse->source = AudioSource::SOURCE_STREAMING;

    /* Start playing */
    sourcePulse->state = AudioSource::SOURCE_PLAYING;

    /* We return a non-null dummy value */
    return reinterpret_cast<pa_simple*>(1);

}

/** Close and free the connection to the server. The connection object becomes invalid when this is called. */
void pa_simple_free(pa_simple *)
{
    DEBUGLOGCALL(LCF_SOUND);

    /* Remove the source from the audio context */
    if (sourcePulse) {
        audiocontext.deleteSource(sourcePulse->id);
    }

    /* Destroy the source object */
    sourcePulse.reset();
}

/** Write some data to the server. */
int pa_simple_write(pa_simple *, const void *data, size_t bytes, int *)
{
    debuglog(LCF_SOUND, __func__, " call with ", bytes, " bytes written");

    /* We try to reuse a buffer that has been processed from the source */
    std::shared_ptr<AudioBuffer> ab;
    if (sourcePulse->nbQueueProcessed() > 0) {
        /* Removing first buffer */
        ab = sourcePulse->buffer_queue[0];
        sourcePulse->buffer_queue.erase(sourcePulse->buffer_queue.begin());
        sourcePulse->queue_index--;
    }
    else {
        /* Building a new buffer */
        int bufferId = audiocontext.createBuffer();
        ab = audiocontext.getBuffer(bufferId);

        /* Getting the parameters of the buffer from one of the queue */
        if (sourcePulse->buffer_queue.empty()) {
            debuglog(LCF_SOUND | LCF_ERROR, "Empty queue, cannot guess buffer parameters");
            return -1;
        }

        auto ref = sourcePulse->buffer_queue[0];
        ab->format = ref->format;
        ab->nbChannels = ref->nbChannels;
        ab->frequency = ref->frequency;
    }

    /* Filling buffer */
    ab->samples.clear();
    ab->samples.insert(ab->samples.end(), static_cast<const uint8_t*>(data), &(static_cast<const uint8_t*>(data))[bytes]);
    ab->size = bytes;
    ab->update();
    sourcePulse->buffer_queue.push_back(ab);

    /* Blocking if latency is too high */
    while (sourcePulse && pa_simple_get_latency(nullptr, nullptr) > 50000) {
        struct timespec mssleep = {0, 10000000};
        NATIVECALL(nanosleep(&mssleep, NULL)); // Wait 10 ms before trying again
    }
    return 0;
}

/** Wait until all data already written is played by the daemon. */
int pa_simple_drain(pa_simple *, int *)
{
    DEBUGLOGCALL(LCF_SOUND);

    /* I don't think we should support this */
    return 0;
}

/** Read some data from the server. This function blocks until \a bytes amount
 * of data has been received from the server, or until an error occurs.
 * Returns a negative value on failure. */
int pa_simple_read(pa_simple *, void *, size_t , int *)
{
    DEBUGLOGCALL(LCF_SOUND);

    /* We don't support stream reading */
    return 0;
}

/** Return the playback or record latency. */
pa_usec_t pa_simple_get_latency(pa_simple *, int *error)
{
    if (sourcePulse->buffer_queue.empty()) {
        /* Nothing is playing, return 0 latency */
        return 0;
    }

    pa_usec_t frequency = static_cast<pa_usec_t>(sourcePulse->buffer_queue[0]->frequency);
    pa_usec_t sample_latency = static_cast<pa_usec_t>(sourcePulse->queueSize() - sourcePulse->getPosition());
    return sample_latency * 1000000 / frequency;
}

/** Flush the playback or record buffer. This discards any audio in the buffer. */
int pa_simple_flush(pa_simple *s, int *error)
{
    DEBUGLOGCALL(LCF_SOUND | LCF_TODO);
    return 0;
}

}
