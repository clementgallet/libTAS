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

#include "al.h"
#include "efx.h"
#include "../../logging.h"
#include "../AudioBuffer.h"
#include "../AudioSource.h"
#include "../AudioContext.h"

#include <cstring> // strcmp

namespace libtas {

const ALchar* alGetString(ALenum param)
{
    switch(param) {
        case AL_VENDOR:
            return "libTAS_AL_vendor";
        case AL_VERSION:
            return "libTAS_AL_version";
        case AL_RENDERER:
            return "libTAS_AL_renderer";
        case AL_EXTENSIONS:
            return "";
    }
    return "";
}

ALenum alError;
void alSetError(ALenum error)
{
    if (alError == AL_NO_ERROR)
        alError = error;
}

ALenum alGetError(ALvoid)
{
    debuglogstdio(LCF_SOUND, "%s call, returning %d", __func__, alError);
    ALenum err = alError;
    alError = AL_NO_ERROR;
    return err;
}

#define CHECKVAL(x) do { \
    if (x) break; \
    alSetError(AL_INVALID_VALUE); \
    return; \
} while(0)

ALboolean alIsExtensionPresent(const ALchar *extname)
{
    debuglogstdio(LCF_SOUND, "%s call with extname %s", __func__, extname);

    if (strcmp(extname, ALC_EXT_EFX_NAME) == 0) {
        return AL_TRUE;
    }

    return AL_FALSE;
}

