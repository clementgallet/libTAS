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

#include "sdlaudio.h"
#include "../../logging.h"
#include "../../hook.h"
#include "../AudioContext.h"
#include "../AudioSource.h"
#include "../AudioBuffer.h"
#include <cstring> // strncpy

namespace libtas {

DECLARE_ORIG_POINTER(SDL_MixAudioFormat);
DECLARE_ORIG_POINTER(SDL_MixAudio);

static const char* dummySDLDriver = "libtas";

/* Keep a copy of the SDL_AudioFormat used to open the audio device, because
 * SDL_MixAudio uses the current audio format, but as we actually don't open the
 * audio device, we call instead SDL_MixAudioFormat with the saved audio format.
 */
static SDL_AudioFormat audioFormat;
static SDL_AudioCallback audioCallback;
static void* callbackArg;
// static Uint16 bufferSamplesSize;

static std::shared_ptr<AudioSource> sourceSDL;

static const char* dummySDLDevice = "libTAS device";
static std::string curDriver;

/* Override */ int SDL_GetNumAudioDrivers(void)
{
    DEBUGLOGCALL(LCF_SDL | LCF_SOUND);
    return 1;
}

/* Override */ const char *SDL_GetAudioDriver(int index)
{
    DEBUGLOGCALL(LCF_SDL | LCF_SOUND);
    if (index == 0)
        return dummySDLDriver;
    return NULL;
}

char * SDL_AudioDriverName(char *namebuf, int maxlen)
{
    DEBUGLOGCALL(LCF_SDL | LCF_SOUND);
    strncpy(namebuf, dummySDLDriver, maxlen);
    /* I don't know if we are in charge of returning a
     * null-terminated string
     */
    if (maxlen > 0)
        namebuf[maxlen-1] = '\0';

    return namebuf;
}

/* Override */ int SDL_AudioInit(const char *driver_name)
{
    if (driver_name == NULL) {
        debuglog(LCF_SDL | LCF_SOUND, "Init SDL Audio with default driver");
        curDriver = dummySDLDevice;
    }
    else {
        debuglog(LCF_SDL | LCF_SOUND, "Init SDL Audio with driver ", driver_name);
        curDriver = driver_name;
    }
    return 0;
}

/* Override */ void SDL_AudioQuit(void)
{
    DEBUGLOGCALL(LCF_SDL | LCF_SOUND);
}

/* Override */ const char *SDL_GetCurrentAudioDriver(void)
{
    DEBUGLOGCALL(LCF_SDL | LCF_SOUND);
    if (! curDriver.empty()) {
        return curDriver.c_str();
    }
    return nullptr;
}

/* Function that is called by an AudioSource when the played sound buffer
 * is empty
 */
void fillBufferCallback(AudioBuffer& ab);
void fillBufferCallback(AudioBuffer& ab)
{
    /* Emptying the audio buffer */
    ab.makeSilent();
    audioCallback(callbackArg, ab.samples.data(), ab.size);
}

/* Override */ int SDL_OpenAudio(SDL_AudioSpec * desired, SDL_AudioSpec * obtained)
{
    DEBUGLOGCALL(LCF_SDL | LCF_SOUND);

    std::lock_guard<std::mutex> lock(audiocontext.mutex);

    int bufferId = audiocontext.createBuffer();
    auto buffer = audiocontext.getBuffer(bufferId);

    /* Sanity check done by SDL */
    if (!desired->freq) desired->freq = 22050;
    buffer->frequency = desired->freq;
    debuglog(LCF_SDL | LCF_SOUND, "Frequency ",buffer->frequency, " Hz");

    /* Sanity check done by SDL */
    if (!desired->format) desired->format = AUDIO_S16LSB;

    switch(desired->format) {
        case AUDIO_U8:
            buffer->format = AudioBuffer::SAMPLE_FMT_U8;
            break;
        case AUDIO_S16LSB:
            buffer->format = AudioBuffer::SAMPLE_FMT_S16;
            break;
        case AUDIO_S32LSB:
            buffer->format = AudioBuffer::SAMPLE_FMT_S32;
            break;
        case AUDIO_F32LSB:
            buffer->format = AudioBuffer::SAMPLE_FMT_FLT;
            break;
        default:
            debuglog(LCF_SDL | LCF_SOUND, "Unsupported audio format");
            return -1;
    }
    /* Sanity check done by SDL */
    if (!desired->channels) desired->channels = 2;
    buffer->nbChannels = desired->channels;
    debuglog(LCF_SDL | LCF_SOUND, "Channels ",buffer->nbChannels);

    buffer->update();
    debuglog(LCF_SDL | LCF_SOUND, "Format ",buffer->bitDepth," bits");

    buffer->size = desired->samples * buffer->alignSize;
    buffer->update(); // Yes, a second time, to fill sampleSize based on size.
    buffer->samples.resize(buffer->size);

    /* Push buffers in a source */
    int sourceId = audiocontext.createSource();
    sourceSDL = audiocontext.getSource(sourceId);

    sourceSDL->buffer_queue.push_back(buffer);

    if (desired->callback != NULL) {
        /* We are using the callback mechanism.
         * When the buffer is depleted, we are supposed to call the
         * callback function so that it fills the buffer again.
         * We will use a single AudioSource and a single AudioBuffer for this.
         * When the source plays the buffer until the end,
         * it calls the callback function to fill the buffer again.
         */

        sourceSDL->source = AudioSource::SOURCE_CALLBACK;
        sourceSDL->callback = fillBufferCallback;
        audioCallback = desired->callback;
        callbackArg = desired->userdata;
    }
    else {
        /* We are using the push mechanism. The game will use
         * SDL_QueueAudio() to push audio buffers.
         */
        sourceSDL->source = AudioSource::SOURCE_STREAMING_CONTINUOUS;
    }
    /* We simulate an empty buffer by setting the position at the end */
    sourceSDL->position = buffer->sampleSize;

    /* We must fill some information in the structs */
    desired->size = buffer->size;

    /* Filling silence value. Not sure what to put here */
    desired->silence = (buffer->format==AudioBuffer::SAMPLE_FMT_U8)?0x80:0x00;

    if (obtained != NULL) {
        memmove(obtained, desired, sizeof(SDL_AudioSpec));
    }

    /* Keep a copy of the audio format */
    audioFormat = desired->format;

    return 0;
}

/* Override */ int SDL_GetNumAudioDevices(int iscapture)
{
    DEBUGLOGCALL(LCF_SDL | LCF_SOUND);
    if (iscapture != 0)
        return 0;
    return 1;
}

/* Override */ const char *SDL_GetAudioDeviceName(int index, int iscapture)
{
    DEBUGLOGCALL(LCF_SDL | LCF_SOUND);
    if (iscapture != 0)
        return NULL;
    if (index == 0)
        return dummySDLDevice;
    return NULL;
}

/* Override */ SDL_AudioDeviceID SDL_OpenAudioDevice(const char *device,
                   int iscapture, const SDL_AudioSpec *desired,
                   SDL_AudioSpec *obtained, int allowed_changes)
{
    DEBUGLOGCALL(LCF_SDL | LCF_SOUND);
    if (iscapture != 0)
        return 0;

    SDL_OpenAudio(const_cast<SDL_AudioSpec*>(desired), obtained); // TODO: cast probably not good
    return 2;
}

/* Override */ SDL_AudioStatus SDL_GetAudioStatus(void)
{
    DEBUGLOGCALL(LCF_SDL | LCF_SOUND);
    switch(sourceSDL->state) {
        case AudioSource::SOURCE_INITIAL:
        case AudioSource::SOURCE_STOPPED:
            return SDL_AUDIO_STOPPED;
        case AudioSource::SOURCE_PLAYING:
            return SDL_AUDIO_PLAYING;
        case AudioSource::SOURCE_PAUSED:
            return SDL_AUDIO_PAUSED;
        default:
            debuglog(LCF_SDL | LCF_SOUND | LCF_ERROR, "Unknown source state");
            return SDL_AUDIO_STOPPED;
    }
}

/* Override */ SDL_AudioStatus SDL_GetAudioDeviceStatus(SDL_AudioDeviceID dev)
{
    DEBUGLOGCALL(LCF_SDL | LCF_SOUND);
    return SDL_GetAudioStatus();
}

/* Override */ void SDL_PauseAudio(int pause_on)
{
    DEBUGLOGCALL(LCF_SDL | LCF_SOUND);
    std::lock_guard<std::mutex> lock(audiocontext.mutex);
    if (!sourceSDL)
        return;
    if (pause_on == 0)
        sourceSDL->state = AudioSource::SOURCE_PLAYING;
    else
        sourceSDL->state = AudioSource::SOURCE_PAUSED;

}

/* Override */ void SDL_PauseAudioDevice(SDL_AudioDeviceID dev, int pause_on)
{
    DEBUGLOGCALL(LCF_SDL | LCF_SOUND);
    SDL_PauseAudio(pause_on);
}

void SDL_MixAudio(Uint8 * dst, const Uint8 * src, Uint32 len, int volume)
{
    DEBUGLOGCALL(LCF_SDL | LCF_SOUND);

    int SDLver = get_sdlversion();

    if (SDLver == 2) {
        /* SDL_MixAudio uses the opened audio device, but we don't open any,
         * so we use SDL_MixAudioFormat which don't required a device to be opened
         * and instead let us specify the audio format.
         */

         LINK_NAMESPACE_SDL2(SDL_MixAudioFormat);
         return orig::SDL_MixAudioFormat(dst, src, audioFormat, len, volume);
    }

    LINK_NAMESPACE_SDL1(SDL_MixAudio);
    return orig::SDL_MixAudio(dst, src, len, volume);
}


/* Override */ int SDL_QueueAudio(SDL_AudioDeviceID dev, const void *data, Uint32 len)
{
    debuglog(LCF_SDL | LCF_SOUND, __func__, " call with ", len, " bytes of data");

    if (sourceSDL->source == AudioSource::SOURCE_CALLBACK) {
        /* We cannot queue samples when using the callback mechanism */
        return -1;
    }

    std::lock_guard<std::mutex> lock(audiocontext.mutex);

    /* We try to reuse a buffer that has been processed from the source */
    std::shared_ptr<AudioBuffer> ab;
    if (sourceSDL->nbQueueProcessed() > 0) {
        /* Removing first buffer */
        ab = sourceSDL->buffer_queue[0];
        sourceSDL->buffer_queue.erase(sourceSDL->buffer_queue.begin());
        sourceSDL->queue_index--;
    }
    else {
        /* Building a new buffer */
        int bufferId = audiocontext.createBuffer();
        ab = audiocontext.getBuffer(bufferId);

        /* Getting the parameters of the buffer from one of the queue */
        if (sourceSDL->buffer_queue.empty()) {
            debuglog(LCF_SDL | LCF_SOUND | LCF_ERROR, "Empty queue, cannot guess buffer parameters");
            return -1;
        }

        auto ref = sourceSDL->buffer_queue[0];
        ab->format = ref->format;
        ab->nbChannels = ref->nbChannels;
        ab->frequency = ref->frequency;
    }

    /* Filling buffer */
    ab->samples.clear();
    ab->samples.insert(ab->samples.end(), static_cast<const uint8_t*>(data), &(static_cast<const uint8_t*>(data))[len]);
    ab->size = len;
    ab->update();
    sourceSDL->buffer_queue.push_back(ab);

    return 0;
}

/* Override */ Uint32 SDL_GetQueuedAudioSize(SDL_AudioDeviceID dev)
{
    DEBUGLOGCALL(LCF_SDL | LCF_SOUND);

    if (sourceSDL->source == AudioSource::SOURCE_CALLBACK) {
        /* We cannot get queue samples when using the callback mechanism */
        return 0;
    }

    std::lock_guard<std::mutex> lock(audiocontext.mutex);
    Uint32 qsize = sourceSDL->queueSize() - sourceSDL->getPosition();
    debuglog(LCF_SDL | LCF_SOUND, "Returning ", qsize);
    return qsize;
}

/* Override */ void SDL_ClearQueuedAudio(SDL_AudioDeviceID dev)
{
    DEBUGLOGCALL(LCF_SDL | LCF_SOUND);

    if (sourceSDL->source == AudioSource::SOURCE_CALLBACK) {
        /* We cannot get queue samples when using the callback mechanism */
        return;
    }

    std::lock_guard<std::mutex> lock(audiocontext.mutex);
    /* We simulate clearing the queue by setting the position to the end
     * of the queue.
     */
    sourceSDL->setPosition(sourceSDL->queueSize());
}

/* Override */ void SDL_LockAudio(void)
{
    DEBUGLOGCALL(LCF_SDL | LCF_SOUND);
}

/* Override */ void SDL_LockAudioDevice(SDL_AudioDeviceID dev)
{
    DEBUGLOGCALL(LCF_SDL | LCF_SOUND);
}

/* Override */ void SDL_UnlockAudio(void)
{
    DEBUGLOGCALL(LCF_SDL | LCF_SOUND);
}

/* Override */ void SDL_UnlockAudioDevice(SDL_AudioDeviceID dev)
{
    DEBUGLOGCALL(LCF_SDL | LCF_SOUND);
}

/* Override */ void SDL_CloseAudio(void)
{
    DEBUGLOGCALL(LCF_SDL | LCF_SOUND);

    std::lock_guard<std::mutex> lock(audiocontext.mutex);
    /* Remove the source from the audio context */
    if (sourceSDL) {
        audiocontext.deleteSource(sourceSDL->id);
    }

    /* Destroy the source object */
    sourceSDL.reset();
}

/* Override */ void SDL_CloseAudioDevice(SDL_AudioDeviceID dev)
{
    DEBUGLOGCALL(LCF_SDL | LCF_SOUND);

    std::lock_guard<std::mutex> lock(audiocontext.mutex);
    /* Remove the source from the audio context */
    if (sourceSDL) {
        audiocontext.deleteSource(sourceSDL->id);
    }

    /* Destroy the source object */
    sourceSDL.reset();
}

}
