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

#include "openal.h"
#include "../logging.h"
#include "AudioBuffer.h"
#include "AudioSource.h"
#include "AudioContext.h"

ALenum alError;
#define ALSETERROR(error) if(alError==AL_NO_ERROR) alError = error

ALenum alGetError(ALvoid)
{
    ALenum err = alError;
    alError = AL_NO_ERROR;
    return err;
}

void alGenBuffers(ALsizei n, ALuint *buffers)
{
    debuglog(LCF_OPENAL, __func__, " call - generate ", n, " buffers");
    for (int i=0; i<n; i++) {
        int id = audiocontext.createBuffer();
        if (id > 0)
            buffers[i] = (ALuint) id;
        /* TODO: else generate an error */ 
    }
}

void alDeleteBuffers(ALsizei n, ALuint *buffers)
{
    debuglog(LCF_OPENAL, __func__, " call - delete ", n, " buffers");
    for (int i=0; i<n; i++) {
        /* Check if all buffers exist before removing any. */
        if (! audiocontext.isBuffer(buffers[i])) {
            ALSETERROR(AL_INVALID_NAME);
            return;
        }
    }
    for (int i=0; i<n; i++) {        
        audiocontext.deleteBuffer(buffers[i]);
    }
}

ALboolean alIsBuffer(ALuint buffer)
{
    DEBUGLOGCALL(LCF_OPENAL);
    return audiocontext.isBuffer(buffer);
}

void alBufferData(ALuint buffer, ALenum format, const ALvoid *data, ALsizei size, ALsizei freq)
{
    debuglog(LCF_OPENAL, __func__, " call - copy buffer data of format ", format, ", size ", size, " and frequency ", freq, " into buffer ", buffer);
	AudioBuffer* ab = audiocontext.getBuffer(buffer);
    if (ab == nullptr) {
        ALSETERROR(AL_INVALID_NAME);
        return;
    }

    std::lock_guard<std::mutex> lock(audiocontext.mutex);
    
    /* Fill the buffer informations */
    ab->size = size;
    ab->frequency = freq;
    switch(format) {
        case AL_FORMAT_MONO8:
            ab->format = SAMPLE_FMT_U8;
            ab->nbChannels = 1;
            break;
        case AL_FORMAT_MONO16:
            ab->format = SAMPLE_FMT_S16;
            ab->nbChannels = 1;
            break;
        case AL_FORMAT_STEREO8:
            ab->format = SAMPLE_FMT_U8;
            ab->nbChannels = 2;
            break;
        case AL_FORMAT_STEREO16:
            ab->format = SAMPLE_FMT_S16;
            ab->nbChannels = 2;
            break;
        case AL_FORMAT_MONO_FLOAT32:
            ab->format = SAMPLE_FMT_FLT;
            ab->nbChannels = 1;
            break;
        case AL_FORMAT_STEREO_FLOAT32:
            ab->format = SAMPLE_FMT_FLT;
            ab->nbChannels = 2;
            break;
        case AL_FORMAT_MONO_DOUBLE_EXT:
            ab->format = SAMPLE_FMT_DBL;
            ab->nbChannels = 1;
            break;
        case AL_FORMAT_STEREO_DOUBLE_EXT:
            ab->format = SAMPLE_FMT_DBL;
            ab->nbChannels = 2;
            break;
        default:
            debuglog(LCF_OPENAL | LCF_ERROR, "Unsupported format: ", format);
            return;
    }

    ab->update();

    /* Check for size validity */
    if ((size % ab->alignSize) != 0) {
        /* Size is not aligned */
        ALSETERROR(AL_INVALID_VALUE);
        return;
    }

    /* Copy the data into our buffer */
    ab->samples.clear();
    ab->samples.insert(ab->samples.end(), &((uint8_t*)data)[0], &((uint8_t*)data)[size]);

}

