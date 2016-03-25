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
#include "logging.h"
#include "hook.h"
#include "audio/AudioBuffer.h"

char* (*alcGetString_real)(void*, int) = nullptr;
void* (*alcOpenDevice_real)(const char*) = nullptr;

void link_openal(void);
void link_openal(void)
{
    LINK_SUFFIX(alcOpenDevice, "openal");
    LINK_SUFFIX(alcGetString, "openal");
}


#define ALC_DEVICE_SPECIFIER                     0x1005

/* Override */ void* alcOpenDevice(const char* devicename)
{

    debuglog(LCF_OPENAL, __func__, " call.");
    link_openal();

    /*
    const char *devices; 
    const char *ptr; 

    devices = alcGetString_real(NULL, ALC_DEVICE_SPECIFIER);
    ptr = devices;

    while (*ptr)
    {
        printf("   %s\n", ptr);
        ptr += strlen(ptr) + 1;
    }*/

    //(void) devicename; // To remove warning
    //const char device[] = "No Output";
    //return alcOpenDevice_real(device);
    return alcOpenDevice_real(devicename);
}

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
		int id = bufferList.createBuffer(nullptr);
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
	    if (! bufferList.isBuffer(buffers[i])) {
            ALSETERROR(AL_INVALID_NAME);
            return;
        }
    }
	for (int i=0; i<n; i++) {        
		bufferList.deleteBuffer(buffers[i]);
	}
}

ALboolean alIsBuffer(ALuint buffer)
{
    DEBUGLOGCALL(LCF_OPENAL);
	return bufferList.isBuffer(buffer);
}

void alBufferData(ALuint buffer, ALenum format, const ALvoid *data, ALsizei size, ALsizei freq)
{
    debuglog(LCF_OPENAL, __func__, " call - copy buffer data of format ", format, ", size ", size, " and frequency ", freq, " into buffer ", buffer);
	AudioBuffer* ab = bufferList.getBuffer(buffer);
    if (ab == nullptr) {
        ALSETERROR(AL_INVALID_NAME);
        return;
    }

    /* Check for size validity */
    int align;
    switch(format) {
        case AL_FORMAT_MONO8:
            align = 1;
            break;
        case AL_FORMAT_MONO16:
            align = 2;
            break;
        case AL_FORMAT_STEREO8:
            align = 2;
            break;
        case AL_FORMAT_STEREO16:
            align = 4;
            break;
    }
    if ((size % align) != 0) {
        /* Size is not aligned */
        ALSETERROR(AL_INVALID_VALUE);
        return;
    }

    /* Copy the data into our buffer */
    ab->samples.clear();
    ab->samples.insert(ab->samples.end(), &((uint8_t*)data)[0], &((uint8_t*)data)[size]);

    /* Fill the buffer informations */
    ab->size = size;
    ab->frequency = freq;
    switch(format) {
        case AL_FORMAT_MONO8:
            ab->bitDepth = 8;
            ab->nbChannels = 1;
            break;
        case AL_FORMAT_MONO16:
            ab->bitDepth = 16;
            ab->nbChannels = 1;
            break;
        case AL_FORMAT_STEREO8:
            ab->bitDepth = 8;
            ab->nbChannels = 2;
            break;
        case AL_FORMAT_STEREO16:
            ab->bitDepth = 16;
            ab->nbChannels = 2;
            break;
        default:
            break;
    }
}

void alGetBufferi(ALuint buffer, ALenum pname, ALint *value)
{
    DEBUGLOGCALL(LCF_OPENAL);

    if (value == nullptr) {
        return;
    }
        
	AudioBuffer* ab = bufferList.getBuffer(buffer);
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
		int id = bufferList.createBuffer(nullptr);
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
	    if (! bufferList.isBuffer(sources[i])) {
            ALSETERROR(AL_INVALID_NAME);
            return;
        }
    }
	for (int i=0; i<n; i++) {
        /* If the source is deleted when playing, the source must be stopped first */
        AudioBuffer* ab = bufferList.getBuffer(sources[i]);
        SourceState state = ab->getState();
        if (state == SOURCE_PLAYING)
            ab->changeState(SOURCE_STOPPED);
		bufferList.deleteBuffer(sources[i]);
	}
}

ALboolean alIsSource(ALuint source)
{
    DEBUGLOGCALL(LCF_OPENAL);
	return bufferList.isBuffer(source);
} 

