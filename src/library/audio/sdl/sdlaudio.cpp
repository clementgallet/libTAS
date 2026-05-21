/*
    Copyright 2015-2026 Clément Gallet <clement.gallet@ens-lyon.org>

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

#include "sdl/sdldynapi.h"
#include "sdl/sdlversion.h"
#include "checkpoint/ThreadManager.h" // getThreadId()
#include "logging.h"
#include "hook.h"
#include "global.h"
#include "audio/AudioContext.h"
#include "audio/AudioSource.h"
#include "audio/AudioBuffer.h"

#include <cstring> // strncpy
#include <mutex>
#include <vector>
#include <algorithm>

namespace libtas {

static const char* dummy_sdl_driver_name = "libtas";

struct SdlDevice
{
    int id;
    SDL_AudioFormat format;
    int channels;
    int freq;
};

/* Keep a copy of the SDL_AudioFormat used to open the audio device, because
 * SDL_MixAudio uses the current audio format, but as we actually don't open the
 * audio device, we call instead SDL_MixAudioFormat with the saved audio format.
 */
static SDL_AudioFormat audioFormat;
// static Uint16 bufferSamplesSize;

/* All SDL devices. From SDL implementation, there is a limit of 16 devices */
#define MAX_SDL_DEVICES 16
static SdlDevice sdl_devices[MAX_SDL_DEVICES];
static std::vector<std::shared_ptr<AudioSource>> sdl_device_sources[MAX_SDL_DEVICES];

static const char* dummy_sdl_device_name = "libTAS device";
static std::string curDriver;
static std::recursive_mutex mutex;

/* Override */ int SDL_GetNumAudioDrivers(void)
{
    LOGTRACE(LCF_SDL | LCF_SOUND);
    return 1;
}

/* Override */ const char *SDL_GetAudioDriver(int index)
{
    LOGTRACE(LCF_SDL | LCF_SOUND);
    if (index == 0)
        return dummy_sdl_driver_name;
    return NULL;
}

/* Override */ sdl3::SDL_AudioDeviceID * SDL_GetAudioPlaybackDevices(int *count)
{
    LOGTRACE(LCF_SDL | LCF_SOUND);
    if (count)
        *count = 1;

    sdl3::SDL_AudioDeviceID *ids = static_cast<sdl3::SDL_AudioDeviceID*>(ORIG_SDL3_CALL(SDL_malloc, (2 * sizeof(sdl3::SDL_AudioDeviceID))));
    ids[0] = 1;
    ids[1] = 0; // null-terminated list of devices, as per SDL documentation
    return ids;
}

char * SDL_AudioDriverName(char *namebuf, int maxlen)
{
    LOGTRACE(LCF_SDL | LCF_SOUND);
    strncpy(namebuf, dummy_sdl_driver_name, maxlen);
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
        LOG(LL_TRACE, LCF_SDL | LCF_SOUND, "Init SDL Audio with default driver");
        curDriver = dummy_sdl_device_name;
    }
    else {
        LOG(LL_TRACE, LCF_SDL | LCF_SOUND, "Init SDL Audio with driver %s", driver_name);
        curDriver = driver_name;
    }
    return 0;
}

/* Override */ void SDL_AudioQuit(void)
{
    LOGTRACE(LCF_SDL | LCF_SOUND);
}

/* Override */ const char *SDL_GetCurrentAudioDriver(void)
{
    LOGTRACE(LCF_SDL | LCF_SOUND);
    if (! curDriver.empty()) {
        return curDriver.c_str();
    }
    return nullptr;
}

/* Helper function to convert SDL audio format to internal format */
static AudioBuffer::SampleFormat sdlFormatToFormat(SDL_AudioFormat sdlFormat)
{
    switch(sdlFormat) {
        case SDL_AUDIO_U8:
            return AudioBuffer::SAMPLE_FMT_U8;
        case SDL_AUDIO_S16:
            return AudioBuffer::SAMPLE_FMT_S16;
        case SDL_AUDIO_S32:
            return AudioBuffer::SAMPLE_FMT_S32;
        case SDL_AUDIO_F32:
            return AudioBuffer::SAMPLE_FMT_FLT;
        default:
            LOG(LL_DEBUG, LCF_SDL | LCF_SOUND, "Unsupported audio format");
            return AudioBuffer::SAMPLE_FMT_UNKNOWN;
    }
}

static SDL_AudioFormat formatToSdlFormat(AudioBuffer::SampleFormat format)
{
    switch(format) {
        case AudioBuffer::SAMPLE_FMT_U8:
            return SDL_AUDIO_U8;
        case AudioBuffer::SAMPLE_FMT_S16:
            return SDL_AUDIO_S16;
        case AudioBuffer::SAMPLE_FMT_S32:
            return SDL_AUDIO_S32;
        case AudioBuffer::SAMPLE_FMT_FLT:
            return SDL_AUDIO_F32;
        default:
            return SDL_AUDIO_UNKNOWN;
    }
}