/* Little helper macro */
#define CHECK_RETURN_MY_FUNCTION(name) \
    if (strcmp(fname, #name) == 0) { \
        return reinterpret_cast<void*>(my##name); \
    }

void* alGetProcAddress(const ALchar *fname)
{
    debuglogstdio(LCF_SOUND, "%s call with name %s", __func__, fname);

    CHECK_RETURN_MY_FUNCTION(alGenEffects)
    CHECK_RETURN_MY_FUNCTION(alDeleteEffects)
    CHECK_RETURN_MY_FUNCTION(alIsEffect)
    CHECK_RETURN_MY_FUNCTION(alEffecti)
    CHECK_RETURN_MY_FUNCTION(alEffectiv)
    CHECK_RETURN_MY_FUNCTION(alEffectf)
    CHECK_RETURN_MY_FUNCTION(alEffectfv)
    CHECK_RETURN_MY_FUNCTION(alGetEffecti)
    CHECK_RETURN_MY_FUNCTION(alGetEffectiv)
    CHECK_RETURN_MY_FUNCTION(alGetEffectf)
    CHECK_RETURN_MY_FUNCTION(alGetEffectfv)

    CHECK_RETURN_MY_FUNCTION(alGenFilters)
    CHECK_RETURN_MY_FUNCTION(alDeleteFilters)
    CHECK_RETURN_MY_FUNCTION(alIsFilter)
    CHECK_RETURN_MY_FUNCTION(alFilteri)
    CHECK_RETURN_MY_FUNCTION(alFilteriv)
    CHECK_RETURN_MY_FUNCTION(alFilterf)
    CHECK_RETURN_MY_FUNCTION(alFilterfv)
    CHECK_RETURN_MY_FUNCTION(alGetFilteri)
    CHECK_RETURN_MY_FUNCTION(alGetFilteriv)
    CHECK_RETURN_MY_FUNCTION(alGetFilterf)
    CHECK_RETURN_MY_FUNCTION(alGetFilterfv)

    CHECK_RETURN_MY_FUNCTION(alGenAuxiliaryEffectSlots)
    CHECK_RETURN_MY_FUNCTION(alDeleteAuxiliaryEffectSlots)
    CHECK_RETURN_MY_FUNCTION(alIsAuxiliaryEffectSlot)
    CHECK_RETURN_MY_FUNCTION(alAuxiliaryEffectSloti)
    CHECK_RETURN_MY_FUNCTION(alAuxiliaryEffectSlotiv)
    CHECK_RETURN_MY_FUNCTION(alAuxiliaryEffectSlotf)
    CHECK_RETURN_MY_FUNCTION(alAuxiliaryEffectSlotfv)
    CHECK_RETURN_MY_FUNCTION(alGetAuxiliaryEffectSloti)
    CHECK_RETURN_MY_FUNCTION(alGetAuxiliaryEffectSlotiv)
    CHECK_RETURN_MY_FUNCTION(alGetAuxiliaryEffectSlotf)
    CHECK_RETURN_MY_FUNCTION(alGetAuxiliaryEffectSlotfv)

    return nullptr;
}

void alGenBuffers(ALsizei n, ALuint *buffers)
{
    debuglogstdio(LCF_SOUND, "%s call - generate %d buffers", __func__, n);

    std::lock_guard<std::mutex> lock(audiocontext.mutex);
    for (int i=0; i<n; i++) {
        int id = audiocontext.createBuffer();
        if (id > 0)
            buffers[i] = (ALuint) id;
        /* TODO: else generate an error */
    }
}

void alDeleteBuffers(ALsizei n, ALuint *buffers)
{
    debuglogstdio(LCF_SOUND, "%s call - delete %d buffers", __func__, n);

    std::lock_guard<std::mutex> lock(audiocontext.mutex);
    for (int i=0; i<n; i++) {
        /* Check if all buffers exist before removing any. */
        if (! audiocontext.isBuffer(buffers[i])) {
            alSetError(AL_INVALID_NAME);
            return;
        }
    }
    for (int i=0; i<n; i++) {
        audiocontext.deleteBuffer(buffers[i]);
    }
}

ALboolean alIsBuffer(ALuint buffer)
{
    DEBUGLOGCALL(LCF_SOUND);
    std::lock_guard<std::mutex> lock(audiocontext.mutex);
    return audiocontext.isBuffer(buffer);
}

void alBufferData(ALuint buffer, ALenum format, const ALvoid *data, ALsizei size, ALsizei freq)
{
    debuglogstdio(LCF_SOUND, "%s call - copy buffer data of format %d, size %d and frequency %d into buffer %d", __func__, format, size, freq, buffer);

    std::lock_guard<std::mutex> lock(audiocontext.mutex);

	auto ab = audiocontext.getBuffer(buffer);
    if (ab == nullptr) {
        alSetError(AL_INVALID_NAME);
        return;
    }

    /* Fill the buffer informations */
    ab->size = size;
    ab->frequency = freq;
    switch(format) {
        case AL_FORMAT_MONO8:
            ab->format = AudioBuffer::SAMPLE_FMT_U8;
            ab->nbChannels = 1;
            break;
        case AL_FORMAT_MONO16:
            ab->format = AudioBuffer::SAMPLE_FMT_S16;
            ab->nbChannels = 1;
            break;
        case AL_FORMAT_STEREO8:
            ab->format = AudioBuffer::SAMPLE_FMT_U8;
            ab->nbChannels = 2;
            break;
        case AL_FORMAT_STEREO16:
            ab->format = AudioBuffer::SAMPLE_FMT_S16;
            ab->nbChannels = 2;
            break;
        case AL_FORMAT_MONO_FLOAT32:
            ab->format = AudioBuffer::SAMPLE_FMT_FLT;
            ab->nbChannels = 1;
            break;
        case AL_FORMAT_STEREO_FLOAT32:
            ab->format = AudioBuffer::SAMPLE_FMT_FLT;
            ab->nbChannels = 2;
            break;
        case AL_FORMAT_MONO_DOUBLE_EXT:
            ab->format = AudioBuffer::SAMPLE_FMT_DBL;
            ab->nbChannels = 1;
            break;
        case AL_FORMAT_STEREO_DOUBLE_EXT:
            ab->format = AudioBuffer::SAMPLE_FMT_DBL;
            ab->nbChannels = 2;
            break;
        case AL_FORMAT_MONO_MSADPCM_SOFT:
            ab->format = AudioBuffer::SAMPLE_FMT_MSADPCM;
            ab->nbChannels = 1;
            if (ab->blockSamples == 0)
                ab->blockSamples = 64;
            break;
        case AL_FORMAT_STEREO_MSADPCM_SOFT:
            ab->format = AudioBuffer::SAMPLE_FMT_MSADPCM;
            ab->nbChannels = 2;
            if (ab->blockSamples == 0)
                ab->blockSamples = 64;
            break;
        default:
            debuglogstdio(LCF_SOUND | LCF_ERROR, "Unsupported format: %d", format);
            return;
    }

    ab->update();

    /* Check for size validity */
    if (! ab->checkSize()) {
        alSetError(AL_INVALID_VALUE);
        return;
    }

    /* Copy the data into our buffer */
    ab->samples.clear();
    ab->samples.insert(ab->samples.end(), &((uint8_t*)data)[0], &((uint8_t*)data)[size]);

}

void alBufferf(ALuint buffer, ALenum param, ALfloat value)
{
    DEBUGLOGCALL(LCF_SOUND);
    debuglogstdio(LCF_SOUND, "Operation not supported");
}

void alBuffer3f(ALuint buffer, ALenum param, ALfloat value1, ALfloat value2, ALfloat value3)
{
    DEBUGLOGCALL(LCF_SOUND);
    debuglogstdio(LCF_SOUND, "Operation not supported");
}

void alBufferfv(ALuint buffer, ALenum param, const ALfloat *values)
{
    DEBUGLOGCALL(LCF_SOUND);
    debuglogstdio(LCF_SOUND, "Operation not supported");
}

void alBufferi(ALuint buffer, ALenum param, ALint value)
{
    DEBUGLOGCALL(LCF_SOUND);
    std::lock_guard<std::mutex> lock(audiocontext.mutex);
    auto ab = audiocontext.getBuffer(buffer);
    if (ab == nullptr) {
        alSetError(AL_INVALID_NAME);
        return;
    }

    switch(param) {
        case AL_UNPACK_BLOCK_ALIGNMENT_SOFT:
            CHECKVAL(value >= 0);
            debuglogstdio(LCF_SOUND, "  Set block alignment %d", value);
            ab->blockSamples = value;
            ab->update();
            break;
        default:
            debuglogstdio(LCF_SOUND, "  Operation not supported: %d", param);
            return;
    }
}

void alBuffer3i(ALuint buffer, ALenum param, ALint value1, ALint value2, ALint value3)
{
    DEBUGLOGCALL(LCF_SOUND);
    debuglogstdio(LCF_SOUND, "Operation not supported");
}

void alBufferiv(ALuint buffer, ALenum param, const ALint *values)
{
    DEBUGLOGCALL(LCF_SOUND);
    if (values == nullptr) {
        alSetError(AL_INVALID_VALUE);
        return;
    }

    auto ab = audiocontext.getBuffer(buffer);
    if (ab == nullptr) {
        alSetError(AL_INVALID_NAME);
        return;
    }

    switch(param) {
        case AL_UNPACK_BLOCK_ALIGNMENT_SOFT:
            CHECKVAL(*values >= 0.0f);
            alBufferi(buffer, param, *values);
            break;
        case AL_LOOP_POINTS_SOFT:
            CHECKVAL(values[0] >= 0.0f && values[1] > values[0] && values[1] <= ab->sampleSize);
            debuglogstdio(LCF_SOUND, "  Set loop points %d -> %d", values[0], values[1]);
            ab->loop_point_beg = values[0];
            ab->loop_point_end = values[1];
            break;
        default:
            debuglogstdio(LCF_SOUND, "  Operation not supported: %d", param);
            return;
    }
}


void alGetBufferi(ALuint buffer, ALenum pname, ALint *value)
{
    DEBUGLOGCALL(LCF_SOUND);

    if (value == nullptr) {
        return;
    }

    std::lock_guard<std::mutex> lock(audiocontext.mutex);
	auto ab = audiocontext.getBuffer(buffer);
    if (ab == nullptr) {
        alSetError(AL_INVALID_NAME);
        return;
    }

    switch(pname) {
        case AL_FREQUENCY:
            *value = ab->frequency;
            debuglogstdio(LCF_SOUND, "  Get frequency of %d", *value);
            return;
        case AL_BITS:
            *value = ab->bitDepth;
            debuglogstdio(LCF_SOUND, "  Get bit depth of %d", *value);
            return;
        case AL_CHANNELS:
            *value = ab->nbChannels;
            debuglogstdio(LCF_SOUND, "  Get channel number of %d", *value);
            return;
        case AL_SIZE:
            *value = ab->size;
            debuglogstdio(LCF_SOUND, "  Get size of %d", *value);
            return;
        case AL_UNPACK_BLOCK_ALIGNMENT_SOFT:
            *value = ab->blockSamples;
            debuglogstdio(LCF_SOUND, "  Get block alignment of %d", *value);
            return;
        default:
            alSetError(AL_INVALID_VALUE);
            return;
    }
}

void alGetBufferiv(ALuint buffer, ALenum pname, ALint *values)
{
    DEBUGLOGCALL(LCF_SOUND);

    if (values == nullptr) {
        return;
    }

    std::lock_guard<std::mutex> lock(audiocontext.mutex);
    auto ab = audiocontext.getBuffer(buffer);
    if (ab == nullptr) {
        alSetError(AL_INVALID_NAME);
        return;
    }

    switch(pname) {
        case AL_FREQUENCY:
            *values = ab->frequency;
            debuglogstdio(LCF_SOUND, "  Get frequency of %d", *values);
            return;
        case AL_BITS:
            *values = ab->bitDepth;
            debuglogstdio(LCF_SOUND, "  Get bit depth of %d", *values);
            return;
        case AL_CHANNELS:
            *values = ab->nbChannels;
            debuglogstdio(LCF_SOUND, "  Get channel number of %d", *values);
            return;
        case AL_SIZE:
            *values = ab->size;
            debuglogstdio(LCF_SOUND, "  Get size of %d", *values);
            return;
        case AL_LOOP_POINTS_SOFT:
            values[0] = ab->loop_point_beg;
            values[1] = ab->loop_point_end;
            debuglogstdio(LCF_SOUND, "  Get loop points %d -> %d", values[0], values[1]);
            break;
        default:
            alSetError(AL_INVALID_VALUE);
            return;
    }
}

/*** Source functions ***/

void alGenSources(ALsizei n, ALuint *sources)
{
    debuglogstdio(LCF_SOUND, "%s call - generate %d sources", __func__, n);
    std::lock_guard<std::mutex> lock(audiocontext.mutex);
	for (int i=0; i<n; i++) {
		int id = audiocontext.createSource();
		if (id > 0) {
			sources[i] = (ALuint) id;
        }
        else {
            alSetError(AL_INVALID_VALUE);
            return;
        }
	}
}

void alDeleteSources(ALsizei n, ALuint *sources)
{
    debuglogstdio(LCF_SOUND, "%s call - delete %d sources", __func__, n);
    std::lock_guard<std::mutex> lock(audiocontext.mutex);
	for (int i=0; i<n; i++) {
        /* Check if all sources exist before removing any. */
	    if (! audiocontext.isSource(sources[i])) {
            alSetError(AL_INVALID_NAME);
            return;
        }
    }
	for (int i=0; i<n; i++) {
        /* If the source is deleted when playing, the source must be stopped first */
        auto as = audiocontext.getSource(sources[i]);
        if (as->state == AudioSource::SOURCE_PLAYING)
            as->state = AudioSource::SOURCE_STOPPED;
		audiocontext.deleteSource(sources[i]);
	}
}

ALboolean alIsSource(ALuint source)
{
    DEBUGLOGCALL(LCF_SOUND);
    std::lock_guard<std::mutex> lock(audiocontext.mutex);
	return audiocontext.isSource(source);
}

void alSourcef(ALuint source, ALenum param, ALfloat value)
{
    debuglogstdio(LCF_SOUND, "%s called with source %d", __func__, source);
    std::lock_guard<std::mutex> lock(audiocontext.mutex);
    auto as = audiocontext.getSource(source);
    if (!as) {
        alSetError(AL_INVALID_NAME);
        return;
    }

    std::shared_ptr<AudioBuffer> ab;
    switch(param) {
        case AL_GAIN:
            CHECKVAL(value >= 0.0f);
            as->volume = value;
            debuglogstdio(LCF_SOUND, "  Set gain of %f", value);
            break;
        case AL_PITCH:
            CHECKVAL(value >= 0.0f);
            if (as->pitch != value) {
                as->dirty();
            }
            as->pitch = value;
            debuglogstdio(LCF_SOUND, "  Set pitch of %f", value);
            break;
        case AL_REFERENCE_DISTANCE:
            CHECKVAL(value >= 0.0f);
            if (value != 1.0) {
                debuglogstdio(LCF_SOUND, "  Set reference distance to %f. Operation not supported", value);
            }
            break;
        case AL_SEC_OFFSET:
            CHECKVAL(value >= 0.0f);
            /* We fetch the buffer format of the source.
             * Normally, all buffers from a queue share the exact same format.
             */
            if (! as->buffer_queue.empty()) {
                ab = as->buffer_queue[0];
                debuglogstdio(LCF_SOUND, "  Set position of %f seconds", value);
                value *= (ALfloat) ab->frequency;
                as->setPosition((int)value);
            }
            break;
        case AL_SAMPLE_OFFSET:
            CHECKVAL(value >= 0.0f);
            /* We fetch the buffer format of the source.
             * Normally, all buffers from a queue share the exact same format.
             */
            debuglogstdio(LCF_SOUND, "  Set position of %f samples", value);
            as->setPosition((int)value);
            break;
        case AL_BYTE_OFFSET:
            CHECKVAL(value >= 0.0f);
            if (! as->buffer_queue.empty()) {
                ab = as->buffer_queue[0];
                value /= (ALfloat) ab->alignSize;
                debuglogstdio(LCF_SOUND, "  Set position of %f bytes", value);
                as->setPosition((int)value);
            }
            break;
        /* Unsupported operations */
        case AL_MIN_GAIN:
            CHECKVAL(value >= 0.0f);
            debuglogstdio(LCF_SOUND, "Operation not supported: %d", param);
            break;
        case AL_MAX_GAIN:
            CHECKVAL(value >= 0.0f);
            debuglogstdio(LCF_SOUND, "Operation not supported: %d", param);
            break;
        case AL_MAX_DISTANCE:
            CHECKVAL(value >= 0.0f);
            debuglogstdio(LCF_SOUND, "Operation not supported: %d", param);
            break;
        case AL_ROLLOFF_FACTOR:
            CHECKVAL(value >= 0.0f);
            debuglogstdio(LCF_SOUND, "Operation not supported: %d", param);
            break;
        case AL_CONE_OUTER_GAIN:
            CHECKVAL(value >= 0.0f && value <= 1.0f);
            debuglogstdio(LCF_SOUND, "Operation not supported: %d", param);
            break;
        case AL_CONE_INNER_ANGLE:
            CHECKVAL(value >= 0.0f && value <= 360.0f);
            debuglogstdio(LCF_SOUND, "Operation not supported: %d", param);
            break;
        case AL_CONE_OUTER_ANGLE:
            CHECKVAL(value >= 0.0f && value <= 360.0f);
            debuglogstdio(LCF_SOUND, "Operation not supported: %d", param);
            break;
        case AL_AUXILIARY_SEND_FILTER:
            debuglogstdio(LCF_SOUND, "Operation not supported: %d", param);
            break;
        default:
            debuglogstdio(LCF_SOUND, "  Unknown param %d", param);
            alSetError(AL_INVALID_OPERATION);
            return;
    }
}

void alSource3f(ALuint source, ALenum param, ALfloat v1, ALfloat v2, ALfloat v3)
{
    debuglogstdio(LCF_SOUND, "%s called with source %d", __func__, source);
    switch(param) {
        case AL_DIRECTION:
            debuglogstdio(LCF_SOUND, "Setting direction not supported");
            break;
        case AL_VELOCITY:
            debuglogstdio(LCF_SOUND, "Setting velocity not supported");
            break;
        default:
            debuglogstdio(LCF_SOUND, "Operation not supported");
            break;
    }
}

void alSourcefv(ALuint source, ALenum param, ALfloat *values)
{
    debuglogstdio(LCF_SOUND, "%s called with source %d", __func__, source);
    if (values == nullptr) {
        alSetError(AL_INVALID_VALUE);
        return;
    }

    switch(param) {
        case AL_POSITION:
            debuglogstdio(LCF_SOUND, "Setting position not supported");
            break;
        default:
            alSourcef(source, param, *values);
            break;
    }
}

void alSourcei(ALuint source, ALenum param, ALint value)
{
    debuglogstdio(LCF_SOUND, "%s called with source %d", __func__, source);
    std::lock_guard<std::mutex> lock(audiocontext.mutex);
    auto as = audiocontext.getSource(source);
    if (!as) {
        alSetError(AL_INVALID_NAME);
        return;
    }

    std::shared_ptr<AudioBuffer> ab;
    switch(param) {
        case AL_LOOPING:
            CHECKVAL(value == AL_FALSE || value == AL_TRUE);
            debuglogstdio(LCF_SOUND, "  Set looping of %d", value);
            if (value == AL_TRUE)
                as->looping = true;
            else if (value == AL_FALSE)
                as->looping = false;
            else
                alSetError(AL_INVALID_VALUE);
            break;
        case AL_BUFFER:
            /* Bind a buffer to the source */

            if ((as->state == AudioSource::SOURCE_PLAYING) || (as->state == AudioSource::SOURCE_PAUSED)) {
                alSetError(AL_INVALID_OPERATION);
                return;
            }

            if (value == 0) {
                /* Unbind buffer from source */
                as->source = AudioSource::SOURCE_UNDETERMINED;
                as->buffer_queue.clear();
                debuglogstdio(LCF_SOUND, "  Unbind buffer");
            }
            else {
                ab = audiocontext.getBuffer(value);
                if (!ab) {
                    alSetError(AL_INVALID_VALUE);
                    return;
                }
                as->buffer_queue.clear();
                as->buffer_queue.push_back(ab);
                as->source = AudioSource::SOURCE_STATIC;
                debuglogstdio(LCF_SOUND, "  Bind to buffer %d", value);
            }
            break;
        case AL_SEC_OFFSET:
            CHECKVAL(value >= 0);
            /* We fetch the buffer format of the source.
             * Normally, all buffers from a queue share the exact same format.
             */
            if (! as->buffer_queue.empty()) {
                ab = as->buffer_queue[0];
                debuglogstdio(LCF_SOUND, "  Set position of %d seconds", value);
                value *= static_cast<ALint>(ab->frequency);
                as->setPosition(static_cast<int>(value));
            }
            break;
        case AL_SAMPLE_OFFSET:
            CHECKVAL(value >= 0);
            /* We fetch the buffer format of the source.
             * Normally, all buffers from a queue share the exact same format.
             */
            debuglogstdio(LCF_SOUND, "  Set position of %d samples", value);
            as->setPosition(static_cast<int>(value));
            break;
        case AL_BYTE_OFFSET:
            CHECKVAL(value >= 0);
            if (! as->buffer_queue.empty()) {
                ab = as->buffer_queue[0];
                value /= static_cast<ALint>(ab->alignSize);
                debuglogstdio(LCF_SOUND, "  Set position of %d bytes", value);
                as->setPosition(static_cast<int>(value));
            }
            break;
        case AL_SOURCE_RELATIVE:
            CHECKVAL(value == AL_FALSE || value == AL_TRUE);
            debuglogstdio(LCF_SOUND, "Operation not supported: %d", param);
            break;
        case AL_CONE_INNER_ANGLE:
            CHECKVAL(value >= 0 && value <= 360);
            debuglogstdio(LCF_SOUND, "Operation not supported: %d", param);
            break;
        case AL_CONE_OUTER_ANGLE:
            CHECKVAL(value >= 0 && value <= 360);
            debuglogstdio(LCF_SOUND, "Operation not supported: %d", param);
            break;
        case AL_DIRECT_FILTER:
        debuglogstdio(LCF_SOUND, "Operation not supported: %d", param);
            break;
        case AL_DIRECT_FILTER_GAINHF_AUTO:
            CHECKVAL(value == AL_FALSE || value == AL_TRUE);
            debuglogstdio(LCF_SOUND, "Operation not supported: %d", param);
            break;
        case AL_AUXILIARY_SEND_FILTER_GAIN_AUTO:
            CHECKVAL(value == AL_FALSE || value == AL_TRUE);
            debuglogstdio(LCF_SOUND, "Operation not supported: %d", param);
            break;
        case AL_AUXILIARY_SEND_FILTER_GAINHF_AUTO:
            CHECKVAL(value == AL_FALSE || value == AL_TRUE);
            debuglogstdio(LCF_SOUND, "Operation not supported: %d", param);
            break;
        default:
            debuglogstdio(LCF_SOUND, "  Unknown param %d", param);
            alSetError(AL_INVALID_OPERATION);
            return;
    }
}

void alSource3i(ALuint source, ALenum param, ALint v1, ALint v2, ALint v3)
{
    debuglogstdio(LCF_SOUND, "%s called with source %d", __func__, source);
    switch(param) {
        case AL_AUXILIARY_SEND_FILTER:
            debuglogstdio(LCF_SOUND, "Operation not supported");
            return;
    }
    debuglogstdio(LCF_SOUND, "Operation not supported");
}

void alSourceiv(ALuint source, ALenum param, ALint *values)
{
    debuglogstdio(LCF_SOUND, "%s called with source %d", __func__, source);
    if (values == nullptr) {
        alSetError(AL_INVALID_VALUE);
        return;
    }
    alSourcei(source, param, *values);
}

void alGetSourcef(ALuint source, ALenum param, ALfloat *value)
{
    DEBUGLOGCALL(LCF_SOUND);

    if (value == nullptr) {
        return;
    }

    std::lock_guard<std::mutex> lock(audiocontext.mutex);
    auto as = audiocontext.getSource(source);
    if (!as)
        return;

    std::shared_ptr<AudioBuffer> ab;
    switch(param) {
        case AL_GAIN:
            *value = as->volume;
            debuglogstdio(LCF_SOUND, "  Get gain of %f", *value);
            break;
        case AL_PITCH:
            *value = as->pitch;
            debuglogstdio(LCF_SOUND, "  Get pitch of %f", *value);
            break;
        case AL_MIN_GAIN:
        case AL_MAX_GAIN:
        case AL_MAX_DISTANCE:
        case AL_ROLLOFF_FACTOR:
        case AL_CONE_OUTER_GAIN:
        case AL_CONE_INNER_ANGLE:
        case AL_CONE_OUTER_ANGLE:
        case AL_REFERENCE_DISTANCE:
        case AL_AIR_ABSORPTION_FACTOR:
        case AL_ROOM_ROLLOFF_FACTOR:
        case AL_CONE_OUTER_GAINHF:
            debuglogstdio(LCF_SOUND, "Operation not supported: %d", param);
            break;
        case AL_SEC_OFFSET:
            /* We fetch the buffer format of the source.
             * Normally, all buffers from a queue share the exact same format.
             */
            if (! as->buffer_queue.empty()) {
                ab = as->buffer_queue[0];
                *value = static_cast<ALfloat>(as->getPosition()) / ab->frequency;
                debuglogstdio(LCF_SOUND, "  Get position of %f seconds", *value);
            }
            break;
        case AL_SAMPLE_OFFSET:
            *value = (ALfloat) as->getPosition();
            debuglogstdio(LCF_SOUND, "  Get position of %f samples", *value);
            break;
        case AL_BYTE_OFFSET:
            /* We fetch the buffer format of the source.
             * Normally, all buffers from a queue share the exact same format.
             */
            if (! as->buffer_queue.empty()) {
                ab = as->buffer_queue[0];
                *value = static_cast<ALfloat>(as->getPosition()) * ab->alignSize;
                debuglogstdio(LCF_SOUND, "  Get position of %f bytes", *value);
            }
            break;
        default:
            debuglogstdio(LCF_SOUND, "  Unknown param %d", param);
            alSetError(AL_INVALID_OPERATION);
            return;
    }
}

void alGetSource3f(ALuint source, ALenum param, ALfloat *v1, ALfloat *v2, ALfloat *v3)
{
    DEBUGLOGCALL(LCF_SOUND);
    debuglogstdio(LCF_SOUND, "Operation not supported");
}

void alGetSourcefv(ALuint source, ALenum param, ALfloat *values)
{
    DEBUGLOGCALL(LCF_SOUND);
    alGetSourcef(source, param, values);
}

void alGetSourcei(ALuint source, ALenum param, ALint *value)
{
    debuglogstdio(LCF_SOUND, "%s call for source %d", __func__, source);

    if (value == nullptr) {
        return;
    }

    std::lock_guard<std::mutex> lock(audiocontext.mutex);
    auto as = audiocontext.getSource(source);
    if (!as) {
        alSetError(AL_INVALID_NAME);
        return;
    }

    std::shared_ptr<AudioBuffer> ab;
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
                case AudioSource::SOURCE_INITIAL:
                    *value = AL_INITIAL;
                    debuglogstdio(LCF_SOUND, "  Get source state INITIAL");
                    break;
                case AudioSource::SOURCE_PREPARED:
                case AudioSource::SOURCE_PLAYING:
                    *value = AL_PLAYING;
                    debuglogstdio(LCF_SOUND, "  Get source state PLAYING");
                    break;
                case AudioSource::SOURCE_PAUSED:
                    *value = AL_PAUSED;
                    debuglogstdio(LCF_SOUND, "  Get source state PAUSED");
                    break;
                case AudioSource::SOURCE_STOPPED:
                    *value = AL_STOPPED;
                    debuglogstdio(LCF_SOUND, "  Get source state STOPPED");
                    break;
                default:
                    alSetError(AL_INVALID_VALUE);
                    break;
            }
            break;
        case AL_SOURCE_TYPE:
            switch(as->source) {
                case AudioSource::SOURCE_UNDETERMINED:
                    *value = AL_UNDETERMINED;
                    debuglogstdio(LCF_SOUND, "  Get source type UNDETERMINED");
                    break;
                case AudioSource::SOURCE_STATIC:
                    *value = AL_STATIC;
                    debuglogstdio(LCF_SOUND, "  Get source type STATIC");
                    break;
                case AudioSource::SOURCE_STREAMING:
                    *value = AL_STREAMING;
                    debuglogstdio(LCF_SOUND, "  Get source type STREAMING");
                    break;
                default:
                    alSetError(AL_INVALID_VALUE);
                    break;
            }
            break;
        case AL_SOURCE_RELATIVE:
        case AL_DIRECT_FILTER_GAINHF_AUTO:
        case AL_AUXILIARY_SEND_FILTER_GAIN_AUTO:
        case AL_AUXILIARY_SEND_FILTER_GAINHF_AUTO:
            debuglogstdio(LCF_SOUND, "Operation not supported: %d", param);
            break;
        case AL_BUFFERS_QUEUED:
            *value = as->nbQueue();
            debuglogstdio(LCF_SOUND, "  Get number of queued buffers of %d", *value);
            break;
        case AL_BUFFERS_PROCESSED:
            if (as->state == AudioSource::SOURCE_STOPPED)
                *value = as->nbQueue();
            else if (as->state == AudioSource::SOURCE_INITIAL)
                *value = 0;
            else
                *value = as->nbQueueProcessed();
            debuglogstdio(LCF_SOUND, "  Get number of processed queued buffers of %d", *value);
            break;
        case AL_SEC_OFFSET:
            /* We fetch the buffer format of the source.
             * Normally, all buffers from a queue share the exact same format.
             */
            if (! as->buffer_queue.empty()) {
                ab = as->buffer_queue[0];
                *value = as->getPosition() / ab->frequency;
                debuglogstdio(LCF_SOUND, "  Get position of %d seconds", *value);
            }
            break;
        case AL_SAMPLE_OFFSET:
            *value = static_cast<ALint>(as->getPosition());
            debuglogstdio(LCF_SOUND, "  Get position of %d samples", *value);
            break;
        case AL_BYTE_OFFSET:
            /* We fetch the buffer format of the source.
             * Normally, all buffers from a queue share the exact same format.
             */
            if (! as->buffer_queue.empty()) {
                ab = as->buffer_queue[0];
                *value = as->getPosition() * ab->alignSize;
                debuglogstdio(LCF_SOUND, "  Get position of %d bytes", *value);
            }
            break;
        default:
            debuglogstdio(LCF_SOUND, "  Unknown param %d", param);
            alSetError(AL_INVALID_OPERATION);
            return;
    }
}

