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

#define MAXBUFFERS 256

/* Helper function to convert ticks into a number of bytes in the audio buffer */
static int ticksToBytes(struct timespec ticks, int bitDepth, int nbChannels, int frequency)
{
    uint64_t nsecs = ((uint64_t) ticks.tv_sec) * 1000000000 + ticks.tv_nsec;
    uint64_t bytes = (nsecs * (bitDepth / 8) * nbChannels * frequency) / 1000000000;
    return (int) bytes;
}

AudioBufferList bufferList;

AudioBuffer::AudioBuffer(void)
{
    id = 0;
    bitDepth = 16;
    nbChannels = 1;
    frequency = 0;
    size = 0;
    position = 0;
    volume = 1.0f; // Default from openal-soft
    source = SOURCE_UNDETERMINED;
    looping = false;
    state = SOURCE_INITIAL;
    nextBuffer = nullptr;
    processed = false;
}

int AudioBuffer::nbQueue()
{
    int nbQueued = 0;
    AudioBuffer* nab = this;
    while (nab->nextBuffer != nullptr) {
        nbQueued++;
        nab = nab->nextBuffer;
    }
    return nbQueued;
}

int AudioBuffer::nbQueueProcessed()
{
    int nbProcessed = 0;
    AudioBuffer* nab = this;
    while (nab->nextBuffer != nullptr) {
        nab = nab->nextBuffer;
        if (nab->processed)
            nbProcessed++;
    }
    return nbProcessed;
}

int AudioBuffer::queueSize()
{
    int totalSize = 0;
    AudioBuffer* nab = this;
    while (nab->nextBuffer != nullptr) {
        nab = nab->nextBuffer;
        totalSize += nab->size;
    }
    return totalSize;
}

int AudioBuffer::getPosition()
{
    int totalPos = 0;
    AudioBuffer* nab = this;
    while (nab->nextBuffer != nullptr) {
        nab = nab->nextBuffer;

        if (nab->processed)
            totalPos += nab->size;
        else {
            totalPos += nab->position;
            break;
        }
    }

    return totalPos;
}

void AudioBuffer::setPosition(int pos)
{
    /* We first store the state of the queue so that we can set it later */
    SourceState curState = this->getState();

    int localPos = pos;
    AudioBuffer* nab = this;
    while (nab->nextBuffer != nullptr) {
        nab = nab->nextBuffer;

        if (nab->looping) {
            /* Position rollback to zero for looping buffers */
            localPos = localPos % nab->size;
        }

        if (localPos < nab->size) {
            /* We set the position in this buffer */

            /* Fix unaligned position */
            localPos -= localPos % (nab->nbChannels * nab->bitDepth / 8);
            nab->position = localPos;
            nab->state = curState;
            nab->processed = false;
            break;
        }
        else {
            /* We traverse the buffer */
            nab->position = 0;
            nab->state = SOURCE_STOPPED;
            nab->processed = true;
            localPos -= nab->size;
        }
    }

    /* Now we set the rest of the queue to inital and non-processed state */
    while (nab->nextBuffer != nullptr) {
        nab = nab->nextBuffer;
        nab->position = 0;
        nab->state = SOURCE_INITIAL;
        nab->processed = false;
    }
}

void AudioBuffer::changeState(SourceState newstate)
{
    AudioBuffer* nab = this;
    while (nab != nullptr) {
        if (! nab->processed) {
            nab->state = newstate;
            return;
        }
        nab = nab->nextBuffer;
    }
}

SourceState AudioBuffer::getState()
{
    AudioBuffer* nab = this;
    while (nab != nullptr) {
        if (! nab->processed) {
            return nab->state;
        }
        nab = nab->nextBuffer;
    }
    return SOURCE_STOPPED;
}

