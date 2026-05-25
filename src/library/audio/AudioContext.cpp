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

#include "AudioContext.h"
#include "AudioBuffer.h"
#include "AudioSource.h"
#ifdef __linux__
#include "AudioPlayerAlsa.h"
#elif defined(__APPLE__) && defined(__MACH__)
#include "AudioPlayerCoreAudio.h"
#endif

#include "logging.h"
#include "global.h" // Global::shared_config
#include "GlobalState.h"
#include "checkpoint/ThreadManager.h" // isMainThread()

#include <stdint.h>
#include <unistd.h>

#define MAXBUFFERS 2048 // Max I've seen so far: 960
#define MAXSOURCES 256 // Max I've seen so far: 112

namespace libtas {

/* Helper function to convert ticks into a number of bytes in the audio buffer */
static int ticksToBytes(struct timespec ticks, int alignSize, int frequency)
{
    if (alignSize <= 0 || frequency <= 0)
        return 0;

    static int64_t samples_frac = 0;
    uint64_t nsecs = static_cast<uint64_t>(ticks.tv_sec) * 1000000000 + ticks.tv_nsec;
    uint64_t samples = (nsecs * frequency) / 1000000000;
    samples_frac += (nsecs * frequency) % 1000000000;
    if (samples_frac >= 500000000) {
        samples_frac -= 1000000000;
        samples++;
    }
    uint64_t bytes = samples * alignSize;
    return static_cast<int>(bytes);
}

/* Helper function to convert a number of samples in the audio buffer into ticks */
static struct timespec samplesToTicks(int nbSamples, int frequency)
{
    if (frequency <= 0)
        return {0, 0};

    static int64_t nsec_frac = 0;
    uint64_t nsecs = (static_cast<uint64_t>(nbSamples) * 1000000000) / frequency;

    struct timespec ticks;
    ticks.tv_sec = nsecs / 1000000000;
    ticks.tv_nsec = nsecs % 1000000000;