void alGetSource3i(ALuint source, ALenum param, ALint *v1, ALint *v2, ALint *v3)
{
    DEBUGLOGCALL(LCF_SOUND);
    debuglogstdio(LCF_SOUND, "Operation not supported");
}

void alGetSourceiv(ALuint source, ALenum param, ALint *values)
{
    DEBUGLOGCALL(LCF_SOUND);
    alGetSourcei(source, param, values);
}

void alSourcePlay(ALuint source)
{
    DEBUGLOGCALL(LCF_SOUND);
    std::lock_guard<std::mutex> lock(audiocontext.mutex);
    auto as = audiocontext.getSource(source);
    if (!as)
        return;

    if (as->state == AudioSource::SOURCE_PLAYING) {
        /* Restart the play from the beginning */
        as->setPosition(0);
    }
    as->state = AudioSource::SOURCE_PLAYING;
}

void alSourcePlayv(ALsizei n, ALuint *sources)
{
    DEBUGLOGCALL(LCF_SOUND);
    for (int i=0; i<n; i++)
        alSourcePlay(sources[i]);
}

void alSourcePause(ALuint source)
{
    DEBUGLOGCALL(LCF_SOUND);
    std::lock_guard<std::mutex> lock(audiocontext.mutex);
    auto as = audiocontext.getSource(source);
    if (!as)
        return;

    if (as->state != AudioSource::SOURCE_PLAYING) {
        /* Illegal operation. */
        return;
    }
    as->state = AudioSource::SOURCE_PAUSED;
}

