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
    debuglog(LCF_OPENAL, __func__, "call - generate ", n, " buffers");
	for (int i=0; i<n; i++) {
		int id = bufferList.createBuffer(nullptr);
		if (id > 0)
			buffers[i] = (ALuint) id;
		/* TODO: else generate an error */ 
	}
}

void alDeleteBuffers(ALsizei n, ALuint *buffers)
{
    debuglog(LCF_OPENAL, __func__, "call - delete ", n, " buffers");
	for (int i=0; i<n; i++) {
		bufferList.deleteBuffer(buffers[i]);
		/* TODO: Generate errors */ 
	}
}

ALboolean alIsBuffer(ALuint buffer)
{
    DEBUGLOGCALL(LCF_OPENAL);
	return bufferList.isBuffer(buffer);
}

void alBufferData(ALuint buffer, ALenum format, const ALvoid *data, ALsizei size, ALsizei freq)
{
    debuglog(LCF_OPENAL, __func__, "call - copy buffer data of format ", format, ", size ", size, " and frequency ", freq, "into buffer ", buffer);
	AudioBuffer* ab = bufferList.getBuffer(buffer);
    if (ab == nullptr)
        return;

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
	AudioBuffer* ab = bufferList.getBuffer(buffer);
    if (ab == nullptr)
        return;

    switch(pname) {
        case AL_FREQUENCY:
            *value = ab->frequency;
            return;
        case AL_BITS:
            *value = ab->bitDepth;
            return;
        case AL_CHANNELS:
            *value = ab->nbChannels;
            return;
        case AL_SIZE:
            *value = ab->size;
            return;
        default:
            return;
    }
}

void alGetBufferiv(ALuint buffer, ALenum pname, ALint *values)
{
    DEBUGLOGCALL(LCF_OPENAL);
    switch(pname) {
        case AL_FREQUENCY:
        case AL_BITS:
        case AL_CHANNELS:
        case AL_SIZE:
            alGetBufferi(buffer, pname, values);
        default:
            return;
    }
}

/*** Source functions ***/