/* Helper function for SDL_OpenAudio() and SDL_OpenAudioDevice() */
static int open_audio_device(const sdl2::SDL_AudioSpec * desired, sdl2::SDL_AudioSpec * obtained, int min_id)
{
    sdl2::SDL_AudioSpec _obtained;

    if (Global::shared_config.audio_disabled)
        return -1;

    /* Look for the next available device. Id 1 is reserved for SDL_OpenAudio() device */
    int id = min_id;
    for (; id < MAX_SDL_DEVICES; id++)
        if (sdl_devices[id].id == 0)
            break;

    if (sdl_devices[id].id != 0) {
        LOG(LL_DEBUG, LCF_SDL | LCF_SOUND, "   no available device");
        return -1;
    }

    if (!obtained) {
        obtained = &_obtained;
    }

    memcpy(obtained, desired, sizeof(sdl2::SDL_AudioSpec));
    AudioContext& audiocontext = AudioContext::get();
    std::lock_guard<std::mutex> lock(audiocontext.mutex);

    /* Sanity checks done by SDL */
    if (desired->freq == 0) obtained->freq = 22050;

    LOG(LL_DEBUG, LCF_SDL | LCF_SOUND, "Frequency %d Hz", obtained->freq);

    if (desired->format == 0) obtained->format = SDL_AUDIO_S16;

    if (desired->samples == 0) {
		/* Pick a default of ~46 ms at desired frequency */
		int samples = (desired->freq / 1000) * 46;
		int power2 = 1;
		while (power2 < samples) {
			power2 *= 2;
		}
		obtained->samples = power2;
	}

    /* Sanity check done by SDL */
    if (desired->channels == 0) obtained->channels = 2;

    LOG(LL_DEBUG, LCF_SDL | LCF_SOUND, "Channels %d", obtained->channels);

    /* Store the specs into the sdl device*/
    sdl_devices[id].channels = obtained->channels;
    sdl_devices[id].format = obtained->format;
    sdl_devices[id].freq = obtained->freq;

    /* Create a source to hold the buffers */
    std::shared_ptr<AudioSource> sdl_source = audiocontext.createSource();

    sdl_source->channels = obtained->channels;
    sdl_source->format = sdlFormatToFormat(obtained->format);
    sdl_source->frequency = obtained->freq;

    sdl_device_sources[id].push_back(sdl_source);

    if (sdl_source->format == AudioBuffer::SAMPLE_FMT_UNKNOWN) {
        LOG(LL_DEBUG, LCF_SDL | LCF_SOUND, "Unsupported audio format");
        return -1;
    }

    if (obtained->callback != NULL) {
        /* We are using the callback mechanism.
         * When the buffer is depleted, we are supposed to call the
         * callback function so that it fills the buffer again.
         * We will use a single AudioSource and a single AudioBuffer for this.
         * When the source plays the buffer until the end,
         * it calls the callback function to fill the buffer again.
         */

        auto buffer = audiocontext.createBuffer();

        buffer->frequency = sdl_source->frequency;
        buffer->channels = sdl_source->channels;
        buffer->format = sdl_source->format;

        buffer->size = obtained->samples * buffer->alignSize;
        buffer->update(); // Yes, a second time, to fill sampleSize based on size.
        buffer->samples.resize(buffer->size);

        sdl_source->source = AudioSource::SOURCE_CALLBACK;
        sdl2::SDL_AudioCallback audioCallback = obtained->callback;
        void* callbackArg = obtained->userdata;
        sdl_source->callback = [audioCallback, callbackArg](AudioBuffer& ab){
            /* Emptying the audio buffer */
            ab.makeSilent();

            mutex.lock();
            audioCallback(callbackArg, ab.samples.data(), ab.size);
            mutex.unlock();
        };
        
        /* We simulate an empty buffer by setting the position at the end */
        sdl_source->position = buffer->sampleSize;

        sdl_source->queueBuffer(buffer);

        /* Specify the thread that opens the audio, to optimize Lock/UnlockAudio
         * and match its implementation */
        audiocontext.audio_thread = ThreadManager::getThreadId();
    }
    else {
        /* We are using the push mechanism. The game will use
         * SDL_QueueAudio() to push audio buffers.
         */
        sdl_source->source = AudioSource::SOURCE_STREAMING_CONTINUOUS;
    }

    /* We must fill some information in the structs */
    obtained->size = obtained->samples * obtained->channels * (obtained->format & 0xFF) / 8;

    /* Filling silence value. Not sure what to put here */
    obtained->silence = (obtained->format==SDL_AUDIO_U8)?0x80:0x00;

    /* Keep a copy of the audio format */
    audioFormat = obtained->format;

    return id;
}