void alSourcePausev(ALsizei n, ALuint *sources)
{
    DEBUGLOGCALL(LCF_SOUND);
    for (int i=0; i<n; i++)
        alSourcePause(sources[i]);
}

void alSourceStop(ALuint source)
{
    DEBUGLOGCALL(LCF_SOUND);
    std::lock_guard<std::mutex> lock(audiocontext.mutex);
    auto as = audiocontext.getSource(source);
    if (!as)
        return;

    if ((as->state == AudioSource::SOURCE_INITIAL) || (as->state == AudioSource::SOURCE_STOPPED)) {
        /* Illegal operation. */
        return;
    }
    as->rewind();
    as->state = AudioSource::SOURCE_STOPPED;
}

void alSourceStopv(ALsizei n, ALuint *sources)
{
    DEBUGLOGCALL(LCF_SOUND);
    for (int i=0; i<n; i++)
        alSourceStop(sources[i]);
}

void alSourceRewind(ALuint source)
{
    DEBUGLOGCALL(LCF_SOUND);
    std::lock_guard<std::mutex> lock(audiocontext.mutex);
    auto as = audiocontext.getSource(source);
    if (!as)
        return;

    if (as->state == AudioSource::SOURCE_INITIAL) {
        /* Illegal operation. */
        return;
    }
    as->setPosition(0);
    as->state = AudioSource::SOURCE_INITIAL;
}