void alGetBufferi(ALuint buffer, ALenum pname, ALint *value)
{
    DEBUGLOGCALL(LCF_OPENAL);

    if (value == nullptr) {
        return;
    }
        
	AudioBuffer* ab = audiocontext.getBuffer(buffer);
    if (ab == nullptr) {
        ALSETERROR(AL_INVALID_NAME);
        return;
    }

    switch(pname) {
        case AL_FREQUENCY:
            *value = ab->frequency;
            debuglog(LCF_OPENAL, "  Get frequency of ", *value);
            return;
        case AL_BITS:
            *value = ab->bitDepth;
            debuglog(LCF_OPENAL, "  Get bit depth of ", *value);
            return;
        case AL_CHANNELS:
            *value = ab->nbChannels;
            debuglog(LCF_OPENAL, "  Get channel number of ", *value);
            return;
        case AL_SIZE:
            *value = ab->size;
            debuglog(LCF_OPENAL, "  Get size of ", *value);
            return;
        case AL_UNPACK_BLOCK_ALIGNMENT_SOFT:
            // TODO
            debuglog(LCF_OPENAL, "  Get block alignment of ", *value);
            return;
        default:
            ALSETERROR(AL_INVALID_VALUE);
            return;
    }
}

void alGetBufferiv(ALuint buffer, ALenum pname, ALint *values)
{
    DEBUGLOGCALL(LCF_OPENAL);
    
    if (values == nullptr) {
        return;
    }
        
    switch(pname) {
        case AL_FREQUENCY:
        case AL_BITS:
        case AL_CHANNELS:
        case AL_SIZE:
            alGetBufferi(buffer, pname, values);
            return;
        default:
            ALSETERROR(AL_INVALID_VALUE);
            return;
    }
}

/*** Source functions ***/

void alGenSources(ALsizei n, ALuint *sources)
{
    debuglog(LCF_OPENAL, __func__, " call - generate ", n, " sources");
	for (int i=0; i<n; i++) {
		int id = audiocontext.createSource();
		if (id > 0) {
			sources[i] = (ALuint) id;
        }
		/* TODO: else generate an error */ 
	}
}

void alDeleteSources(ALsizei n, ALuint *sources)
{
    debuglog(LCF_OPENAL, __func__, " call - delete ", n, " sources");
	for (int i=0; i<n; i++) {
        /* Check if all sources exist before removing any. */
	    if (! audiocontext.isSource(sources[i])) {
            ALSETERROR(AL_INVALID_NAME);
            return;
        }
    }
	for (int i=0; i<n; i++) {
        /* If the source is deleted when playing, the source must be stopped first */
        AudioSource* as = audiocontext.getSource(sources[i]);
        if (as->state == SOURCE_PLAYING)
            as->state = SOURCE_STOPPED;
		audiocontext.deleteSource(sources[i]);
	}
}

ALboolean alIsSource(ALuint source)
{
    DEBUGLOGCALL(LCF_OPENAL);
	return audiocontext.isSource(source);
} 

void alSourcef(ALuint source, ALenum param, ALfloat value)
{
    DEBUGLOGCALL(LCF_OPENAL);
    AudioSource* as = audiocontext.getSource(source);
    if (as == nullptr) {
        ALSETERROR(AL_INVALID_NAME);
        return;
    }

    AudioBuffer* ab;
    switch(param) {
        case AL_GAIN:
            as->volume = value;
            debuglog(LCF_OPENAL, "  Set gain of ", value);
            break;
        case AL_PITCH:
        case AL_MIN_GAIN:
        case AL_MAX_GAIN:
        case AL_MAX_DISTANCE:
        case AL_ROLLOFF_FACTOR:
        case AL_CONE_OUTER_GAIN:
        case AL_CONE_INNER_ANGLE:
        case AL_CONE_OUTER_ANGLE:
        case AL_REFERENCE_DISTANCE:
            debuglog(LCF_OPENAL, "Operation not supported");
            break;
        case AL_SEC_OFFSET:
            /* We fetch the buffer format of the source.
             * Normally, all buffers from a queue share the exact same format.
             */
            if (! as->buffer_queue.empty()) {
                ab = as->buffer_queue[0];
                debuglog(LCF_OPENAL, "  Set position of ", value, " seconds");
                value *= (ALfloat) ab->frequency;
                as->setPosition((int)value);
            }
            break;
        case AL_SAMPLE_OFFSET:
            /* We fetch the buffer format of the source.
             * Normally, all buffers from a queue share the exact same format.
             */
            debuglog(LCF_OPENAL, "  Set position of ", value, " samples");
            as->setPosition((int)value);
            break;
        case AL_BYTE_OFFSET:
            if (! as->buffer_queue.empty()) {
                ab = as->buffer_queue[0];
                value /= (ALfloat) ab->alignSize;
                debuglog(LCF_OPENAL, "  Set position of ", value, " bytes");
                as->setPosition((int)value);
            }
            break;
        default:
            ALSETERROR(AL_INVALID_OPERATION);
            return;
    }
}