/* Override */ int SDL_OpenAudio(sdl2::SDL_AudioSpec * desired, sdl2::SDL_AudioSpec * obtained)
{
    LOGTRACE(LCF_SDL | LCF_SOUND);

    /* SDL1 has only one device which is id 1 */    
    if (sdl_devices[1].id != 0) {
        LOG(LL_DEBUG, LCF_SDL | LCF_SOUND, "   audio device already opened");
        return -1;
    }

    sdl2::SDL_AudioDeviceID id = 0;
    
    if (obtained) {
        id = open_audio_device(desired, obtained, 1);
    }
    else {
        sdl2::SDL_AudioSpec _obtained;
        memset(&_obtained, 0, sizeof(sdl2::SDL_AudioSpec));
        id = open_audio_device(desired, &_obtained, 1);
        /* On successful open, copy calculated values into 'desired'. */
        if (id > 0) {
            desired->size = _obtained.size;
            desired->silence = _obtained.silence;
            desired->samples = _obtained.samples;
        }
    }
    
    return id==1?0:-1;
}

/* Override */ int SDL_GetNumAudioDevices(int iscapture)
{
    LOGTRACE(LCF_SDL | LCF_SOUND);
    if (iscapture != 0)
        return 0;
    return 1;
}

/* Override */ const char *sdl2::SDL_GetAudioDeviceName(int index, int iscapture)
{
    LOGTRACE(LCF_SDL | LCF_SOUND);
    if (iscapture != 0)
        return NULL;
    if (index == 0)
        return dummy_sdl_device_name;
    return NULL;
}

/* Override */ const char * sdl3::SDL_GetAudioDeviceName(SDL_AudioDeviceID devid)
{
    LOGTRACE(LCF_SDL | LCF_SOUND);
    return dummy_sdl_device_name;
}

/* Override */ int SDL_GetAudioDeviceSpec(int index, int iscapture, sdl2::SDL_AudioSpec *spec)
{
    LOGTRACE(LCF_SDL | LCF_SOUND);
    if (iscapture != 0)
        return -1;
    if (!spec)
        return -1;    
    if (index != 0)
        return -1;

    spec->freq = Global::shared_config.audio_frequency;
    switch (Global::shared_config.audio_bitdepth) {
        case 8:
            spec->format = SDL_AUDIO_U8;
            break;
        case 16:
            spec->format = SDL_AUDIO_S16;
            break;
    }
    spec->channels = Global::shared_config.audio_channels;

    return 0;
}

/* Override */ bool SDL_GetAudioDeviceFormat(sdl3::SDL_AudioDeviceID devid, sdl3::SDL_AudioSpec *spec, int *sample_frames)
{
    LOGTRACE(LCF_SDL | LCF_SOUND);
    if (devid >= MAX_SDL_DEVICES)
        return false;
    if (!spec)
        return false;

    if (sdl_devices[devid].id != 0) {
        spec->freq = sdl_devices[devid].freq;
        spec->format = sdl_devices[devid].format;
        spec->channels = sdl_devices[devid].channels;
    }
    else {
        /* Device not opened, fill with global values */
        spec->freq = Global::shared_config.audio_frequency;
        switch (Global::shared_config.audio_bitdepth) {
            case 8:
                spec->format = SDL_AUDIO_U8;
                break;
            case 16:
                spec->format = SDL_AUDIO_S16LE;
                break;
        }
        spec->channels = Global::shared_config.audio_channels;
    }

    if (sample_frames) {
        LOG(LL_DEBUG, LCF_SDL | LCF_SOUND, "Game wants to set the buffer size to %d frames", *sample_frames);
    }

    return true;
}

/* Override */ int SDL_GetDefaultAudioInfo(char **name, sdl2::SDL_AudioSpec *spec, int iscapture)
{
    LOGTRACE(LCF_SDL | LCF_SOUND);
    if (iscapture != 0)
        return -1;
    if (!spec)
        return -1;
    if (name)
        *name = nullptr;

    spec->freq = Global::shared_config.audio_frequency;
    switch (Global::shared_config.audio_bitdepth) {
        case 8:
            spec->format = SDL_AUDIO_U8;
            break;
        case 16:
            spec->format = SDL_AUDIO_S16;
            break;
    }
    spec->channels = Global::shared_config.audio_channels;

    return 0;
}

/* Override */ sdl2::SDL_AudioDeviceID SDL_OpenAudioDevice(const char *device,
                   int iscapture, const sdl2::SDL_AudioSpec *desired,
                   sdl2::SDL_AudioSpec *obtained, int allowed_changes)
{
    LOG(LL_DEBUG, LCF_SDL | LCF_SOUND, "%s called for device %s", __func__, device?device:"NULL");
    if (iscapture != 0)
        return 0;

    sdl2::SDL_AudioDeviceID id = open_audio_device(desired, obtained, 2);
    return (id > 0)? id : 0;
}