void alSourceRewindv(ALsizei n, ALuint *sources)
{
    DEBUGLOGCALL(LCF_SOUND);
    for (int i=0; i<n; i++)
        alSourceRewind(sources[i]);
}

void alSourceQueueBuffers(ALuint source, ALsizei n, ALuint* buffers)
{
    debuglogstdio(LCF_SOUND, "Pushing %d buffers in the queue of source ", n, source);
    std::lock_guard<std::mutex> lock(audiocontext.mutex);
    auto as = audiocontext.getSource(source);
    if (!as)
        return;

    /* Check if the source has a static buffer attached */
    if (as->source == AudioSource::SOURCE_STATIC) {
        alSetError(AL_INVALID_OPERATION);
        return;
    }

    as->source = AudioSource::SOURCE_STREAMING;

    /* TODO: Check that all buffers have the same format */
    for (int i=0; i<n; i++) {
        auto queue_ab = audiocontext.getBuffer(buffers[i]);
        if (!queue_ab)
            return;

        as->buffer_queue.push_back(queue_ab);
        debuglogstdio(LCF_SOUND, "  Pushed buffer %d", buffers[i]);
    }
}

void alSourceUnqueueBuffers(ALuint source, ALsizei n, ALuint* buffers)
{
    DEBUGLOGCALL(LCF_SOUND);
    std::lock_guard<std::mutex> lock(audiocontext.mutex);
    auto as = audiocontext.getSource(source);
    if (!as)
        return;

    /* Check if we can unqueue that number of buffers */
    int processedBuffers;
    if (as->state == AudioSource::SOURCE_STOPPED)
        processedBuffers = as->nbQueue();
    else
        processedBuffers = as->nbQueueProcessed();
    if (processedBuffers < n) {
        alSetError(AL_INVALID_VALUE);
        return;
    }

    debuglogstdio(LCF_SOUND, "Unqueueing %d buffers out of %d", n, as->nbQueue());

    /* Save the id of the unqueued buffers */
    for (int i=0; i<n; i++) {
        buffers[i] = as->buffer_queue[i]->id;
    }

    /* Remove the buffers from the queue.
     * TODO: This is slow on a vector, maybe use forward_list?
     */
    as->buffer_queue.erase(as->buffer_queue.begin(), as->buffer_queue.begin()+n);
    if (as->state != AudioSource::SOURCE_STOPPED)
        as->queue_index -= n;
}

