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

#include "../logging.h"
#include "AudioContext.h"
#include "AudioPlayer.h"

#define MAXBUFFERS 2048 // Max I've seen so far: 960
#define MAXSOURCES 256 // Max I've seen so far: 112

AudioContext audiocontext;

/* Helper function to convert ticks into a number of bytes in the audio buffer */
static int ticksToBytes(struct timespec ticks, int alignSize, int frequency)
{
    static int64_t samples_frac = 0;
    uint64_t nsecs = ((uint64_t) ticks.tv_sec) * 1000000000 + ticks.tv_nsec;
    uint64_t samples = (nsecs * frequency) / 1000000000;
    samples_frac += (nsecs * frequency) % 1000000000;
    if (samples_frac >= 500000000) {
        samples_frac -= 1000000000;
        samples++;
    }
    uint64_t bytes = samples * alignSize;
    return (int) bytes;
}

AudioContext::AudioContext(void)
{
    outVolume = 1.0f;
    outBitDepth = 16;
    outNbChannels = 2;
    outAlignSize = 4;
    outFrequency = 44100;
}

int AudioContext::createBuffer(void)
{
    std::lock_guard<std::mutex> lock(mutex);
    
    if ((! buffers.empty()) && buffers.front()->id >= MAXBUFFERS)
        return -1;

    AudioBuffer* newab = new AudioBuffer;

    if (buffers.empty()) {
        newab->id = 1;
    }
    else {
        /* Get the id of the last AudioBuffer
         * (situated at the beginning of the list) */
        int last_id = buffers.front()->id;
        newab->id = last_id + 1;
    }
    buffers.push_front(newab);

    return newab->id;
}

void AudioContext::deleteBuffer(int id)
{
    std::lock_guard<std::mutex> lock(mutex);
    
    buffers.remove_if([id](AudioBuffer* const& buffer)
        {
            if (buffer->id == id) {
                delete buffer;
                return true;
            }
            return false;
        });
}

bool AudioContext::isBuffer(int id)
{
    for (auto& buffer : buffers) {
        if (buffer->id == id)
            return true;
    }

    return false;
}

AudioBuffer* AudioContext::getBuffer(int id)
{
    for (auto& buffer : buffers) {
        if (buffer->id == id)
            return buffer;
    }

    return nullptr;
}

int AudioContext::createSource(void)
{
    std::lock_guard<std::mutex> lock(mutex);
    
    if ((! sources.empty()) && sources.front()->id >= MAXSOURCES)
        return -1;

    AudioSource* newas = new AudioSource;

    if (sources.empty()) {
        newas->id = 1;
    }
    else {
        /* Get the id of the last AudioSource
         * (situated at the beginning of the list) */
        int last_id = sources.front()->id;
        newas->id = last_id + 1;
    }
    sources.push_front(newas);

    return newas->id;
}

void AudioContext::deleteSource(int id)
{
    std::lock_guard<std::mutex> lock(mutex);
    
    sources.remove_if([id](AudioSource* const& source)
        {
            if (source->id == id) {
                delete source;
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

AudioSource* AudioContext::getSource(int id)
{
    for (auto& source : sources) {
        if (source->id == id)
            return source;
    }

    return nullptr;
}

void AudioContext::mixAllSources(struct timespec ticks)
{
    //std::lock_guard<std::mutex> lock(mutex);

    /* Check that ticks is positive! */
    if (ticks.tv_sec < 0) {
        debuglog(LCF_SOUND | LCF_FRAME | LCF_ERROR, "Negative number of ticks for audio mixing!");
        return;
    }

    outBytes = ticksToBytes(ticks, outAlignSize, outFrequency);
	/* Save the actual number of samples and size */
	outNbSamples = outBytes / outAlignSize;

    debuglog(LCF_SOUND | LCF_FRAME, "Start mixing about ", outNbSamples, " samples");

    /* Silent the output buffer */
    if (outBitDepth == 8) // Unsigned 8-bit samples
        outSamples.assign(outBytes, 0x80);
    if (outBitDepth == 16) // Signed 16-bit samples
        outSamples.assign(outBytes, 0);

    for (auto& source : sources) {
        source->mixWith(ticks, &outSamples[0], outBytes, outBitDepth, outNbChannels, outFrequency, outVolume);
    }

#ifdef LIBTAS_ENABLE_SOUNDPLAYBACK
    /* Play the music */
    audioplayer.play(*this);
#endif
}