void alGenSources(ALsizei n, ALuint *sources)
{
    debuglog(LCF_OPENAL, __func__, "call - generate ", n, " sources");
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
    debuglog(LCF_OPENAL, __func__, "call - delete ", n, " sources");
	for (int i=0; i<n; i++) {
		bufferList.deleteBuffer(sources[i]);
        /* TODO: The doc says if the source is deleted when playing, the source is stopped first */
		/* TODO: Generate errors */ 
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
    if (ab == nullptr)
        return;

    switch(param) {
        case AL_GAIN:
            ab->volume = value;
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
                value *= (ALfloat) (ab->nextBuffer->frequency * ab->nextBuffer->nbChannels * ab->nextBuffer->bitDepth / 8);
                ab->setPosition((int)value);
            }
            break;
        case AL_SAMPLE_OFFSET:
            /* We fetch the buffer format of the source.
             * Normally, all buffers from a queue share the exact same format.
             */
            if (ab->nextBuffer != nullptr) {
                value *= (ALfloat) (ab->nextBuffer->nbChannels * ab->nextBuffer->bitDepth / 8);
                ab->setPosition((int)value);
            }
            break;
        case AL_BYTE_OFFSET:
            ab->setPosition((int)value);
            break;
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
    debuglog(LCF_OPENAL, "Operation not supported");
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
            ab->looping = value;
            /* FIXME: Propagate looping to buffers here? */
            break;
        case AL_BUFFER:
            /* Bind a buffer to the source */
            state = ab->getState();

            if ((state == SOURCE_PLAYING) || (state == SOURCE_PAUSED)) {
                ALSETERROR(AL_INVALID_OPERATION);
            }

            if ((state == SOURCE_INITIAL) || (state == SOURCE_STOPPED)) {
                if (value == 0) {
                    /* Unbind buffer from source */
                    ab->nextBuffer = nullptr;
                    ab->processed = false;
                }
                else {
                    bindab = bufferList.getBuffer(value);
                    if (bindab == nullptr) {
                        ALSETERROR(AL_INVALID_VALUE);
                        return;
                    }
                    ab->nextBuffer = bindab;
                    ab->processed = true;
                    bindab->nextBuffer = nullptr;
                    bindab->state = state;
                }
            }
            break;
        case AL_SOURCE_STATE:
            switch(value) {
                case AL_INITIAL:
                    ab->changeState(SOURCE_INITIAL);
                    break;
                case AL_PLAYING:
                    ab->changeState(SOURCE_PLAYING);
                    break;
                case AL_PAUSED:
                    ab->changeState(SOURCE_PAUSED);
                    break;
                case AL_STOPPED:
                    ab->changeState(SOURCE_STOPPED);
                    break;
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
            break;;
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
    debuglog(LCF_OPENAL, "Operation not supported");
}

void alGetSourcef(ALuint source, ALenum param, ALfloat *value)
{
    DEBUGLOGCALL(LCF_OPENAL);
    AudioBuffer* ab = bufferList.getBuffer(source);
    if (ab == nullptr)
        return;

    switch(param) {
        case AL_GAIN:
            *value = ab->volume;
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
            }
            break;
        case AL_BYTE_OFFSET:
            *value = (ALfloat) ab->getPosition();
            break;
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
    debuglog(LCF_OPENAL, "Operation not supported");
}

void alGetSourcei(ALuint source, ALenum param, ALint *value)
{
    DEBUGLOGCALL(LCF_OPENAL);
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
                    break;
                case SOURCE_PLAYING:
                    *value = AL_PLAYING;
                    break;
                case SOURCE_PAUSED:
                    *value = AL_PAUSED;
                    break;
                case SOURCE_STOPPED:
                    *value = AL_STOPPED;
                    break;
            }
            break;
        case AL_SOURCE_RELATIVE:
            debuglog(LCF_OPENAL, "Operation not supported");
            break;
        case AL_BUFFERS_QUEUED:
            *value = ab->nbQueue();
            break;
        case AL_BUFFERS_PROCESSED:
            state = ab->getState();
            if (state == SOURCE_STOPPED)
                *value = ab->nbQueue();
            else if (state == SOURCE_INITIAL)
                *value = 0;
            else
                *value = ab->nbQueueProcessed();
            break;
        case AL_SEC_OFFSET:
        case AL_SAMPLE_OFFSET:
        case AL_BYTE_OFFSET:
            ALfloat res;
            alGetSourcef(source, param, &res);
            *value = (ALint) res;
            break;
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
    debuglog(LCF_OPENAL, "Operation not supported");
}

void alSourcePlay(ALuint source)
{
    DEBUGLOGCALL(LCF_OPENAL);
    AudioBuffer* ab = bufferList.getBuffer(source);
    if (ab == nullptr)
        return;

    if (ab->state == SOURCE_PLAYING) {
        /* TODO: Restart the play from the beginning */
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

    ab->changeState(SOURCE_INITIAL);
    /* TODO: What is exactly rewinding for queued buffers?? */
}

void alSourceRewindv(ALsizei n, ALuint *sources)
{
    DEBUGLOGCALL(LCF_OPENAL);
    for (int i=0; i<n; i++)
        alSourceRewind(sources[i]);
}

void alSourceQueueBuffers(ALuint source, ALsizei n, ALuint* buffers)
{
    DEBUGLOGCALL(LCF_OPENAL);
    AudioBuffer* ab = bufferList.getBuffer(source);
    if (ab == nullptr)
        return;

    /* Check if the source has a static buffer attached */
    if (ab->source == SOURCE_STATIC)
        return;

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
    if (processedBuffers < n)
        return;

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
    if (param == AL_GAIN)
        *value = bufferList.outVolume;
}

void alGetListener3f(ALenum param, ALfloat *v1, ALfloat *v2, ALfloat *v3)
{
    DEBUGLOGCALL(LCF_OPENAL);
    debuglog(LCF_OPENAL, "Operation not supported");
}

void alGetListenerfv(ALenum param, ALfloat *values)
{
    DEBUGLOGCALL(LCF_OPENAL);
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