void alSource3f(ALuint source, ALenum param, ALfloat v1, ALfloat v2, ALfloat v3)
{
    DEBUGLOGCALL(LCF_OPENAL);
    debuglog(LCF_OPENAL, "Operation not supported");
}

void alSourcefv(ALuint source, ALenum param, ALfloat *values)
{
    DEBUGLOGCALL(LCF_OPENAL);
    if (values == nullptr) {
        ALSETERROR(AL_INVALID_VALUE);
        return;
    }
    alSourcef(source, param, *values);
}

void alSourcei(ALuint source, ALenum param, ALint value)
{
    DEBUGLOGCALL(LCF_OPENAL);
    AudioSource* as = audiocontext.getSource(source);
    if (as == nullptr) {
        ALSETERROR(AL_INVALID_NAME);
        return;
    }

    std::lock_guard<std::mutex> lock(audiocontext.mutex);

    AudioBuffer* bindab;
    switch(param) {
        case AL_LOOPING:
            debuglog(LCF_OPENAL, "  Set looping of ", value);
            if (value == AL_TRUE)
                as->looping = true;
            else if (value == AL_FALSE)
                as->looping = false;
            else
                ALSETERROR(AL_INVALID_VALUE);
            break;
        case AL_BUFFER:
            /* Bind a buffer to the source */

            if ((as->state == SOURCE_PLAYING) || (as->state == SOURCE_PAUSED)) {
                ALSETERROR(AL_INVALID_OPERATION);
                return;
            }

            if (value == 0) {
                /* Unbind buffer from source */
                as->init();
                as->buffer_queue.clear();
                as->source = SOURCE_UNDETERMINED;
                as->state = SOURCE_INITIAL;
                debuglog(LCF_OPENAL, "  Unbind buffer");
            }
            else {
                bindab = audiocontext.getBuffer(value);
                if (bindab == nullptr) {
                    ALSETERROR(AL_INVALID_VALUE);
                    return;
                }
                as->init();
                as->buffer_queue.clear();
                as->buffer_queue.push_back(bindab);
                as->source = SOURCE_STATIC;
                as->state = SOURCE_INITIAL;
                debuglog(LCF_OPENAL, "  Bind to buffer ", value);
            }
            break;
        case AL_SOURCE_RELATIVE:
        case AL_CONE_INNER_ANGLE:
        case AL_CONE_OUTER_ANGLE:
            debuglog(LCF_OPENAL, "Operation not supported");
            break;
        case AL_SEC_OFFSET:
        case AL_SAMPLE_OFFSET:
        case AL_BYTE_OFFSET:
            alSourcef(source, param, (ALfloat)value);
            break;
        default:
            ALSETERROR(AL_INVALID_OPERATION);
            return;
    }
}

void alSource3i(ALuint source, ALenum param, ALint v1, ALint v2, ALint v3)
{
    DEBUGLOGCALL(LCF_OPENAL);
    debuglog(LCF_OPENAL, "Operation not supported");
}

void alSourceiv(ALuint source, ALenum param, ALint *values)
{
    DEBUGLOGCALL(LCF_OPENAL);
    if (values == nullptr) {
        ALSETERROR(AL_INVALID_VALUE);
        return;
    }
    alSourcei(source, param, *values);
}

