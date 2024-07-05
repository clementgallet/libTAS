/*
    Copyright 2015-2024 Clément Gallet <clement.gallet@ens-lyon.org>

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

#include "alext.h"
#include "alc.h"
#include "alsoft.h"
#include "efx.h"

#include "audio/AudioContext.h"
#include "audio/AudioBuffer.h"
#include "logging.h"
#include "global.h"

namespace libtas {

// DEFINE_ORIG_POINTER(alBufferSubDataSOFT)
// DEFINE_ORIG_POINTER(alBufferDataStatic)

static ALCdevice dummyDevice = 0;
static ALCcontext dummyContext = -1;

ALCboolean myalcSetThreadContext(ALCcontext *context)
{
    LOGTRACE(LCF_SOUND | LCF_TODO);
    return ALC_TRUE;
}

ALCcontext* myalcGetThreadContext(void)
{
    LOGTRACE(LCF_SOUND | LCF_TODO);
    return &dummyContext;
}

ALCdevice* myalcLoopbackOpenDeviceSOFT(const ALCchar *deviceName)
{
    LOGTRACE(LCF_SOUND | LCF_TODO);
    Global::game_info.audio |= GameInfo::OPENAL;
    Global::game_info.tosend = true;
    return &dummyDevice;
}

ALCboolean myalcIsRenderFormatSupportedSOFT(ALCdevice *device, ALCsizei freq, ALCenum channels, ALCenum type)
{
    LOG(LL_TRACE, LCF_SOUND | LCF_TODO, " call with freq %d, channels %d and type %d", __func__, freq, channels, type);
    return ALC_TRUE;
}

void myalcRenderSamplesSOFT(ALCdevice *device, ALCvoid *buffer, ALCsizei samples)
{
    LOGTRACE(LCF_SOUND | LCF_TODO);
    AudioContext& audiocontext = AudioContext::get();
    audiocontext.mixAllSources(samples*audiocontext.outAlignSize);
    memcpy(buffer, audiocontext.outSamples.data(), audiocontext.outBytes);
}

const ALCchar* myalcGetStringiSOFT(ALCdevice *device, ALCenum paramName, ALCsizei index)
{
    LOGTRACE(LCF_SOUND | LCF_TODO);
    return "";
}

ALCboolean myalcResetDeviceSOFT(ALCdevice *device, const ALCint *attribs)
{
    LOGTRACE(LCF_SOUND | LCF_TODO);
    return ALC_TRUE;
}

void myalcDevicePauseSOFT(ALCdevice *device)
{
    LOGTRACE(LCF_SOUND);
    AudioContext::get().paused = true;
}

void myalcDeviceResumeSOFT(ALCdevice *device)
{
    LOGTRACE(LCF_SOUND | LCF_TODO);
    AudioContext::get().paused = false;
}

void myalBufferSubDataSOFT(ALuint buffer, ALenum format, const ALvoid *data, ALsizei offset, ALsizei length)
{
    LOG(LL_TRACE, LCF_SOUND, "%s call - copy buffer sub data of format %d, length %d and offset %d into buffer %d", __func__, format, length, offset, buffer);

    AudioContext& audiocontext = AudioContext::get();
    std::lock_guard<std::mutex> lock(audiocontext.mutex);

    auto ab = audiocontext.getBuffer(buffer);
    if (ab == nullptr) {
        alSetError(AL_INVALID_VALUE);
        return;
    }

    bool match;
    switch(format) {
        case AL_FORMAT_MONO8:
            match = ab->format == AudioBuffer::SAMPLE_FMT_U8;
            match &= ab->nbChannels == 1;
            break;
        case AL_FORMAT_MONO16:
            match = ab->format == AudioBuffer::SAMPLE_FMT_S16;
            match &= ab->nbChannels == 1;
            break;
        case AL_FORMAT_STEREO8:
            match = ab->format == AudioBuffer::SAMPLE_FMT_U8;
            match &= ab->nbChannels == 2;
            break;
        case AL_FORMAT_STEREO16:
            match = ab->format == AudioBuffer::SAMPLE_FMT_S16;
            match &= ab->nbChannels == 2;
            break;
        case AL_FORMAT_MONO_FLOAT32:
            match = ab->format == AudioBuffer::SAMPLE_FMT_FLT;
            match &= ab->nbChannels == 1;
            break;
        case AL_FORMAT_STEREO_FLOAT32:
            match = ab->format == AudioBuffer::SAMPLE_FMT_FLT;
            match &= ab->nbChannels == 2;
            break;
        case AL_FORMAT_MONO_DOUBLE_EXT:
            match = ab->format == AudioBuffer::SAMPLE_FMT_DBL;
            match &= ab->nbChannels == 1;
            break;
        case AL_FORMAT_STEREO_DOUBLE_EXT:
            match = ab->format == AudioBuffer::SAMPLE_FMT_DBL;
            match &= ab->nbChannels == 2;
            break;
        case AL_FORMAT_MONO_MSADPCM_SOFT:
            match = ab->format == AudioBuffer::SAMPLE_FMT_MSADPCM;
            match &= ab->nbChannels == 1;
            break;
        case AL_FORMAT_STEREO_MSADPCM_SOFT:
            match = ab->format == AudioBuffer::SAMPLE_FMT_MSADPCM;
            match &= ab->nbChannels == 2;
            break;
        default:
            LOG(LL_ERROR, LCF_SOUND, "Unsupported format: %d", format);
            return;
    }

    if (!match) {
        alSetError(AL_INVALID_ENUM);
        return;
    }

    if (ab->format == AudioBuffer::SAMPLE_FMT_MSADPCM) {
        if ((length % ab->blockSamples) != 0 || (offset % ab->blockSamples) != 0) {
            alSetError(AL_INVALID_VALUE);
            return;
        }
    }

    uint8_t* samples = nullptr;
    auto samplesAvail = ab->getSamples(samples, length / ab->alignSize, offset / ab->alignSize, ab->loop_point_end != 0); // not sure how to access looping here
    if (samplesAvail * ab->alignSize != length) {
        alSetError(AL_INVALID_VALUE);
        return;
    }

    LOG(LL_DEBUG, LCF_SOUND, "%s - do copy of length %d bytes", __func__, length);

    memcpy(samples, data, length);
}

void myalBufferDataStatic(ALint bid, ALenum format, const ALvoid* data, ALsizei size, ALsizei freq)
{
    LOG(LL_TRACE, LCF_SOUND, "%s call - copy buffer data of format %d, size %d and frequency %d into buffer %d", __func__, format, size, freq, bid);
    /* This function is OSX/iOS only, we do not need to implement it */
}

}