void alSourcef(ALuint source, ALenum param, ALfloat value)
{
    DEBUGLOGCALL(LCF_OPENAL);
    AudioBuffer* ab = bufferList.getBuffer(source);
    if (ab == nullptr) {
        ALSETERROR(AL_INVALID_NAME);
        return;
    }

    switch(param) {
        case AL_GAIN:
            ab->volume = value;
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
            if (ab->nextBuffer != nullptr) {
                debuglog(LCF_OPENAL, "  Set position of ", value, " seconds");
                value *= (ALfloat) (ab->nextBuffer->frequency * ab->nextBuffer->nbChannels * ab->nextBuffer->bitDepth / 8);
                ab->setPosition((int)value);
            }
            break;
        case AL_SAMPLE_OFFSET:
            /* We fetch the buffer format of the source.
             * Normally, all buffers from a queue share the exact same format.
             */
            if (ab->nextBuffer != nullptr) {
                debuglog(LCF_OPENAL, "  Set position of ", value, " samples");
                value *= (ALfloat) (ab->nextBuffer->nbChannels * ab->nextBuffer->bitDepth / 8);
                ab->setPosition((int)value);
            }
            break;
        case AL_BYTE_OFFSET:
            debuglog(LCF_OPENAL, "  Set position of ", value, " bytes");
            ab->setPosition((int)value);
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
    AudioBuffer* ab = bufferList.getBuffer(source);
    if (ab == nullptr) {
        ALSETERROR(AL_INVALID_NAME);
        return;
    }

    AudioBuffer* bindab;
    SourceState state;
    switch(param) {
        case AL_LOOPING:
            debuglog(LCF_OPENAL, "  Set looping of ", value);
            if (value == AL_TRUE)
                ab->looping = true;
            else if (value == AL_FALSE)
                ab->looping = false;
            else
                ALSETERROR(AL_INVALID_VALUE);
            /* FIXME: Propagate looping to buffers here? */
            break;
        case AL_BUFFER:
            /* Bind a buffer to the source */
            state = ab->getState();

            if ((state == SOURCE_PLAYING) || (state == SOURCE_PAUSED)) {
                ALSETERROR(AL_INVALID_OPERATION);
                return;
            }

            if (value == 0) {
                /* Unbind buffer from source */
                ab->nextBuffer = nullptr;
                ab->processed = false;
                ab->source = SOURCE_UNDETERMINED;
                debuglog(LCF_OPENAL, "  Unbind buffer");
            }
            else {
                bindab = bufferList.getBuffer(value);
                if (bindab == nullptr) {
                    ALSETERROR(AL_INVALID_VALUE);
                    return;
                }
                ab->nextBuffer = bindab;
                ab->processed = true;
                ab->source = SOURCE_STATIC;
                bindab->nextBuffer = nullptr;
                bindab->state = state;
                bindab->position = 0;
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
        
    AudioBuffer* ab = bufferList.getBuffer(source);
    if (ab == nullptr)
        return;

    switch(param) {
        case AL_GAIN:
            *value = ab->volume;
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
            if (ab->nextBuffer != nullptr) {
                ALfloat pos = (ALfloat) ab->getPosition();
                pos /= (ALfloat) (ab->nextBuffer->frequency * ab->nextBuffer->nbChannels * ab->nextBuffer->bitDepth / 8);
                *value = pos;
                debuglog(LCF_OPENAL, "  Get position of ", *value, " seconds");
            }
            break;
        case AL_SAMPLE_OFFSET:
            /* We fetch the buffer format of the source.
             * Normally, all buffers from a queue share the exact same format.
             */
            if (ab->nextBuffer != nullptr) {
                ALfloat pos = (ALfloat) ab->getPosition();
                pos /= (ALfloat) (ab->nextBuffer->nbChannels * ab->nextBuffer->bitDepth / 8);
                *value = pos;
                debuglog(LCF_OPENAL, "  Get position of ", *value, " samples");
            }
            break;
        case AL_BYTE_OFFSET:
            *value = (ALfloat) ab->getPosition();
            debuglog(LCF_OPENAL, "  Get position of ", *value, " bytes");
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
    DEBUGLOGCALL(LCF_OPENAL);

    if (value == nullptr) {
        return;
    }
        
    AudioBuffer* ab = bufferList.getBuffer(source);
    if (ab == nullptr) {
        ALSETERROR(AL_INVALID_NAME);
        return;
    }

    SourceState state;
    switch(param) {
        case AL_BUFFER:
            if (ab->nextBuffer != nullptr)
                *value = ab->nextBuffer->id;
            else
                *value = AL_NONE;
            break;
        case AL_SOURCE_STATE:
            state = ab->getState();
            switch(state) {
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
            switch(ab->source) {
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
            }
            break;
        case AL_SOURCE_RELATIVE:
            debuglog(LCF_OPENAL, "Operation not supported");
            break;
        case AL_BUFFERS_QUEUED:
            *value = ab->nbQueue();
            debuglog(LCF_OPENAL, "  Get number of queued buffers of ", *value);
            break;
        case AL_BUFFERS_PROCESSED:
            state = ab->getState();
            if (state == SOURCE_STOPPED)
                *value = ab->nbQueue();
            else if (state == SOURCE_INITIAL)
                *value = 0;
            else
                *value = ab->nbQueueProcessed();
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
    AudioBuffer* ab = bufferList.getBuffer(source);
    if (ab == nullptr)
        return;

    SourceState state = ab->getState();
    if (state == SOURCE_PLAYING) {
        /* Restart the play from the beginning */
        ab->setPosition(0);
    }
    ab->changeState(SOURCE_PLAYING);
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
    AudioBuffer* ab = bufferList.getBuffer(source);
    if (ab == nullptr)
        return;

    SourceState state = ab->getState();
    if (state != SOURCE_PLAYED) {
        /* Illegal operation. */
        return;
    }
    ab->changeState(SOURCE_PAUSED);
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
    AudioBuffer* ab = bufferList.getBuffer(source);
    if (ab == nullptr)
        return;

    SourceState state = ab->getState();
    if ((state == SOURCE_INITIAL) || (state == SOURCE_STOPPED)) {
        /* Illegal operation. */
        return;
    }
    ab->changeState(SOURCE_STOPPED);
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
    AudioBuffer* ab = bufferList.getBuffer(source);
    if (ab == nullptr)
        return;

    SourceState state = ab->getState();
    if (state == SOURCE_INITIAL) {
        /* Illegal operation. */
        return;
    }
    ab->setPosition(0);
    ab->changeState(SOURCE_INITIAL);
}

void alSourceRewindv(ALsizei n, ALuint *sources)
{
    DEBUGLOGCALL(LCF_OPENAL);
    for (int i=0; i<n; i++)
        alSourceRewind(sources[i]);
}

void alSourceQueueBuffers(ALuint source, ALsizei n, ALuint* buffers)
{
    debuglog(LCF_OPENAL, "Pushing ", n, " buffers in the queue");
    AudioBuffer* ab = bufferList.getBuffer(source);
    if (ab == nullptr)
        return;

    /* Check if the source has a static buffer attached */
    if (ab->source == SOURCE_STATIC) {
        ALSETERROR(AL_INVALID_OPERATION);
        return;
    }

    ab->source = SOURCE_STREAMING;

    /* Locate the last buffer on the queue */
    while (ab->nextBuffer != nullptr) {
        ab = ab->nextBuffer;
    }

    for (int i=0; i<n; i++) {
        AudioBuffer* queue_ab = bufferList.getBuffer(buffers[i]);
        if (queue_ab == nullptr)
            return;

        queue_ab->source = SOURCE_STREAMING; // Is it necessary?

        ab->nextBuffer = queue_ab;
        ab = queue_ab;
        debuglog(LCF_OPENAL, "  Pushed buffer ", buffers[i]);
    }
}

void alSourceUnqueueBuffers(ALuint source, ALsizei n, ALuint* buffers)
{
    DEBUGLOGCALL(LCF_OPENAL);
    AudioBuffer* ab = bufferList.getBuffer(source);
    if (ab == nullptr)
        return;

    /* Check if we can unqueue that number of buffers */
    int processedBuffers = ab->nbQueueProcessed();
    if (processedBuffers < n) {
        ALSETERROR(AL_INVALID_VALUE);
        return;
    }

    AudioBuffer* ab_queue = ab;
    for (int i=0; i<n; i++) {
        ab_queue = ab_queue->nextBuffer;
        if (ab_queue == nullptr)
            return;

        if (! ab_queue->processed)
            /* This should not happen because of the previous check */
            return;

        /* Save the id of the unqueued buffer */
        buffers[i] = ab_queue->id;
    }

    /* Link the source AudioBuffer to the first non-unqueued Audio Buffer */
    ab->nextBuffer = ab_queue->nextBuffer;
}

/*** Listener ***/

void alListenerf(ALenum param, ALfloat value)
{
    DEBUGLOGCALL(LCF_OPENAL);
    if (param == AL_GAIN)
        bufferList.outVolume = value;
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
        *value = bufferList.outVolume;
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





