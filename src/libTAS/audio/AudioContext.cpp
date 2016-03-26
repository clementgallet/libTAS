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

#define MAXBUFFERS 2048 // TODO: put correct value
#define MAXSOURCES 256

AudioContext audiocontext;

/* Helper function to convert ticks into a number of bytes in the audio buffer */
static int ticksToBytes(struct timespec ticks, int bitDepth, int nbChannels, int frequency)
{
    uint64_t nsecs = ((uint64_t) ticks.tv_sec) * 1000000000 + ticks.tv_nsec;
    uint64_t bytes = (nsecs * (bitDepth / 8) * nbChannels * frequency) / 1000000000;
    return (int) bytes;
}

AudioContext::AudioContext(void)
{
    outVolume = 1.0f;
    outBitDepth = 16;
    outNbChannels = 2;
    outFrequency = 44100;
    
    /* TEMP! WAV out */
    file = SndfileHandle("test.wav", SFM_WRITE, SF_FORMAT_WAV | SF_FORMAT_PCM_16, outNbChannels, outFrequency);
}

int AudioContext::createBuffer(void)
{
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
    int outBytes = ticksToBytes(ticks, outBitDepth, outNbChannels, outFrequency);
    debuglog(LCF_SOUND | LCF_FRAME, "Start mixing ", outBytes, " of buffers");

    /* Silent the output buffer */
    if (outBitDepth == 8) // Unsigned 8-bit samples
        outSamples.assign(outBytes, 0x80);
    if (outBitDepth == 16) // Signed 16-bit samples
        outSamples.assign(outBytes, 0);

    for (auto& source : sources) {
        source->mixWith(ticks, &outSamples[0], outBytes, outBitDepth, outNbChannels, outFrequency, outVolume);
    }

    /* TEMP! WAV output */
    if (outBitDepth == 16)
        file.write((int16_t*)&outSamples[0], outBytes/2);
}