/* Override */ sdl3::SDL_AudioDeviceID sdl3::SDL_OpenAudioDevice(SDL_AudioDeviceID devid, const SDL_AudioSpec *spec)
{
    LOG(LL_DEBUG, LCF_SDL | LCF_SOUND, "%s called for device %d", __func__, devid);
    if (devid == 0)
        return 0;

    sdl2::SDL_AudioSpec specs_sdl2;
    memset(&specs_sdl2, 0, sizeof(sdl2::SDL_AudioSpec));

    specs_sdl2.freq = spec->freq;
    specs_sdl2.format = static_cast<SDL_AudioFormat>(spec->format);
    specs_sdl2.channels = spec->channels;

    sdl2::SDL_AudioDeviceID id = open_audio_device(&specs_sdl2, nullptr, devid);
    return (id > 0)? id : 0;
}

/* Override */ sdl2::SDL_AudioStatus SDL_GetAudioStatus(void)
{
    LOGTRACE(LCF_SDL | LCF_SOUND);
    return SDL_GetAudioDeviceStatus(1);
}

/* Override */ sdl2::SDL_AudioStatus SDL_GetAudioDeviceStatus(sdl2::SDL_AudioDeviceID dev)
{
    LOGTRACE(LCF_SDL | LCF_SOUND);

    if ((dev <= 1) || (dev > MAX_SDL_DEVICES) || (sdl_devices[dev].id == 0))
        return sdl2::SDL_AUDIO_STOPPED;

    if (sdl_device_sources[dev].empty()) {
        LOG(LL_WARN, LCF_SDL | LCF_SOUND, "No source associated to SDL device");
        return sdl2::SDL_AUDIO_STOPPED;
    }

    auto& source = sdl_device_sources[dev][0];
    switch(source->state) {
        case AudioSource::SOURCE_INITIAL:
        case AudioSource::SOURCE_STOPPED:
            return sdl2::SDL_AUDIO_STOPPED;
        case AudioSource::SOURCE_PLAYING:
            return sdl2::SDL_AUDIO_PLAYING;
        case AudioSource::SOURCE_PAUSED:
            return sdl2::SDL_AUDIO_PAUSED;
        default:
            LOG(LL_FATAL, LCF_SDL | LCF_SOUND, "Unknown source state");
            return sdl2::SDL_AUDIO_STOPPED;
    }
}

/* Override */ void SDL_PauseAudio(int pause_on)
{
    LOGTRACE(LCF_SDL | LCF_SOUND);
    sdl2::SDL_PauseAudioDevice(1, pause_on);
}

/* Override */ void sdl2::SDL_PauseAudioDevice(sdl2::SDL_AudioDeviceID dev, int pause_on)
{
    LOGTRACE(LCF_SDL | LCF_SOUND);
    if ((dev <= 1) || (dev > MAX_SDL_DEVICES) || (sdl_devices[dev].id == 0))
        return;

    std::lock_guard<std::mutex> lock(AudioContext::get().mutex);
    for (auto& sdl_source : sdl_device_sources[dev]) {
        sdl_source->state = pause_on ? AudioSource::SOURCE_PAUSED : AudioSource::SOURCE_PLAYING;
    }
}

bool sdl3::SDL_PauseAudioDevice(SDL_AudioDeviceID devid)
{
    LOGTRACE(LCF_SDL | LCF_SOUND);
    if ((devid <= 1) || (devid > MAX_SDL_DEVICES) || (sdl_devices[devid].id == 0))
        return false;

    std::lock_guard<std::mutex> lock(AudioContext::get().mutex);
    for (auto& sdl_source : sdl_device_sources[devid]) {
        sdl_source->state = AudioSource::SOURCE_PAUSED;
    }
    return true;
}

bool SDL_ResumeAudioDevice(sdl3::SDL_AudioDeviceID devid)
{
    LOGTRACE(LCF_SDL | LCF_SOUND);
    if ((devid <= 1) || (devid > MAX_SDL_DEVICES) || (sdl_devices[devid].id == 0))
        return false;

    std::lock_guard<std::mutex> lock(AudioContext::get().mutex);
    for (auto& sdl_source : sdl_device_sources[devid]) {
        sdl_source->state = AudioSource::SOURCE_PLAYING;
    }
    return true;
}

bool SDL_AudioDevicePaused(sdl3::SDL_AudioDeviceID devid)
{
    LOGTRACE(LCF_SDL | LCF_SOUND);
    if ((devid <= 1) || (devid > MAX_SDL_DEVICES) || (sdl_devices[devid].id == 0))
        return false;

    std::lock_guard<std::mutex> lock(AudioContext::get().mutex);
    if (sdl_device_sources[devid].empty()) {
        return false;
    }
    return sdl_device_sources[devid][0]->state == AudioSource::SOURCE_PAUSED;
}

void SDL_MixAudio(Uint8 * dst, const Uint8 * src, Uint32 len, int volume)
{
    LOGTRACE(LCF_SDL | LCF_SOUND);

    int SDLver = get_sdlversion();

    if (SDLver == 2) {
        /* SDL_MixAudio uses the opened audio device, but we don't open any,
         * so we use SDL_MixAudioFormat which don't required a device to be opened
         * and instead let us specify the audio format.
         */

        return ORIG_SDL2_CALL(SDL_MixAudioFormat, (dst, src, audioFormat, len, volume));
    }

    return ORIG_SDL2_CALL(SDL_MixAudio, (dst, src, len, volume));
}