void alGetSourcef(ALuint source, ALenum param, ALfloat *value)
{
    DEBUGLOGCALL(LCF_OPENAL);

    if (value == nullptr) {
        return;
    }
        
    AudioSource* as = audiocontext.getSource(source);
    if (as == nullptr)
        return;

    AudioBuffer* ab;
    switch(param) {
        case AL_GAIN:
            *value = as->volume;
            debuglog(LCF_OPENAL, "  Get gain of ", *value);
            break;
        case AL_PITCH:
        case AL_MIN_GAIN:
        case AL_MAX_GAIN:
        case AL_MAX_DISTANCE:
        case AL_ROLLOFF_FACTOR:
        case AL_CONE_OUTER_GAIN:
        case AL_CONE_INNER_ANGLE:
        case AL_CONE_OUTER_ANGLE:
        case AL_REFERENCE_DISTANCE:
            debuglog(LCF_OPENAL, "Operation not supported");
            break;
        case AL_SEC_OFFSET:
            /* We fetch the buffer format of the source.
             * Normally, all buffers from a queue share the exact same format.
             */
            if (! as->buffer_queue.empty()) {
                ab = as->buffer_queue[0];
                ALfloat pos = (ALfloat) as->getPosition();
                pos /= (ALfloat) ab->frequency;
                *value = pos;
                debuglog(LCF_OPENAL, "  Get position of ", *value, " seconds");
            }
            break;
        case AL_SAMPLE_OFFSET:
            *value = (ALfloat) as->getPosition();
            debuglog(LCF_OPENAL, "  Get position of ", *value, " samples");
            break;
        case AL_BYTE_OFFSET:
            /* We fetch the buffer format of the source.
             * Normally, all buffers from a queue share the exact same format.
             */
            if (! as->buffer_queue.empty()) {
                ab = as->buffer_queue[0];
                ALfloat pos = (ALfloat) as->getPosition();
                pos *= (ALfloat) ab->alignSize;
                *value = pos;
                debuglog(LCF_OPENAL, "  Get position of ", *value, " bytes");
            }
            break;
        default:
            ALSETERROR(AL_INVALID_OPERATION);
            return;
    }
}

void alGetSource3f(ALuint source, ALenum param, ALfloat *v1, ALfloat *v2, ALfloat *v3)
{
    DEBUGLOGCALL(LCF_OPENAL);
    debuglog(LCF_OPENAL, "Operation not supported");
}

void alGetSourcefv(ALuint source, ALenum param, ALfloat *values)
{
    DEBUGLOGCALL(LCF_OPENAL);
    alGetSourcef(source, param, values);
}

void alGetSourcei(ALuint source, ALenum param, ALint *value)
{
    debuglog(LCF_OPENAL, __func__, " call for source ", source);

    if (value == nullptr) {
        return;
    }
        
    AudioSource* as = audiocontext.getSource(source);
    if (as == nullptr) {
        ALSETERROR(AL_INVALID_NAME);
        return;
    }

    switch(param) {
        case AL_BUFFER:
            if (! as->buffer_queue.empty())
                // TODO: for queued buffers, return the index of current buffer?
                *value = as->buffer_queue[0]->id;
            else
                *value = AL_NONE;
            break;
        case AL_SOURCE_STATE:
            switch(as->state) {
                case SOURCE_INITIAL:
                    *value = AL_INITIAL;
                    debuglog(LCF_OPENAL, "  Get source state INITIAL");
                    break;
                case SOURCE_PLAYING:
                    *value = AL_PLAYING;
                    debuglog(LCF_OPENAL, "  Get source state PLAYING");
                    break;
                case SOURCE_PAUSED:
                    *value = AL_PAUSED;
                    debuglog(LCF_OPENAL, "  Get source state PAUSED");
                    break;
                case SOURCE_STOPPED:
                    *value = AL_STOPPED;
                    debuglog(LCF_OPENAL, "  Get source state STOPPED");
                    break;
            }
            break;
        case AL_SOURCE_TYPE:
            switch(as->source) {
                case SOURCE_UNDETERMINED:
                    *value = AL_UNDETERMINED;
                    debuglog(LCF_OPENAL, "  Get source type UNDETERMINED");
                    break;
                case SOURCE_STATIC:
                    *value = AL_STATIC;
                    debuglog(LCF_OPENAL, "  Get source type STATIC");
                    break;
                case SOURCE_STREAMING:
                    *value = AL_STREAMING;
                    debuglog(LCF_OPENAL, "  Get source type STREAMING");
                    break;
                default:
                    ALSETERROR(AL_INVALID_VALUE);
                    break;
            }
            break;
        case AL_SOURCE_RELATIVE:
            debuglog(LCF_OPENAL, "Operation not supported");
            break;
        case AL_BUFFERS_QUEUED:
            *value = as->nbQueue();
            debuglog(LCF_OPENAL, "  Get number of queued buffers of ", *value);
            break;
        case AL_BUFFERS_PROCESSED:
            if (as->state == SOURCE_STOPPED)
                *value = as->nbQueue();
            else if (as->state == SOURCE_INITIAL)
                *value = 0;
            else
                *value = as->nbQueueProcessed();
            debuglog(LCF_OPENAL, "  Get number of processed queued buffers of ", *value);
            break;
        case AL_SEC_OFFSET:
        case AL_SAMPLE_OFFSET:
        case AL_BYTE_OFFSET:
            ALfloat res;
            alGetSourcef(source, param, &res);
            *value = (ALint) res;
            break;
        default:
            ALSETERROR(AL_INVALID_OPERATION);
            return;
    }
}

