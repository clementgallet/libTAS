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

#include "../logging.h"
#include "AudioContext.h"
#ifdef __linux__
#include "AudioPlayerAlsa.h"
#elif defined(__APPLE__) && defined(__MACH__)
#include "AudioPlayerCoreAudio.h"
#endif
#include "../global.h" // shared_config

#include <stdint.h>
#include <unistd.h>

#define MAXBUFFERS 2048 // Max I've seen so far: 960
#define MAXSOURCES 256 // Max I've seen so far: 112

namespace libtas {

AudioContext audiocontext;

/* Helper function to convert ticks into a number of bytes in the audio buffer */
static int ticksToBytes(struct timespec ticks, int alignSize, int frequency)
{
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
    static int64_t nsec_frac = 0;
    uint64_t nsecs = (static_cast<uint64_t>(nbSamples) * 1000000000) / frequency;

    struct timespec ticks;
    ticks.tv_sec = nsecs / 1000000000;
    ticks.tv_nsec = nsecs % 1000000000;

    nsec_frac += (static_cast<uint64_t>(nbSamples) * 1000000000) % frequency;
    if (nsec_frac > frequency) {
        nsec_frac -= frequency;
        ticks.tv_nsec++;
    }
    return ticks;
}


AudioContext::AudioContext(void)
{
    outVolume = 1.0f;
    audio_thread = 0;
    init();
}

void AudioContext::init(void)
{
    outBitDepth = shared_config.audio_bitdepth;
    outNbChannels = shared_config.audio_channels;
    outFrequency = shared_config.audio_frequency;
    outAlignSize = outNbChannels * outBitDepth / 8;
    isLoopback = false;
}

int AudioContext::createBuffer(void)
{
    if (buffers.size() >= MAXBUFFERS)
        return -1;

    /* Check if we can recycle a deleted buffer */
    if (!buffers_pool.empty()) {
        buffers.push_front(buffers_pool.front());
        buffers_pool.pop_front();
        return buffers.front()->id;
    }

    /* If not, we create a new buffer.
     * The next available id equals the size of the buffer list + 1
     * (ids must start by 1, because 0 is reserved for no buffer)
     */
    auto newab = std::make_shared<AudioBuffer>();
    newab->id = buffers.size() + 1;
    buffers.push_front(newab);
    return newab->id;
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

bool AudioContext::isBuffer(int id)
{
    for (auto const& buffer : buffers) {
        if (buffer->id == id)
            return true;
    }

    return false;
}

std::shared_ptr<AudioBuffer> AudioContext::getBuffer(int id)
{
    for (auto& buffer : buffers) {
        if (buffer->id == id)
            return buffer;
    }

    return nullptr;
}

int AudioContext::createSource(void)
{
    if (sources.size() >= MAXSOURCES)
        return -1;

    /* Check if we can recycle a deleted source */
    if (!sources_pool.empty()) {
        sources.push_front(sources_pool.front());
        sources_pool.pop_front();
        sources.front()->init();
        return sources.front()->id;
    }

    /* If not, we create a new source.
     * The next available id equals the size of the source list + 1
     * (ids must start by 1, because 0 is reserved for no source)
     */
    auto newas = std::make_shared<AudioSource>();
    newas->id = sources.size() + 1;
    sources.push_front(newas);
    return newas->id;
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

bool AudioContext::isSource(int id)
{
    for (auto& source : sources) {
        if (source->id == id)
            return true;
    }

    return false;
}

std::shared_ptr<AudioSource> AudioContext::getSource(int id)
{
    for (auto& source : sources) {
        if (source->id == id)
            return source;
    }

    return nullptr;
}

void AudioContext::mixAllSources(int nbSamples)
{
    return mixAllSources(samplesToTicks(nbSamples, outFrequency));
}


void AudioContext::mixAllSources(struct timespec ticks)
{
    /* Check that ticks is positive! */
    if (ticks.tv_sec < 0) {
        debuglogstdio(LCF_SOUND | LCF_ERROR, "Negative number of ticks for audio mixing!");
        return;
    }

    outBytes = ticksToBytes(ticks, outAlignSize, outFrequency);
  	/* Save the actual number of samples and size */
  	outNbSamples = outBytes / outAlignSize;

    debuglogstdio(LCF_SOUND, "Start mixing about %d samples", outNbSamples);

    /* Silent the output buffer */
    if (outBitDepth == 8) // Unsigned 8-bit samples
        outSamples.assign(outBytes, 0x80);
    if (outBitDepth == 16) // Signed 16-bit samples
        outSamples.assign(outBytes, 0);

    pthread_t mix_thread = ThreadManager::getThreadId();

    mutex.lock();

    for (auto& source : sources) {
        /* If an audio source is filled asynchronously, and we will underrun,
         * try to wait until the source is filled.
         */

        if ((source->source == AudioSource::SOURCE_STREAMING_CONTINUOUS) &&
            audio_thread &&
            (mix_thread != audio_thread) &&
            source->willEnd(ticks)) {

            debuglogstdio(LCF_SOUND | LCF_WARNING, "Audio mixing will underrun, waiting for the game to send audio samples");
            int i;
            for (i=0; i<1000; i++) {

                mutex.unlock();
                NATIVECALL(usleep(100));
                mutex.lock();

                if (!source->willEnd(ticks))
                    break;
            }
            if (i == 1000) {
                debuglogstdio(LCF_SOUND | LCF_WARNING, "    Timeout");
            }
        }

        source->mixWith(ticks, &outSamples[0], outBytes, outBitDepth, outNbChannels, outFrequency, outVolume);
    }
    
    mutex.unlock();

    if (!audiocontext.isLoopback && !shared_config.audio_mute) {
        /* Play the music */
#ifdef __linux__
        AudioPlayerAlsa::play(*this);
#elif defined(__APPLE__) && defined(__MACH__)
        AudioPlayerCoreAudio::play(*this);
#endif
    }
}

}