/* Override */ float SDL_GetAudioDeviceGain(sdl3::SDL_AudioDeviceID devid)
{
    LOGTRACE(LCF_SDL | LCF_SOUND);
    if ((devid <= 1) || (devid > MAX_SDL_DEVICES) || (sdl_devices[devid].id == 0))
        return -1.0f;

    /* We don't support per-device gain. Returning the global audio context gain.*/
    std::lock_guard<std::mutex> lock(AudioContext::get().mutex);
    AudioContext& audiocontext = AudioContext::get();
    return audiocontext.outVolume;
}

/* Override */ bool SDL_SetAudioDeviceGain(sdl3::SDL_AudioDeviceID devid, float gain)
{
    LOGTRACE(LCF_SDL | LCF_SOUND);
    if ((devid <= 1) || (devid > MAX_SDL_DEVICES) || (sdl_devices[devid].id == 0))
        return false;

    /* We don't support per-device gain. Setting the global audio context gain.*/
    std::lock_guard<std::mutex> lock(AudioContext::get().mutex);
    AudioContext& audiocontext = AudioContext::get();
    audiocontext.outVolume = gain;
    return true;
}

/* Override */ int SDL_QueueAudio(sdl2::SDL_AudioDeviceID dev, const void *data, Uint32 len)
{
    LOG(LL_TRACE, LCF_SDL | LCF_SOUND, "%s call with %d bytes of data", __func__, len);

    if ((dev <= 1) || (dev > MAX_SDL_DEVICES) || (sdl_devices[dev].id == 0))
        return -1;

    if (sdl_device_sources[dev].empty())
        return -1;

    auto& sdl_source = sdl_device_sources[dev][0];
    if (sdl_source->source == AudioSource::SOURCE_CALLBACK) {
        /* We cannot queue samples when using the callback mechanism */
        return -1;
    }

    AudioContext& audiocontext = AudioContext::get();
    std::lock_guard<std::mutex> lock(audiocontext.mutex);

    /* We try to reuse a buffer that has been processed from the source */
    std::shared_ptr<AudioBuffer> ab = audiocontext.reuseBufferFromSourceOrCreate(sdl_source);

    /* Filling buffer */
    ab->samples.clear();
    ab->samples.insert(ab->samples.end(), static_cast<const uint8_t*>(data), &(static_cast<const uint8_t*>(data))[len]);
    ab->size = len;
    ab->update();
    sdl_source->queueBuffer(ab);

    /* If an underrun occurred, resume the playback */
    if (sdl_source->state == AudioSource::SOURCE_UNDERRUN)
        sdl_source->state = AudioSource::SOURCE_PLAYING;

    return 0;
}

/* Override */ Uint32 SDL_GetQueuedAudioSize(sdl2::SDL_AudioDeviceID dev)
{
    LOGTRACE(LCF_SDL | LCF_SOUND);

    if ((dev <= 1) || (dev > MAX_SDL_DEVICES) || (sdl_devices[dev].id == 0))
        return -1;

    if (sdl_device_sources[dev].empty())
        return -1;

    auto& sdl_source = sdl_device_sources[dev][0];
    if (sdl_source->source == AudioSource::SOURCE_CALLBACK) {
        /* We cannot get queue samples when using the callback mechanism */
        return 0;
    }

    std::lock_guard<std::mutex> lock(AudioContext::get().mutex);
    Uint32 qsize = sdl_source->queueSize() - sdl_source->getPosition();
    LOG(LL_DEBUG, LCF_SDL | LCF_SOUND, "Returning %d", qsize);
    return qsize;
}

/* Override */ void SDL_ClearQueuedAudio(sdl2::SDL_AudioDeviceID dev)
{
    LOGTRACE(LCF_SDL | LCF_SOUND);

    if ((dev <= 1) || (dev > MAX_SDL_DEVICES) || (sdl_devices[dev].id == 0))
        return;

    if (sdl_device_sources[dev].empty())
        return;

    auto& sdl_source = sdl_device_sources[dev][0];
    if (sdl_source->source == AudioSource::SOURCE_CALLBACK) {
        /* We cannot get queue samples when using the callback mechanism */
        return;
    }

    std::lock_guard<std::mutex> lock(AudioContext::get().mutex);
    /* We simulate clearing the queue by setting the position to the end
     * of the queue.
     */
    sdl_source->setPosition(sdl_source->queueSize());
}