/*** Listener ***/

void alListenerf(ALenum param, ALfloat value)
{
    DEBUGLOGCALL(LCF_SOUND);
    if (param == AL_GAIN)
        audiocontext.outVolume = value;
}

void alListener3f(ALenum param, ALfloat v1, ALfloat v2, ALfloat v3)
{
    DEBUGLOGCALL(LCF_SOUND);
    switch(param) {
    case AL_POSITION:
        debuglogstdio(LCF_SOUND, "   Set Position to: %f, %f; %f", v1, v2, v3);
        break;
    case AL_VELOCITY:
        debuglogstdio(LCF_SOUND, "   Set Velocity to: %f, %f, %f", v1, v2, v3);
        break;
    }
    debuglogstdio(LCF_SOUND, "Operation not supported: %d", param);
}

void alListenerfv(ALenum param, ALfloat *values)
{
    DEBUGLOGCALL(LCF_SOUND);
    switch(param) {
    case AL_ORIENTATION:
        debuglogstdio(LCF_SOUND, "   Set Orientation to: %f, %f, %f, %f, %f, %f", values[0], values[1], values[2], values[3], values[4], values[5]);
        break;
    }
    debuglogstdio(LCF_SOUND, "Operation not supported: %d", param);
}

void alListeneri(ALenum param, ALint value)
{
    DEBUGLOGCALL(LCF_SOUND);
    debuglogstdio(LCF_SOUND, "Operation not supported");
}

