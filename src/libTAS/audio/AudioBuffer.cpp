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

#include "AudioBuffer.h"
#include "audiomixing.h"
#include <iterator>     // std::back_inserter
#include <algorithm>    // std::copy
#include "../logging.h"

#define MAXBUFFERS 256 // TODO: put correct value
#define MAXSOURCES 256

/* Helper function to convert ticks into a number of bytes in the audio buffer */
static int ticksToBytes(struct timespec ticks, int bitDepth, int nbChannels, int frequency)
{
    uint64_t nsecs = ((uint64_t) ticks.tv_sec) * 1000000000 + ticks.tv_nsec;
    uint64_t bytes = (nsecs * (bitDepth / 8) * nbChannels * frequency) / 1000000000;
    return (int) bytes;
}

AudioContext audiocontext;

AudioBuffer::AudioBuffer(void)
{
    id = 0;
    bitDepth = 16;
    nbChannels = 1;
    frequency = 0;
    size = 0;
    processed = false;
}

AudioSource::AudioSource(void)
{
    id = 0;
    position = 0;
    volume = 1.0f; // Default from openal-soft
    source = SOURCE_UNDETERMINED;
    looping = false;
    state = SOURCE_INITIAL;
    queue_index = 0;
}

int AudioSource::nbQueue()
{
    return buffer_queue.size();
}

int AudioSource::nbQueueProcessed()
{
    return queue_index;
}

int AudioSource::queueSize()
{
    int totalSize = 0;
    for (auto& buffer : buffer_queue) {
        totalSize += buffer->size;
    }
    return totalSize;
}

int AudioSource::getPosition()
{
    int totalPos = 0;
    for (int i=0; i<queue_index; i++) {
        totalPos += buffer_queue[i]->size;
    }
    totalPos += position;

    return totalPos;
}

void AudioSource::setPosition(int pos)
{
    int localPos = pos;
    
    if (looping) {
        localPos %= queueSize();
    }

    for (int i=0; i<buffer_queue.size(); i++) {
        AudioBuffer* ab = buffer_queue[i];
        if (localPos == -1) {
            /* We have set the position already.
             * We have to mark the remaining buffers as unprocessed
             */
            ab->processed = false;
        }
        else if (localPos < ab->size) {
            /* We set the position in this buffer */

            /* Fix unaligned position */
            localPos -= localPos % (ab->nbChannels * ab->bitDepth / 8);
            position = localPos;
            ab->processed = false;
            localPos = -1;
        }
        else {
            /* We traverse the buffer */
            ab->processed = true;
            localPos -= ab->size;
        }
    }
}

void AudioSource::mixWith( struct timespec ticks, uint8_t* outSamples, int outBytes, int outBitDepth, int outNbChannels, int outFrequency, float outVolume)
{
    //debuglog(LCF_SOUND | LCF_FRAME, "Start mixing buffer ", id, " of state ", state);
    if (state != SOURCE_PLAYING)
        return;

    if (buffer_queue.empty())
        return;

    debuglog(LCF_SOUND | LCF_FRAME, "Start mixing source ", id);

    /* Mixing source volume and master volume.
     * Taken from openAL doc:
     * "The implementation is free to clamp the total gain (effective gain
     * per-source multiplied by the listener gain) to one to prevent overflow."
     *
     * TODO: This is where we can support panning.
     */
    float resultVolume = std::max(1.0f, volume * outVolume);
    int lvas = (int)(resultVolume * 65536);
    int rvas = (int)(resultVolume * 65536);

    AudioBuffer* curBuf = buffer_queue[queue_index];

    /* Number of bytes to advance in the buffer samples */
    int inBytes = ticksToBytes(ticks, curBuf->bitDepth, curBuf->nbChannels, curBuf->frequency);

    int oldPosition = position;
    int newPosition = position + inBytes;

    if (newPosition < curBuf->size) {
        /* We did not reach the end of the buffer, easy case */

        position = newPosition;
        debuglog(LCF_SOUND | LCF_FRAME, "Source ", id, " plays buffer ", curBuf->id, " in range ", oldPosition, " - ", position);
        MixFromToInternal(&curBuf->samples[oldPosition], inBytes,
                outSamples, outBytes,
                curBuf->bitDepth, curBuf->nbChannels,
                outBitDepth, outNbChannels,
                false, lvas, rvas);
    }
    else {
        /* We reached the end of the buffer
         *
         * Let's take the dumb solution:
         * just copy the rest of the buffer queue in a new array, while supporting looping
         */

        /* TODO: In the special case of last element of the queue and no looping,
         * we don't need any copy
         */

        /* Use the vector constructor for the first copy */
        /* TODO: Keep the vector allocated somewhere? */
        std::vector<uint8_t> unwrapSamples (&curBuf->samples[oldPosition], &curBuf->samples[curBuf->size]);
        unwrapSamples.reserve(inBytes);

        int remainingBytes = inBytes - unwrapSamples.size();
        int queue_size = buffer_queue.size();
        int finalIndex;
        int finalPos;

        /* Our for loop conditions are different if we are looping or not */
        if (looping) {
            for (int i=(queue_index+1)%queue_size; remainingBytes>0; i=(i+1)%queue_size) {
                AudioBuffer* loopbuf = buffer_queue[i];
                if (remainingBytes > loopbuf->size) {
                    std::copy(&loopbuf->samples[0], &loopbuf->samples[loopbuf->size], std::back_inserter(unwrapSamples));
                    loopbuf->processed = true; // Are buffers in a loop ever processed??
                }
                else {
                    std::copy(&loopbuf->samples[0], &loopbuf->samples[remainingBytes], std::back_inserter(unwrapSamples));
                    finalIndex = i;
                    finalPos = remainingBytes;
                }
                remainingBytes = inBytes - unwrapSamples.size();
            }
        }
        else {
            for (int i=queue_index+1; (remainingBytes>0) && (i<queue_size); i++) {
                AudioBuffer* loopbuf = buffer_queue[i];
                if (remainingBytes > loopbuf->size) {
                    std::copy(&loopbuf->samples[0], &loopbuf->samples[loopbuf->size], std::back_inserter(unwrapSamples));
                    loopbuf->processed = true;
                }
                else {
                    std::copy(&loopbuf->samples[0], &loopbuf->samples[remainingBytes], std::back_inserter(unwrapSamples));
                    finalIndex = i;
                    finalPos = remainingBytes;
                }
                remainingBytes = inBytes - unwrapSamples.size();
            }

        }

        if (remainingBytes > 0) {
            /* We reached the end of the buffer queue */
            position = 0;
            queue_index = 0;
            state = SOURCE_STOPPED;
            debuglog(LCF_SOUND | LCF_FRAME, "Source ", id, " plays from buffer ", curBuf->id, " until the end of the queue");
        }
        else {
            /* Update the position in the buffer */
            queue_index = finalIndex;
            position = finalPos;
            debuglog(LCF_SOUND | LCF_FRAME, "Source ", id, " plays from buffer ", curBuf->id, " to some other buffers");
        }

        /* Our unwrap buffer is ready to be mixed */
        MixFromToInternal(&unwrapSamples[0], inBytes, 
                outSamples, outBytes,
                curBuf->bitDepth, curBuf->nbChannels,
                outBitDepth, outNbChannels,
                false, lvas, rvas);
    }
}


AudioContext::AudioContext(void)
{
    outVolume = 1; // Not sure it is default value
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