/* Override */ bool SDL_BindAudioStreams(sdl3::SDL_AudioDeviceID devid, SDL_AudioStream * const *streams, int num_streams)
{
    LOGTRACE(LCF_SDL | LCF_SOUND);

    if ((devid <= 1) || (devid > MAX_SDL_DEVICES) || (sdl_devices[devid].id == 0))
        return false;

    /* SDL_AudioStream type is just our AudioSource object. For now, mapping a stream to a device
     * means that we start playing it. */
    for (int i = 0; i < num_streams; i++) {
        if (!streams[i]) {
            return false;
        }

        auto source = std::shared_ptr<AudioSource>(reinterpret_cast<AudioSource*>(streams[i]));

        if (source->state != AudioSource::SOURCE_INITIAL) {
            return false;
        }
        source->state = AudioSource::SOURCE_PLAYING;
        sdl_device_sources[devid].push_back(source);
    }

    return true;
}

/* Override */ bool SDL_BindAudioStream(sdl3::SDL_AudioDeviceID devid, SDL_AudioStream *stream)
{
    LOGTRACE(LCF_SDL | LCF_SOUND);

    if ((devid <= 1) || (devid > MAX_SDL_DEVICES) || (sdl_devices[devid].id == 0))
        return false;

    if (!stream) {
        return false;
    }
    auto source = std::shared_ptr<AudioSource>(reinterpret_cast<AudioSource*>(stream));

    if (source->state != AudioSource::SOURCE_INITIAL) {
        return false;
    }
    source->state = AudioSource::SOURCE_PLAYING;
    sdl_device_sources[devid].push_back(source);

    return true;
}

/* Override */ void SDL_UnbindAudioStreams(SDL_AudioStream * const *streams, int num_streams)
{
    LOGTRACE(LCF_SDL | LCF_SOUND);

    for (int i = 0; i < num_streams; i++) {
        if (!streams[i]) {
            continue;
        }
        auto source = std::shared_ptr<AudioSource>(reinterpret_cast<AudioSource*>(streams[i]));
        source->state = AudioSource::SOURCE_INITIAL;

        /* Remove source from all devices */
        for (int d = 1; d < MAX_SDL_DEVICES; d++) {
            if (sdl_devices[d].id != 0) {
                auto& sources = sdl_device_sources[d];
                sources.erase(std::remove_if(sources.begin(), sources.end(),
                    [&source](const std::shared_ptr<AudioSource>& s) { return s->id == source->id; }),
                    sources.end());
            }
        }
    }
}

/* Override */ void SDL_UnbindAudioStream(SDL_AudioStream *stream)
{
    LOGTRACE(LCF_SDL | LCF_SOUND);

    if (!stream) {
        return;
    }
    auto source = std::shared_ptr<AudioSource>(reinterpret_cast<AudioSource*>(stream));
    source->state = AudioSource::SOURCE_INITIAL;

    /* Remove source from all devices */
    for (int d = 1; d < MAX_SDL_DEVICES; d++) {
        if (sdl_devices[d].id != 0) {
            auto& sources = sdl_device_sources[d];
            sources.erase(std::remove_if(sources.begin(), sources.end(),
                [&source](const std::shared_ptr<AudioSource>& s) { return s->id == source->id; }),
                sources.end());
        }
    }
}

/* Get the device associated with a stream. This will be used on several functions. */
static int getDeviceFromStream(SDL_AudioStream *stream)
{
    auto source = std::shared_ptr<AudioSource>(reinterpret_cast<AudioSource*>(stream));
    for (int d = 1; d < MAX_SDL_DEVICES; d++) {
        if (sdl_devices[d].id != 0) {
            for (const auto& sdl_source : sdl_device_sources[d]) {
                if (sdl_source->id == source->id) {
                    return d;
                }
            }
        }
    }

    return 0;
}

/* Override */ sdl3::SDL_AudioDeviceID SDL_GetAudioStreamDevice(SDL_AudioStream *stream)
{
    LOGTRACE(LCF_SDL | LCF_SOUND);

    if (!stream) {
        return 0;
    }

    return getDeviceFromStream(stream);
}

/* Override */ SDL_AudioStream * SDL_CreateAudioStream(const sdl3::SDL_AudioSpec *src_spec, const sdl3::SDL_AudioSpec *dst_spec)
{
    LOG(LL_TRACE, LCF_SDL | LCF_SOUND, "%s call with input format %d, input channels %d and input freq %d", __func__, src_spec->format, src_spec->channels, src_spec->freq);
    auto source = AudioContext::get().createSource();

    source->format = sdlFormatToFormat(src_spec->format);
    source->channels = src_spec->channels;
    source->frequency = src_spec->freq;

    return reinterpret_cast<SDL_AudioStream*>(source.get());
}

/* Override */ bool SDL_GetAudioStreamFormat(SDL_AudioStream *stream, sdl3::SDL_AudioSpec *src_spec, sdl3::SDL_AudioSpec *dst_spec)
{
    LOGTRACE(LCF_SDL | LCF_SOUND);

    auto source = std::shared_ptr<AudioSource>(reinterpret_cast<AudioSource*>(stream));
    if (src_spec) {
        src_spec->format = formatToSdlFormat(source->format);
        src_spec->channels = source->channels;
        src_spec->freq = source->frequency;
    }
    return true;
}