void alGetSource3i(ALuint source, ALenum param, ALint *v1, ALint *v2, ALint *v3)
{
    DEBUGLOGCALL(LCF_OPENAL);
    debuglog(LCF_OPENAL, "Operation not supported");
}

void alGetSourceiv(ALuint source, ALenum param, ALint *values)
{
    DEBUGLOGCALL(LCF_OPENAL);
    alGetSourcei(source, param, values);
}

void alSourcePlay(ALuint source)
{
    DEBUGLOGCALL(LCF_OPENAL);
    AudioSource* as = audiocontext.getSource(source);
    if (as == nullptr)
        return;

    if (as->state == SOURCE_PLAYING) {
        /* Restart the play from the beginning */
        as->setPosition(0);
    }
    as->state = SOURCE_PLAYING;
}

void alSourcePlayv(ALsizei n, ALuint *sources)
{
    DEBUGLOGCALL(LCF_OPENAL);
    for (int i=0; i<n; i++)
        alSourcePlay(sources[i]);
}

void alSourcePause(ALuint source)
{
    DEBUGLOGCALL(LCF_OPENAL);
    AudioSource* as = audiocontext.getSource(source);
    if (as == nullptr)
        return;

    if (as->state != SOURCE_PLAYING) {
        /* Illegal operation. */
        return;
    }
    as->state = SOURCE_PAUSED;
}

void alSourcePausev(ALsizei n, ALuint *sources)
{
    DEBUGLOGCALL(LCF_OPENAL);
    for (int i=0; i<n; i++)
        alSourcePause(sources[i]);
}

void alSourceStop(ALuint source)
{
    DEBUGLOGCALL(LCF_OPENAL);
    AudioSource* as = audiocontext.getSource(source);
    if (as == nullptr)
        return;

    if ((as->state == SOURCE_INITIAL) || (as->state == SOURCE_STOPPED)) {
        /* Illegal operation. */
        return;
    }
    as->init();
    as->state = SOURCE_STOPPED;
}

void alSourceStopv(ALsizei n, ALuint *sources)
{
    DEBUGLOGCALL(LCF_OPENAL);
    for (int i=0; i<n; i++)
        alSourceStop(sources[i]);
}

void alSourceRewind(ALuint source)
{
    DEBUGLOGCALL(LCF_OPENAL);
    AudioSource* as = audiocontext.getSource(source);
    if (as == nullptr)
        return;

    if (as->state == SOURCE_INITIAL) {
        /* Illegal operation. */
        return;
    }
    as->setPosition(0);
    as->state = SOURCE_INITIAL;
}

void alSourceRewindv(ALsizei n, ALuint *sources)
{
    DEBUGLOGCALL(LCF_OPENAL);
    for (int i=0; i<n; i++)
        alSourceRewind(sources[i]);
}

void alSourceQueueBuffers(ALuint source, ALsizei n, ALuint* buffers)
{
    debuglog(LCF_OPENAL, "Pushing ", n, " buffers in the queue of source ", source);
    AudioSource* as = audiocontext.getSource(source);
    if (as == nullptr)
        return;

    std::lock_guard<std::mutex> lock(audiocontext.mutex);

    /* Check if the source has a static buffer attached */
    if (as->source == SOURCE_STATIC) {
        ALSETERROR(AL_INVALID_OPERATION);
        return;
    }

    as->source = SOURCE_STREAMING;

    /* TODO: Check that all buffers have the same format */
    for (int i=0; i<n; i++) {
        AudioBuffer* queue_ab = audiocontext.getBuffer(buffers[i]);
        if (queue_ab == nullptr)
            return;

        queue_ab->processed = false;
        as->buffer_queue.push_back(queue_ab);
        debuglog(LCF_OPENAL, "  Pushed buffer ", buffers[i]);
    }
}