void alListener3i(ALenum param, ALint v1, ALint v2, ALint v3)
{
    DEBUGLOGCALL(LCF_SOUND);
    debuglogstdio(LCF_SOUND, "Operation not supported");
}

void alListeneriv(ALenum param, ALint *values)
{
    DEBUGLOGCALL(LCF_SOUND);
    debuglogstdio(LCF_SOUND, "Operation not supported");
}

void alGetListenerf(ALenum param, ALfloat *value)
{
    DEBUGLOGCALL(LCF_SOUND);
    if (param == AL_GAIN) {
        if (!value) {
            // alSetError(AL_INVALID_VALUE);
            return;
        }
        *value = audiocontext.outVolume;
    }
}

void alGetListener3f(ALenum param, ALfloat *v1, ALfloat *v2, ALfloat *v3)
{
    DEBUGLOGCALL(LCF_SOUND);
    debuglogstdio(LCF_SOUND, "Operation not supported");
}

void alGetListenerfv(ALenum param, ALfloat *values)
{
    DEBUGLOGCALL(LCF_SOUND);
    if (param == AL_GAIN)
        alGetListenerf(param, values);
    else
        debuglogstdio(LCF_SOUND, "Operation not supported");
}

void alGetListeneri(ALenum param, ALint *value)
{
    DEBUGLOGCALL(LCF_SOUND);
    debuglogstdio(LCF_SOUND, "Operation not supported");
}

void alGetListener3i(ALenum param, ALint *v1, ALint *v2, ALint *v3)
{
    DEBUGLOGCALL(LCF_SOUND);
    debuglogstdio(LCF_SOUND, "Operation not supported");
}

void alGetListeneriv(ALenum param, ALint *values)
{
    DEBUGLOGCALL(LCF_SOUND);
    debuglogstdio(LCF_SOUND, "Operation not supported");
}

}