/* Override */ bool SDL_SetAudioStreamFormat(SDL_AudioStream *stream, const sdl3::SDL_AudioSpec *src_spec, const sdl3::SDL_AudioSpec *dst_spec)
{
    LOGTRACE(LCF_SDL | LCF_SOUND);

    auto source = std::shared_ptr<AudioSource>(reinterpret_cast<AudioSource*>(stream));
    if (src_spec && source) {
        source->format = sdlFormatToFormat(src_spec->format);
        source->channels = src_spec->channels;
        source->frequency = src_spec->freq;
    }
    return true;
}

/* Override */ float SDL_GetAudioStreamFrequencyRatio(SDL_AudioStream *stream)
{
    LOGTRACE(LCF_SDL | LCF_SOUND);

    auto source = std::shared_ptr<AudioSource>(reinterpret_cast<AudioSource*>(stream));
    return source->pitch;
}

/* Override */ bool SDL_SetAudioStreamFrequencyRatio(SDL_AudioStream *stream, float ratio)
{
    LOGTRACE(LCF_SDL | LCF_SOUND);

    auto source = std::shared_ptr<AudioSource>(reinterpret_cast<AudioSource*>(stream));
    source->pitch = ratio;
    return true;
}

/* Override */ float SDL_GetAudioStreamGain(SDL_AudioStream *stream)
{
    LOGTRACE(LCF_SDL | LCF_SOUND);

    auto source = std::shared_ptr<AudioSource>(reinterpret_cast<AudioSource*>(stream));
    return source->volume;
}

/* Override */ bool SDL_SetAudioStreamGain(SDL_AudioStream *stream, float gain)
{
    LOGTRACE(LCF_SDL | LCF_SOUND);

    auto source = std::shared_ptr<AudioSource>(reinterpret_cast<AudioSource*>(stream));
    source->volume = gain;
    return true;
}

/* Override */ int * SDL_GetAudioStreamInputChannelMap(SDL_AudioStream *stream, int *count)
{
    LOGTRACE(LCF_SDL | LCF_SOUND);
    return nullptr;
}

/* Override */ int * SDL_GetAudioStreamOutputChannelMap(SDL_AudioStream *stream, int *count)
{
    LOGTRACE(LCF_SDL | LCF_SOUND);
    return nullptr;
}

/* Override */ bool SDL_SetAudioStreamInputChannelMap(SDL_AudioStream *stream, const int *chmap, int count)
{
    LOGTRACE(LCF_SDL | LCF_SOUND | LCF_TODO);
    return true;
}

/* Override */ bool SDL_SetAudioStreamOutputChannelMap(SDL_AudioStream *stream, const int *chmap, int count)
{
    LOGTRACE(LCF_SDL | LCF_SOUND | LCF_TODO);
    return true;
}

/* Override */ bool SDL_PutAudioStreamData(SDL_AudioStream *stream, const void *buf, int len)
{
    LOG(LL_TRACE, LCF_SDL | LCF_SOUND, "%s called with %d bytes", __func__, len);
    auto source = std::shared_ptr<AudioSource>(reinterpret_cast<AudioSource*>(stream));

    AudioContext& audiocontext = AudioContext::get();
    std::lock_guard<std::mutex> lock(audiocontext.mutex);

    /* We try to reuse a buffer that has been processed from the source */
    std::shared_ptr<AudioBuffer> ab = audiocontext.reuseBufferFromSourceOrCreate(source);

    /* Filling buffer */
    ab->samples.clear();
    ab->samples.insert(ab->samples.end(), static_cast<const uint8_t*>(buf), &(static_cast<const uint8_t*>(buf))[len]);
    ab->size = len;
    ab->update();
    source->queueBuffer(ab);

    /* If an underrun occurred, resume the playback */
    if (source->state == AudioSource::SOURCE_UNDERRUN)
        source->state = AudioSource::SOURCE_PLAYING;

    return true;
}

/* Override */ int SDL_GetAudioStreamData(SDL_AudioStream *stream, void *buf, int len)
{
    LOG(LL_TRACE, LCF_SDL | LCF_SOUND | LCF_TODO, "%s called with %d bytes", __func__, len);
    auto source = std::shared_ptr<AudioSource>(reinterpret_cast<AudioSource*>(stream));

    std::lock_guard<std::mutex> lock(AudioContext::get().mutex);

    /* TODO: For now, we just return samples from the first buffer. */

    std::shared_ptr<AudioBuffer> ab = source->buffer(0);
    if (!ab)
        return 0;

    uint8_t* samples;
    int returned_samples = ab->getSamples(samples, len / source->frameToByteRatio(), 0, false);
    int returned_bytes = returned_samples * source->frameToByteRatio();

    if (returned_samples > 0) {
        if (returned_bytes > len)
            returned_bytes = len;

        memcpy(buf, samples, returned_bytes);
    }

    return returned_bytes;
}