void alSourceUnqueueBuffers(ALuint source, ALsizei n, ALuint* buffers)
{
    DEBUGLOGCALL(LCF_OPENAL);
    AudioSource* as = audiocontext.getSource(source);
    if (as == nullptr)
        return;

    /* Check if we can unqueue that number of buffers */
    int processedBuffers;
    if (as->state == SOURCE_STOPPED)
        processedBuffers = as->nbQueue();
    else
        processedBuffers = as->nbQueueProcessed();
    if (processedBuffers < n) {
        ALSETERROR(AL_INVALID_VALUE);
        return;
    }

    debuglog(LCF_OPENAL, "Unqueueing ", n, " buffers out of ", as->nbQueue());
    
    std::lock_guard<std::mutex> lock(audiocontext.mutex);

    /* Save the id of the unqueued buffers */
    for (int i=0; i<n; i++) {
        buffers[i] = as->buffer_queue[i]->id;
    }

    /* Remove the buffers from the queue.
     * TODO: This is slow on a vector, maybe use forward_list?
     */
    as->buffer_queue.erase(as->buffer_queue.begin(), as->buffer_queue.begin()+n);
    if (as->state != SOURCE_STOPPED)
        as->queue_index -= n;
}

/*** Listener ***/

void alListenerf(ALenum param, ALfloat value)
{
    DEBUGLOGCALL(LCF_OPENAL);
    if (param == AL_GAIN)
        audiocontext.outVolume = value;
}

void alListener3f(ALenum param, ALfloat v1, ALfloat v2, ALfloat v3)
{
    DEBUGLOGCALL(LCF_OPENAL);
    debuglog(LCF_OPENAL, "Operation not supported");
}

void alListenerfv(ALenum param, ALfloat *values)
{
    DEBUGLOGCALL(LCF_OPENAL);
    debuglog(LCF_OPENAL, "Operation not supported");
}

void alListeneri(ALenum param, ALint value)
{
    DEBUGLOGCALL(LCF_OPENAL);
    debuglog(LCF_OPENAL, "Operation not supported");
}

void alListener3i(ALenum param, ALint v1, ALint v2, ALint v3)
{
    DEBUGLOGCALL(LCF_OPENAL);
    debuglog(LCF_OPENAL, "Operation not supported");
}

void alListeneriv(ALenum param, ALint *values)
{
    DEBUGLOGCALL(LCF_OPENAL);
    debuglog(LCF_OPENAL, "Operation not supported");
}

void alGetListenerf(ALenum param, ALfloat *value)
{
    DEBUGLOGCALL(LCF_OPENAL);
    if (param == AL_GAIN) {
        if (*value < 0) {
            ALSETERROR(AL_INVALID_VALUE);
            return;
        }
        *value = audiocontext.outVolume;
    }
}

void alGetListener3f(ALenum param, ALfloat *v1, ALfloat *v2, ALfloat *v3)
{
    DEBUGLOGCALL(LCF_OPENAL);
    debuglog(LCF_OPENAL, "Operation not supported");
}

void alGetListenerfv(ALenum param, ALfloat *values)
{
    DEBUGLOGCALL(LCF_OPENAL);
    if (param == AL_GAIN)
        alGetListenerf(param, values);
    else
        debuglog(LCF_OPENAL, "Operation not supported");
}

void alGetListeneri(ALenum param, ALint *value)
{
    DEBUGLOGCALL(LCF_OPENAL);
    debuglog(LCF_OPENAL, "Operation not supported");
}

void alGetListener3i(ALenum param, ALint *v1, ALint *v2, ALint *v3)
{
    DEBUGLOGCALL(LCF_OPENAL);
    debuglog(LCF_OPENAL, "Operation not supported");
}

void alGetListeneriv(ALenum param, ALint *values)
{
    DEBUGLOGCALL(LCF_OPENAL);
    debuglog(LCF_OPENAL, "Operation not supported");
}