    nsec_frac += (static_cast<uint64_t>(nbSamples) * 1000000000) % frequency;
    if (nsec_frac >= frequency) {
        nsec_frac -= frequency;
        ticks.tv_nsec++;
    }
    return ticks;
}

AudioBuffer::SampleFormat AudioContext::default_format = AudioBuffer::SAMPLE_FMT_S16;
int AudioContext::default_channels = 2;
int AudioContext::default_frequency = 44100;

AudioContext::AudioContext(void)
{
    volume = 1.0f;
    audio_thread = 0;
    format = AudioBuffer::SAMPLE_FMT_UNKNOWN;
    channels = 0;
    frequency = 0;
    is_loopback = false;
    paused = false;
    init();
}

AudioContext& AudioContext::get() {
    static AudioContext* instance = new AudioContext;
    return *instance;
}

void AudioContext::init(void)
{
    AudioBuffer::SampleFormat format = AudioBuffer::SAMPLE_FMT_UNKNOWN;
    switch (Global::shared_config.audio_bitdepth) {
        case 8:
            format = AudioBuffer::SAMPLE_FMT_U8;
            break;
        case 16:
            format = AudioBuffer::SAMPLE_FMT_S16;
            break;
        case 32:
            format = AudioBuffer::SAMPLE_FMT_FLT;
            break;
    }

    initValues(format, Global::shared_config.audio_channels, Global::shared_config.audio_frequency);
}

void AudioContext::initDefaults(void)
{
    initValues(default_format, default_channels, default_frequency);
}

void AudioContext::initValues(AudioBuffer::SampleFormat new_format, int new_channels, int new_frequency)
{
    if (format == AudioBuffer::SAMPLE_FMT_UNKNOWN)
        format = new_format;
    
    if (!channels)
        channels = new_channels;
    
    if (!frequency)
        frequency = new_frequency;

    bytes_per_sample = channels * AudioBuffer::formatToBitDepth(format) / 8;
}

bool AudioContext::isInited(void) const
{
    return format != AudioBuffer::SAMPLE_FMT_UNKNOWN && channels > 0 && frequency > 0;
}

std::shared_ptr<AudioBuffer> AudioContext::createBuffer(void)
{
    if (buffers.size() >= MAXBUFFERS)
        return nullptr;

    /* Check if we can recycle a deleted buffer */
    if (!buffers_pool.empty()) {
        buffers.push_front(buffers_pool.front());
        buffers_pool.pop_front();
        return buffers.front();
    }

    /* If not, we create a new buffer.
     * The next available id equals the size of the buffer list + 1
     * (ids must start by 1, because 0 is reserved for no buffer)
     */
    auto newab = std::make_shared<AudioBuffer>();
    newab->id = buffers.size() + 1;
    buffers.push_front(newab);
    return newab;
}

std::shared_ptr<AudioBuffer> AudioContext::reuseBufferFromSourceOrCreate(AudioSource* source)
{
    auto buffer = source->reuseBuffer();
    if (buffer)
        return buffer;

    /* Apply the source specs to the buffer */
    buffer = createBuffer();
    if (!buffer)
        return nullptr;

    buffer->format = source->format;
    buffer->channels = source->channels;
    buffer->frequency = source->frequency;
    buffer->update();

    return buffer;
}

void AudioContext::deleteBuffer(int id)
{
    buffers.remove_if([id,this](std::shared_ptr<AudioBuffer> const& buffer)
        {
            if (buffer->id == id) {
                /* Push the deleted buffer into the pool */
                buffers_pool.push_front(buffer);
                return true;
            }
            return false;
        });
}

bool AudioContext::isBuffer(int id) const
{
    for (auto const& buffer : buffers) {
        if (buffer->id == id)
            return true;
    }

    return false;
}

std::shared_ptr<AudioBuffer> AudioContext::getBuffer(int id) const
{
    for (auto& buffer : buffers) {
        if (buffer->id == id)
            return buffer;
    }

    return nullptr;
}

std::shared_ptr<AudioSource> AudioContext::createSource(void)
{
    if (sources.size() >= MAXSOURCES)
        return nullptr;

    /* Check if we can recycle a deleted source */
    if (!sources_pool.empty()) {
        sources.push_front(sources_pool.front());
        sources_pool.pop_front();
        sources.front()->init();
        return sources.front();
    }

    /* If not, we create a new source.
     * The next available id equals the size of the source list + 1
     * (ids must start by 1, because 0 is reserved for no source)
     */
    auto newas = std::make_shared<AudioSource>();
    newas->id = sources.size() + 1;
    sources.push_front(newas);
    return newas;
}

void AudioContext::deleteSource(int id)
{
    sources.remove_if([id,this](std::shared_ptr<AudioSource> const& source)
        {
            if (source->id == id) {
                /* Push the deleted source into the pool */
                sources_pool.push_front(source);
                return true;
            }
            return false;
        });
}

bool AudioContext::isSource(int id) const
{
    for (auto& source : sources) {
        if (source->id == id)
            return true;
    }

    return false;
}

std::shared_ptr<AudioSource> AudioContext::getSource(int id) const
{
    for (auto& source : sources) {
        if (source->id == id)
            return source;
    }

    return nullptr;
}

void AudioContext::mixAllSources(int nbSamples)
{
    return mixAllSources(samplesToTicks(nbSamples, frequency));
}

void AudioContext::mixAllSources(struct timespec ticks)
{
    /* Check that ticks is positive! */
    if (ticks.tv_sec < 0) {
        LOG(LL_ERROR, LCF_SOUND, "Negative number of ticks for audio mixing!");
        return;
    }

    /* Skip when no ticks to mix */
    if (ticks.tv_sec == 0 && ticks.tv_nsec == 0) {
        samples_byte_size = 0;
        samples_size = 0;
        return;
    }

    /* Check if at least one source will output audio samples. If not, we can return immediately. */
    bool will_output = false;
    for (const auto& source : sources) {
        will_output |= source->willOutput();
    }
    if (!will_output) {
        samples_byte_size = 0;
        samples_size = 0;
        return;
    }

    /* Now that we will output audio, we must make sure that our output parameters are set */
    if (!isInited()) {
        initDefaults();
    }

    if (bytes_per_sample <= 0 || frequency <= 0) {
        LOG(LL_ERROR, LCF_SOUND, "Invalid output audio format, alignment %d frequency %d", bytes_per_sample, frequency);
        samples_byte_size = 0;
        samples_size = 0;
        return;
    }

    samples_byte_size = ticksToBytes(ticks, bytes_per_sample, frequency);
  	/* Save the actual number of samples and size */
  	samples_size = samples_byte_size / bytes_per_sample;

    LOG(LL_DEBUG, LCF_SOUND, "Start mixing about %d samples", samples_size);

    /* Silent the output buffer */
    samples_data.assign(samples_byte_size, AudioBuffer::formatToSilenceByte(format));

    if (paused) return;

    pthread_t mix_thread = ThreadManager::getThreadId();

    mutex.lock();

    for (auto& source : sources) {
        /* If an audio source is filled asynchronously, and we will underrun,
         * try to wait until the source is filled.
         */

        if ((source->source == AudioSource::SOURCE_STREAMING_CONTINUOUS) &&
            !source->callback &&
            audio_thread &&
            (mix_thread != audio_thread) &&
            source->willEnd(ticks)) {

            LOG(LL_WARN, LCF_SOUND, "Audio mixing will underrun, waiting for the game to send audio samples");
            int i;
            for (i=0; i<1000; i++) {

                mutex.unlock();
                NATIVECALL(usleep(100));
                mutex.lock();

                if (!source->willEnd(ticks))
                    break;
            }
            if (i == 1000) {
                LOG(LL_WARN, LCF_SOUND, "    Timeout");
            }
        }

        source->mixWith(ticks, samples_data.data(), samples_byte_size, format, channels, frequency, volume);
    }
    
    mutex.unlock();

    if (!is_loopback && !Global::shared_config.audio_mute) {
        /* Play the music */
#ifdef __linux__
        AudioPlayerAlsa::play(*this);
#elif defined(__APPLE__) && defined(__MACH__)
        AudioPlayerCoreAudio::play(*this);
#endif
    }
}

}