/* Override */ int SDL_GetAudioStreamAvailable(SDL_AudioStream *stream)
{
    LOGTRACE(LCF_SDL | LCF_SOUND | LCF_TODO);

    /* TODO: For now, we just return the number of available unconverted bytes */
    auto source = std::shared_ptr<AudioSource>(reinterpret_cast<AudioSource*>(stream));
    if (!source)
        return -1;

    int available_samples = source->queueSize() - source->getPosition();
    return available_samples * source->frameToByteRatio();
}

/* Override */ int SDL_GetAudioStreamQueued(SDL_AudioStream *stream)
{
    LOGTRACE(LCF_SDL | LCF_SOUND);

    auto source = std::shared_ptr<AudioSource>(reinterpret_cast<AudioSource*>(stream));
    if (!source)
        return -1;

    int available_samples = source->queueSize() - source->getPosition();
    return available_samples * source->frameToByteRatio();
}

/* Override */ bool SDL_FlushAudioStream(SDL_AudioStream *stream)
{
    LOGTRACE(LCF_SDL | LCF_SOUND);
    return true;
}

/* Override */ bool SDL_ClearAudioStream(SDL_AudioStream *stream)
{
    LOGTRACE(LCF_SDL | LCF_SOUND);

    auto source = std::shared_ptr<AudioSource>(reinterpret_cast<AudioSource*>(stream));
    if (!source)
        return -1;

    std::lock_guard<std::mutex> lock(AudioContext::get().mutex);
    source->clearBuffers();
    return true;
}

/* Override */ bool SDL_PauseAudioStreamDevice(SDL_AudioStream *stream)
{
    LOGTRACE(LCF_SDL | LCF_SOUND);

    if (!stream)
        return -1;

    int stream_device = getDeviceFromStream(stream);

    if (stream_device == 0)
        return false;

    std::lock_guard<std::mutex> lock(AudioContext::get().mutex);
    for (auto& sdl_source : sdl_device_sources[stream_device]) {
        sdl_source->state = AudioSource::SOURCE_PAUSED;
    }
    return true;
}

/* Override */ bool SDL_ResumeAudioStreamDevice(SDL_AudioStream *stream)
{
    LOGTRACE(LCF_SDL | LCF_SOUND);

    if (!stream)
        return -1;

    int stream_device = getDeviceFromStream(stream);

    if (stream_device == 0)
        return false;

    std::lock_guard<std::mutex> lock(AudioContext::get().mutex);
    for (auto& sdl_source : sdl_device_sources[stream_device]) {
        sdl_source->state = AudioSource::SOURCE_PLAYING;
    }
    return true;
}

/* Override */ bool SDL_AudioStreamDevicePaused(SDL_AudioStream *stream)
{
    LOGTRACE(LCF_SDL | LCF_SOUND);

    /* Only check this stream status */
    auto source = std::shared_ptr<AudioSource>(reinterpret_cast<AudioSource*>(stream));
    if (!source)
        return false;

    return source->state == AudioSource::SOURCE_PAUSED;
}

/* Override */ void SDL_LockAudio(void)
{
    LOGTRACE(LCF_SDL | LCF_SOUND);
    return SDL_LockAudioDevice(1);
}

/* Override */ void SDL_LockAudioDevice(sdl2::SDL_AudioDeviceID dev)
{
    LOGTRACE(LCF_SDL | LCF_SOUND);
    if (ThreadManager::getThreadId() == AudioContext::get().audio_thread)
        return;
    mutex.lock();
}

/* Override */ void SDL_UnlockAudio(void)
{
    LOGTRACE(LCF_SDL | LCF_SOUND);
    return SDL_UnlockAudioDevice(1);
}

/* Override */ void SDL_UnlockAudioDevice(sdl2::SDL_AudioDeviceID dev)
{
    LOGTRACE(LCF_SDL | LCF_SOUND);
    if (ThreadManager::getThreadId() == AudioContext::get().audio_thread)
        return;
    mutex.unlock();
}

/* Override */ void SDL_CloseAudio(void)
{
    LOGTRACE(LCF_SDL | LCF_SOUND);
    return SDL_CloseAudioDevice(1);
}

/* Override */ void SDL_CloseAudioDevice(sdl2::SDL_AudioDeviceID dev)
{
    LOG(LL_TRACE, LCF_SDL | LCF_SOUND, "%s called with dev %d", __func__, dev);

    if (dev <= 1 || dev > MAX_SDL_DEVICES)
        return;

    AudioContext& audiocontext = AudioContext::get();
    std::lock_guard<std::mutex> lock(audiocontext.mutex);

    if (sdl_devices[dev].id != 0) {
        /* Remove the source from the audio context */
        for (auto& sdl_source : sdl_device_sources[dev]) {
            audiocontext.deleteSource(sdl_source->id);
        }
        sdl_device_sources[dev].clear();
        sdl_devices[dev].id = 0;
    }
}

}