void AudioBuffer::mixWith( struct timespec ticks, uint8_t* outSamples, int outBytes, int outBitDepth, int outNbChannels, int outFrequency, float outVolume)
{
    if (processed)
        return;

    if (state != SOURCE_PLAYING)
        return;

    if (size == 0)
        return;


    /* Mixing buffer volume and master volume.
	 * Taken from openAL doc:
     * "The implementation is free to clamp the total gain (effective gain
     * per-source multiplied by the listener gain) to one to prevent overflow."
     *
     * TODO: This is where we can support panning.
     */
    float resultVolume = std::max(1.0f, volume * outVolume);
    int lvas = (int)(resultVolume * 65536);
    int rvas = (int)(resultVolume * 65536);

    /* Number of bytes to advance in the buffer samples */
    int inBytes = ticksToBytes(ticks, bitDepth, nbChannels, frequency);

    int oldPosition = position;
    int newPosition = position + inBytes;

    if (newPosition < size) {
        /* We did not reach the end of the buffer, easy case */

        position = newPosition;
        MixFromToInternal(&samples[oldPosition], inBytes,
                outSamples, outBytes,
                bitDepth, nbChannels,
                outBitDepth, outNbChannels,
                false, lvas, rvas);
    }
    else {
        /* We reached the end of the buffer. We have several scenarios here: */

        if (looping) {
            /*
             * Let's take the dumb solution when looping:
             * just replicate the buffer as many times as needed in a new buffer
             */
            /* FIXME: Apparently looping is done on the entire queue, not the current buffer!!! */
            /* Use the vector constructor for the first copy */
            /* TODO: Keep the vector allocated somewhere? */
            std::vector<uint8_t> unwrapSamples (&samples[oldPosition], &samples[size]);
            unwrapSamples.reserve(inBytes);

            int remainingBytes = inBytes - unwrapSamples.size();
            while (remainingBytes > 0) {
                if (remainingBytes > size)
                    std::copy(&samples[0], &samples[size], std::back_inserter(unwrapSamples));
                else
                    std::copy(&samples[0], &samples[remainingBytes], std::back_inserter(unwrapSamples));
                remainingBytes = inBytes - unwrapSamples.size();
            }

            /* Update the position in the buffer */
            position = newPosition % size;

            /* Our unwrap buffer is ready to be mixed */
            MixFromToInternal(&unwrapSamples[0], inBytes, 
                    outSamples, outBytes,
                    bitDepth, nbChannels,
                    outBitDepth, outNbChannels,
                    false, lvas, rvas);
        }
        else {
            /* We did not loop, so this buffer is read entirely */
            position = 0;
            state = SOURCE_STOPPED;
            processed = true;

            if (nextBuffer == nullptr) {
                /* There are no queued buffers, so we are pretty much done. */

                /* Mix the buffer until the end */
                MixFromToInternal(&samples[oldPosition], size,
                        outSamples, outBytes,
                        bitDepth, nbChannels,
                        outBitDepth, outNbChannels,
                        true, lvas, rvas);
            }
            else {
                /* We reached the end of a buffer and need to read from the 
                 * next buffer in the queue.
                 * Again, we take the (bad?) solution of building an unwrap buffer.
                 * Alternate solutions: call MixFromToInternal multiple times
                 * or call this function recursively (basically the same).
                 * Or use libavresample for mixing...
                 */
                std::vector<uint8_t> unwrapSamples (&samples[oldPosition], &samples[size]);
                unwrapSamples.reserve(inBytes);

                int remainingBytes = inBytes - unwrapSamples.size();
                bool remaining = true;

                AudioBuffer* ab_queue = nextBuffer;

                /* Crawling the AudioBuffer queue */
                while (ab_queue != nullptr) {

                    if (ab_queue->size > remainingBytes) {
                        /* We play part of this queue buffer */
                        ab_queue->state = SOURCE_PLAYED;
                        ab_queue->position = remainingBytes;
                        std::copy(&ab_queue->samples[0], &ab_queue->samples[remaining], std::back_inserter(unwrapSamples));
                        break;
                    }
                    else {
                        /* This buffer is also read entirely */
                        ab_queue->state = SOURCE_STOPPED;
                        ab_queue->position = 0;
                        ab_queue->processed = true;
                        std::copy(&ab_queue->samples[0], &ab_queue->samples[ab_queue->size], std::back_inserter(unwrapSamples));
                    }
                    remainingBytes = inBytes - unwrapSamples.size();
                    ab_queue = ab_queue->nextBuffer;
                }

                /* Now we mix with our unwrap buffer */
                MixFromToInternal(&unwrapSamples[0], unwrapSamples.size(),
                        outSamples, outBytes,
                        bitDepth, nbChannels,
                        outBitDepth, outNbChannels,
                        ab_queue == nullptr, lvas, rvas);
            }
        }
    }
}


AudioBufferList::AudioBufferList(void)
{
    outVolume = 1; // Not sure it is default value
    outBitDepth = 16;
    outNbChannels = 2;
    outFrequency = 44100;
    
    /* TEMP! WAV out */
    file = SndfileHandle("test.wav", SFM_WRITE, SF_FORMAT_WAV | SF_FORMAT_PCM_16, outNbChannels, outFrequency);
}

int AudioBufferList::createBuffer(AudioBuffer** abp)
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

    /* Optionally return the pointer to the buffer as well */
    if (abp != nullptr)
        *abp = newab;

    return newab->id;
}

void AudioBufferList::deleteBuffer(int id)
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

bool AudioBufferList::isBuffer(int id)
{
    for (auto& buffer : buffers) {
        if (buffer->id == id)
            return true;
    }

    return false;
}

AudioBuffer* AudioBufferList::getBuffer(int id)
{
    for (auto& buffer : buffers) {
        if (buffer->id == id)
            return buffer;
    }

    return nullptr;
}

void AudioBufferList::mixAllBuffers(struct timespec ticks)
{
    int outBytes = ticksToBytes(ticks, outBitDepth, outNbChannels, outFrequency);

    /* Silent the output buffer */
    if (outBitDepth == 8) // Unsigned 8-bit samples
        outSamples.assign(outBytes, 0x80);
    if (outBitDepth == 16) // Signed 16-bit samples
        outSamples.assign(outBytes, 0);

    for (auto& buffer : buffers) {
        buffer->mixWith(ticks, &outSamples[0], outBytes, outBitDepth, outNbChannels, outFrequency, outVolume);
    }

    /* We change all SOURCE_PLAYED to SOURCE_PLAYING */
    for (auto& buffer : buffers) {
        if (buffer->state == SOURCE_PLAYED)
            buffer->state = SOURCE_PLAYING;
    }

    /* TEMP! WAV output */
    if (outBitDepth == 16)
        file.write((int16_t*)&outSamples[0], outBytes/2);
}


