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
#include "../../checkpoint/ThreadManager.h" // getThreadId()
#include "../../logging.h"
#include "../../hook.h"
#include "../AudioContext.h"
#include "../AudioSource.h"
#include "../AudioBuffer.h"
#include <cstring> // strncpy
#include <mutex>

namespace libtas {

DECLARE_ORIG_POINTER(SDL_MixAudioFormat)
DECLARE_ORIG_POINTER(SDL_MixAudio)

static const char* dummySDLDriver = "libtas";

/* Keep a copy of the SDL_AudioFormat used to open the audio device, because
 * SDL_MixAudio uses the current audio format, but as we actually don't open the
 * audio device, we call instead SDL_MixAudioFormat with the saved audio format.
 */
static SDL_AudioFormat audioFormat;
// static Uint16 bufferSamplesSize;

/* All SDL devices. From SDL implementation, there is a limit of 16 devices */
#define MAX_SDL_SOURCES 16
static std::shared_ptr<AudioSource> sourcesSDL[MAX_SDL_SOURCES];

static const char* dummySDLDevice = "libTAS device";
static std::string curDriver;
static std::recursive_mutex mutex;

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
        debuglogstdio(LCF_SDL | LCF_SOUND, "Init SDL Audio with default driver");
        curDriver = dummySDLDevice;
    }
    else {
        debuglogstdio(LCF_SDL | LCF_SOUND, "Init SDL Audio with driver %s", driver_name);
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

/* Helper function for SDL_OpenAudio() and SDL_OpenAudioDevice() */
static int open_audio_device(const SDL_AudioSpec * desired, SDL_AudioSpec * obtained, int min_id)
{
    if (shared_config.audio_disabled)
        return -1;

    /* Look for the next available device. Id 1 is reserved for SDL_OpenAudio() device */
    int id = min_id;
    for (; id < MAX_SDL_SOURCES; id++)
        if (!sourcesSDL[id-1])
            break;

    if (sourcesSDL[id-1]) {
        debuglogstdio(LCF_SDL | LCF_SOUND, "   no available device");
        return -1;
    }

    if (obtained != NULL) {
        memmove(obtained, desired, sizeof(SDL_AudioSpec));
    }

    std::lock_guard<std::mutex> lock(audiocontext.mutex);

    int bufferId = audiocontext.createBuffer();
    auto buffer = audiocontext.getBuffer(bufferId);

    /* Sanity check done by SDL */
    if (!obtained->freq) obtained->freq = 22050;
    buffer->frequency = obtained->freq;
    debuglogstdio(LCF_SDL | LCF_SOUND, "Frequency %d Hz", buffer->frequency);

    /* Sanity check done by SDL */
    if (!obtained->format) obtained->format = AUDIO_S16LSB;

    switch(obtained->format) {
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
            debuglogstdio(LCF_SDL | LCF_SOUND, "Unsupported audio format");
            return -1;
    }
    /* Sanity check done by SDL */
    if (!obtained->channels) obtained->channels = 2;
    buffer->nbChannels = obtained->channels;
    debuglogstdio(LCF_SDL | LCF_SOUND, "Channels %d", buffer->nbChannels);

    buffer->update();
    debuglogstdio(LCF_SDL | LCF_SOUND, "Format %d bits", buffer->bitDepth);

    buffer->size = obtained->samples * buffer->alignSize;
    buffer->update(); // Yes, a second time, to fill sampleSize based on size.
    buffer->samples.resize(buffer->size);

    /* Push buffers in a source */
    int sourceId = audiocontext.createSource();
    sourcesSDL[id-1] = audiocontext.getSource(sourceId);

    sourcesSDL[id-1]->buffer_queue.push_back(buffer);

    if (obtained->callback != NULL) {
        /* We are using the callback mechanism.
         * When the buffer is depleted, we are supposed to call the
         * callback function so that it fills the buffer again.
         * We will use a single AudioSource and a single AudioBuffer for this.
         * When the source plays the buffer until the end,
         * it calls the callback function to fill the buffer again.
         */

        sourcesSDL[id-1]->source = AudioSource::SOURCE_CALLBACK;
        SDL_AudioCallback audioCallback = obtained->callback;
        void* callbackArg = obtained->userdata;
        sourcesSDL[id-1]->callback = [audioCallback, callbackArg](AudioBuffer& ab){
            /* Emptying the audio buffer */
            ab.makeSilent();

            mutex.lock();
            audioCallback(callbackArg, ab.samples.data(), ab.size);
            mutex.unlock();
        };
        
        /* Specify the thread that opens the audio, to optimize Lock/UnlockAudio
         * and match its implementation */
        audiocontext.audio_thread = ThreadManager::getThreadId();
    }
    else {
        /* We are using the push mechanism. The game will use
         * SDL_QueueAudio() to push audio buffers.
         */
        sourcesSDL[id-1]->source = AudioSource::SOURCE_STREAMING_CONTINUOUS;
    }
    /* We simulate an empty buffer by setting the position at the end */
    sourcesSDL[id-1]->position = buffer->sampleSize;

    /* We must fill some information in the structs */
    obtained->size = buffer->size;

    /* Filling silence value. Not sure what to put here */
    obtained->silence = (buffer->format==AudioBuffer::SAMPLE_FMT_U8)?0x80:0x00;

    /* Keep a copy of the audio format */
    audioFormat = obtained->format;

    return id;
}

/* Override */ int SDL_OpenAudio(SDL_AudioSpec * desired, SDL_AudioSpec * obtained)
{
    DEBUGLOGCALL(LCF_SDL | LCF_SOUND);

    /* SDL1 has only one device which is id 1 */    
    if (sourcesSDL[0]) {
        debuglogstdio(LCF_SDL | LCF_SOUND, "   audio device already opened");
        return -1;
    }

    SDL_AudioDeviceID id = 0;
    
    if (obtained) {
        id = open_audio_device(desired, obtained, 1);
    }
    else {
        SDL_AudioSpec _obtained;
        memset(&_obtained, 0, sizeof(SDL_AudioSpec));
        id = open_audio_device(desired, &_obtained, 1);
        /* On successful open, copy calculated values into 'desired'. */
        if (id > 0) {
            desired->size = _obtained.size;
            desired->silence = _obtained.silence;
        }
    }
    
    return id==1?0:-1;
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
    debuglogstdio(LCF_SDL | LCF_SOUND, "%s called for device %s", __func__, device?device:"NULL");
    if (iscapture != 0)
        return 0;

    SDL_AudioDeviceID id = open_audio_device(desired, obtained, 2);
    return (id > 0)? id : 0;
}

/* Override */ SDL_AudioStatus SDL_GetAudioStatus(void)
{
    DEBUGLOGCALL(LCF_SDL | LCF_SOUND);
    return SDL_GetAudioDeviceStatus(1);
}

/* Override */ SDL_AudioStatus SDL_GetAudioDeviceStatus(SDL_AudioDeviceID dev)
{
    DEBUGLOGCALL(LCF_SDL | LCF_SOUND);

    if ((dev < 1) || (dev > MAX_SDL_SOURCES) || (!sourcesSDL[dev-1]))
        return SDL_AUDIO_STOPPED;

    switch(sourcesSDL[dev-1]->state) {
        case AudioSource::SOURCE_INITIAL:
        case AudioSource::SOURCE_STOPPED:
            return SDL_AUDIO_STOPPED;
        case AudioSource::SOURCE_PLAYING:
            return SDL_AUDIO_PLAYING;
        case AudioSource::SOURCE_PAUSED:
            return SDL_AUDIO_PAUSED;
        default:
            debuglogstdio(LCF_SDL | LCF_SOUND | LCF_ERROR, "Unknown source state");
            return SDL_AUDIO_STOPPED;
    }
}

/* Override */ void SDL_PauseAudio(int pause_on)
{
    DEBUGLOGCALL(LCF_SDL | LCF_SOUND);
    SDL_PauseAudioDevice(1, pause_on);
}

/* Override */ void SDL_PauseAudioDevice(SDL_AudioDeviceID dev, int pause_on)
{
    DEBUGLOGCALL(LCF_SDL | LCF_SOUND);
    if ((dev < 1) || (dev > MAX_SDL_SOURCES) || (!sourcesSDL[dev-1]))
        return;

    std::lock_guard<std::mutex> lock(audiocontext.mutex);
    if (pause_on == 0)
        sourcesSDL[dev-1]->state = AudioSource::SOURCE_PLAYING;
    else
        sourcesSDL[dev-1]->state = AudioSource::SOURCE_PAUSED;
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
    debuglogstdio(LCF_SDL | LCF_SOUND, "%s call with %d bytes of data", __func__, len);

    if ((dev < 1) || (dev > MAX_SDL_SOURCES) || (!sourcesSDL[dev-1]))
        return -1;

    if (sourcesSDL[dev-1]->source == AudioSource::SOURCE_CALLBACK) {
        /* We cannot queue samples when using the callback mechanism */
        return -1;
    }

    std::lock_guard<std::mutex> lock(audiocontext.mutex);

    /* We try to reuse a buffer that has been processed from the source */
    std::shared_ptr<AudioBuffer> ab;
    if (sourcesSDL[dev-1]->nbQueueProcessed() > 0) {
        /* Removing first buffer */
        ab = sourcesSDL[dev-1]->buffer_queue[0];
        sourcesSDL[dev-1]->buffer_queue.erase(sourcesSDL[dev-1]->buffer_queue.begin());
        sourcesSDL[dev-1]->queue_index--;
    }
    else {
        /* Building a new buffer */
        int bufferId = audiocontext.createBuffer();
        ab = audiocontext.getBuffer(bufferId);

        /* Getting the parameters of the buffer from one of the queue */
        if (sourcesSDL[dev-1]->buffer_queue.empty()) {
            debuglogstdio(LCF_SDL | LCF_SOUND | LCF_ERROR, "Empty queue, cannot guess buffer parameters");
            return -1;
        }

        auto ref = sourcesSDL[dev-1]->buffer_queue[0];
        ab->format = ref->format;
        ab->nbChannels = ref->nbChannels;
        ab->frequency = ref->frequency;
    }

    /* Filling buffer */
    ab->samples.clear();
    ab->samples.insert(ab->samples.end(), static_cast<const uint8_t*>(data), &(static_cast<const uint8_t*>(data))[len]);
    ab->size = len;
    ab->update();
    sourcesSDL[dev-1]->buffer_queue.push_back(ab);

    return 0;
}

/* Override */ Uint32 SDL_GetQueuedAudioSize(SDL_AudioDeviceID dev)
{
    DEBUGLOGCALL(LCF_SDL | LCF_SOUND);

    if ((dev < 1) || (dev > MAX_SDL_SOURCES) || (!sourcesSDL[dev-1]))
        return -1;

    if (sourcesSDL[dev-1]->source == AudioSource::SOURCE_CALLBACK) {
        /* We cannot get queue samples when using the callback mechanism */
        return 0;
    }

    std::lock_guard<std::mutex> lock(audiocontext.mutex);
    Uint32 qsize = sourcesSDL[dev-1]->queueSize() - sourcesSDL[dev-1]->getPosition();
    debuglogstdio(LCF_SDL | LCF_SOUND, "Returning %d", qsize);
    return qsize;
}

/* Override */ void SDL_ClearQueuedAudio(SDL_AudioDeviceID dev)
{
    DEBUGLOGCALL(LCF_SDL | LCF_SOUND);

    if ((dev < 1) || (dev > MAX_SDL_SOURCES) || (!sourcesSDL[dev-1]))
        return;

    if (sourcesSDL[dev-1]->source == AudioSource::SOURCE_CALLBACK) {
        /* We cannot get queue samples when using the callback mechanism */
        return;
    }

    std::lock_guard<std::mutex> lock(audiocontext.mutex);
    /* We simulate clearing the queue by setting the position to the end
     * of the queue.
     */
    sourcesSDL[dev-1]->setPosition(sourcesSDL[dev-1]->queueSize());
}

/* Override */ void SDL_LockAudio(void)
{
    DEBUGLOGCALL(LCF_SDL | LCF_SOUND);
    return SDL_LockAudioDevice(1);
}

/* Override */ void SDL_LockAudioDevice(SDL_AudioDeviceID dev)
{
    DEBUGLOGCALL(LCF_SDL | LCF_SOUND);
    if (ThreadManager::getThreadId() == audiocontext.audio_thread)
        return;
    mutex.lock();
}

/* Override */ void SDL_UnlockAudio(void)
{
    DEBUGLOGCALL(LCF_SDL | LCF_SOUND);
    return SDL_UnlockAudioDevice(1);
}

/* Override */ void SDL_UnlockAudioDevice(SDL_AudioDeviceID dev)
{
    DEBUGLOGCALL(LCF_SDL | LCF_SOUND);
    if (ThreadManager::getThreadId() == audiocontext.audio_thread)
        return;
    mutex.unlock();
}

/* Override */ void SDL_CloseAudio(void)
{
    DEBUGLOGCALL(LCF_SDL | LCF_SOUND);
    return SDL_CloseAudioDevice(1);
}

/* Override */ void SDL_CloseAudioDevice(SDL_AudioDeviceID dev)
{
    debuglogstdio(LCF_SDL | LCF_SOUND, "%s called with dev %d", __func__, dev);

    if (dev < 1 || dev > MAX_SDL_SOURCES)
        return;

    std::lock_guard<std::mutex> lock(audiocontext.mutex);
    /* Remove the source from the audio context */
    if (sourcesSDL[dev-1]) {
        audiocontext.deleteSource(sourcesSDL[dev-1]->id);
    }

    /* Destroy the source object */
    sourcesSDL[dev-1].reset();
}

}
