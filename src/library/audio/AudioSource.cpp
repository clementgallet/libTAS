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

#include "AudioSource.h"
#include "../logging.h"
#include "../global.h" // shared_config
#include <stdlib.h>
#include <stdint.h>
#include "../DeterministicTimer.h" // detTimer.fakeAdvanceTimer()
#include "AudioConverter.h"
#ifdef __unix__
#include "AudioConverterSwr.h"
#elif defined(__APPLE__) && defined(__MACH__)
#include "AudioConverterCoreAudio.h"
#endif

namespace libtas {

/* Helper function to convert ticks into a number of bytes in the audio buffer */
int AudioSource::ticksToSamples(struct timespec ticks, int frequency)
{
    uint64_t nsecs = static_cast<uint64_t>(ticks.tv_sec) * 1000000000 + ticks.tv_nsec;
    uint64_t samples = (nsecs * frequency) / 1000000000;
    samples_frac += (nsecs * frequency) % 1000000000;
    if (samples_frac >= 500000000) {
        samples_frac -= 1000000000;
        samples++;
    }
    return static_cast<int>(samples);
}

AudioSource::AudioSource(void)
{
#ifdef __unix__
    audioConverter = std::unique_ptr<AudioConverter>(new AudioConverterSwr());
#elif defined(__APPLE__) && defined(__MACH__)
    audioConverter = std::unique_ptr<AudioConverter>(new AudioConverterCoreAudio());
#endif

    init();
}

void AudioSource::init(void)
{
    volume = 1.0f;
    pitch = 1.0f;
    looping = false;
    source = SOURCE_UNDETERMINED;
    state = SOURCE_INITIAL;
    buffer_queue.clear();
    rewind();
}

void AudioSource::rewind(void)
{
    position = 0;
    samples_frac = 0;
    queue_index = 0;
    dirty();
}

void AudioSource::dirty(void)
{
    audioConverter->dirty();
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
        totalSize += buffer->sampleSize;
    }
    return totalSize;
}

int AudioSource::getPosition()
{
    int totalPos = 0;
    for (int i=0; i<queue_index; i++) {
        totalPos += buffer_queue[i]->sampleSize;
    }
    totalPos += position;

    return totalPos;
}

void AudioSource::setPosition(int pos)
{
    if (looping) {
        pos %= queueSize();
    }

    int bi = 0;
    for (const auto& buffer : buffer_queue) {
        if (pos < buffer->sampleSize) {
            /* We set the position in this buffer */
            queue_index = bi;
            position = pos;
            samples_frac = 0;
            return;
        }
        else {
            /* We traverse the buffer */
            pos -= buffer->sampleSize;
        }
        bi++;
    }

    /* We set position to the end of the source */
    if (!buffer_queue.empty()) {
        queue_index = buffer_queue.size() - 1;
        position = buffer_queue[queue_index]->sampleSize;
    }
    else {
        queue_index = 0;
        position = 0;
    }
    samples_frac = 0;
}

bool AudioSource::willEnd(struct timespec ticks)
{
    if (state != SOURCE_PLAYING)
        return false;

    if (buffer_queue.empty())
        return true;

    if (looping)
        return false;

    std::shared_ptr<AudioBuffer> curBuf = buffer_queue[queue_index];

    /* Number of samples to advance in the buffer. */
    int inNbSamples = ticksToSamples(ticks, static_cast<int>(curBuf->frequency*pitch));

    int size = queueSize();
    int pos = getPosition();
    return ((pos + inNbSamples) > size);
}


int AudioSource::mixWith( struct timespec ticks, uint8_t* outSamples, int outBytes, int outBitDepth, int outNbChannels, int outFrequency, float outVolume)
{
    if (state != SOURCE_PLAYING)
        return -1;

    if (buffer_queue.empty())
        return -1;

    debuglogstdio(LCF_SOUND, "Start mixing source %d", id);

    bool skipMixing = (!audioConverter->isAvailable()) || 
                        (!shared_config.av_dumping && 
                            (shared_config.audio_mute ||
                                (shared_config.fastforward && 
                                    (shared_config.fastforward_mode & SharedConfig::FF_MIXING))));

    std::shared_ptr<AudioBuffer> curBuf = buffer_queue[queue_index];

    if (!skipMixing) {
        /* Check if audio converter is initialized.
         * If not, set parameters and init it */
        if (! audioConverter->isInited()) {
            /* Get the sample format */
            AudioBuffer::SampleFormat outFormat = AudioBuffer::SAMPLE_FMT_U8;
            switch (outBitDepth) {
                case 8:
                    outFormat = AudioBuffer::SAMPLE_FMT_U8;
                    break;
                case 16:
                    outFormat = AudioBuffer::SAMPLE_FMT_S16;
                    break;
                default:
                    debuglogstdio(LCF_SOUND | LCF_ERROR, "Unknown audio format");
                    break;
            }

            audioConverter->init(curBuf->format, curBuf->nbChannels, static_cast<int>(curBuf->frequency*pitch), outFormat, outNbChannels, outFrequency);
        }
    }

    /* Mixing source volume and master volume.
     * Taken from openAL doc:
     * "The implementation is free to clamp the total gain (effective gain
     * per-source multiplied by the listener gain) to one to prevent overflow."
     *
     * TODO: This is where we can support panning.
     */
    float resultVolume = (volume * outVolume) > 1.0?1.0:(volume*outVolume);
    int lvas = (int)(resultVolume * 65536.0f);
    int rvas = (int)(resultVolume * 65536.0f);

    /* Number of samples to advance in the buffer. */
    int inNbSamples = ticksToSamples(ticks, static_cast<int>(curBuf->frequency*pitch));

    int oldPosition = position;
    int newPosition = position + inNbSamples;

    uint8_t* begSamples;
    int availableSamples = curBuf->getSamples(begSamples, inNbSamples, oldPosition, (source == SOURCE_STATIC) && looping);

    if (availableSamples == inNbSamples) {
        /* We did not reach the end of the buffer, easy case */

        position = newPosition;
        debuglogstdio(LCF_SOUND, "  Buffer %d in read in range %d - %d", curBuf->id, oldPosition, position);
        if (!skipMixing) {
            audioConverter->queueSamples(begSamples, inNbSamples);
        }
    }
    else {
        /* We reached the end of the buffer */
        debuglogstdio(LCF_SOUND, "  Buffer %d is read from %d to its end %d", curBuf->id, oldPosition, curBuf->sampleSize);
        if (!skipMixing) {
            if (availableSamples > 0)
                audioConverter->queueSamples(begSamples, availableSamples);
        }

        int remainingSamples = inNbSamples - availableSamples;
        if (source == SOURCE_CALLBACK) {
            /* We refill our buffer using the callback function,
             * until we got enough bytes for this frame
             */
            while (remainingSamples > 0) {
                /* Before doing the callback, we must fake that the timer has
                 * advanced by the number of samples already read
                 */
                int64_t extraTicks = static_cast<int64_t>(1000000000) * (-remainingSamples);
                extraTicks /= curBuf->frequency;
                detTimer.fakeAdvanceTimer({static_cast<time_t>(extraTicks / 1000000000), static_cast<long>(extraTicks % 1000000000)});
                callback(*curBuf);
                detTimer.fakeAdvanceTimer({0, 0});
                availableSamples = curBuf->getSamples(begSamples, remainingSamples, 0, false);
                if (!skipMixing) {
                    audioConverter->queueSamples(begSamples, availableSamples);
                }

                debuglogstdio(LCF_SOUND, "  Buffer %d is read again from 0 to %d", curBuf->id, availableSamples);
                if (remainingSamples == availableSamples)
                    position = availableSamples;
                remainingSamples -= availableSamples;
            }
        }
        else {
            int queue_size = buffer_queue.size();
            int finalIndex = queue_index;
            int finalPos = oldPosition + availableSamples;

            /* Our for loop conditions are different if we are looping or not */
            if (looping) {
                for (int i=(queue_index+1)%queue_size; remainingSamples>0; i=(i+1)%queue_size) {
                    std::shared_ptr<AudioBuffer> loopbuf = buffer_queue[i];
                    availableSamples = loopbuf->getSamples(begSamples, remainingSamples, loopbuf->loop_point_beg, (source == SOURCE_STATIC) && looping);
                    debuglogstdio(LCF_SOUND, "  Buffer %d in read in range %d - %d", loopbuf->id, loopbuf->loop_point_beg, availableSamples);

                    if (!skipMixing) {
                        audioConverter->queueSamples(begSamples, availableSamples);
                    }

                    finalIndex = i;
                    finalPos = loopbuf->loop_point_beg + availableSamples;
                    remainingSamples -= availableSamples;
                }
            }
            else {
                for (int i=queue_index+1; (remainingSamples>0) && (i<queue_size); i++) {
                    std::shared_ptr<AudioBuffer> loopbuf = buffer_queue[i];
                    availableSamples = loopbuf->getSamples(begSamples, remainingSamples, 0, false);
                    debuglogstdio(LCF_SOUND, "  Buffer %d in read in range 0 - %d", loopbuf->id, availableSamples);

                    if (!skipMixing) {
                        audioConverter->queueSamples(begSamples, availableSamples);
                    }

                    finalIndex = i;
                    finalPos = availableSamples;
                    remainingSamples -= availableSamples;
                }
            }

            if (remainingSamples > 0) {
                /* We reached the end of the buffer queue */
                debuglogstdio(LCF_SOUND, "  End of the queue reached");
                if (source == SOURCE_STREAMING_CONTINUOUS) {
                    /* Update the position in the buffer */
                    queue_index = finalIndex;
                    position = finalPos;
                    state = SOURCE_UNDERRUN;
                }
                else {
                    queue_index = 0;
                    position = 0;
                    samples_frac = 0;
                    state = SOURCE_STOPPED;
                }
            }
            else {
                /* Update the position in the buffer */
                queue_index = finalIndex;
                position = finalPos;
            }
        }
    }
    
    int convOutSamples = 0;

    if (!skipMixing) {
        /* Allocate the mixed audio array */
        int outNbSamples = outBytes / (outNbChannels * outBitDepth / 8);
        mixedSamples.resize(outBytes);

        /* Get the converter samples */
        convOutSamples = audioConverter->getSamples(mixedSamples.data(), outNbSamples);

        #define clamptofullsignedrange(x,lo,hi) ((static_cast<unsigned int>((x)-(lo))<=static_cast<unsigned int>((hi)-(lo)))?(x):(((x)<0)?(lo):(hi)))

        int nbSaturate = 0;

        /* Add mixed source to the output buffer */
        if (outBitDepth == 8) {
            for (int s=0; s<convOutSamples*outNbChannels; s+=outNbChannels) {
                int myL = mixedSamples[s];
                int otherL = outSamples[s];
                int sumL = otherL + ((myL * lvas) >> 16) - 256;
                outSamples[s] = clamptofullsignedrange(sumL, 0, UINT8_MAX);
                nbSaturate += (sumL < 0) || (sumL > UINT8_MAX);

                if (outNbChannels == 2) {
                    int myR = mixedSamples[s+1];
                    int otherR = outSamples[s+1];
                    int sumR = otherR + ((myR * rvas) >> 16) - 256;
                    outSamples[s+1] = clamptofullsignedrange(sumR, 0, UINT8_MAX);
                    nbSaturate += (sumR < 0) || (sumR > UINT8_MAX);
                }
            }
        }

        if (outBitDepth == 16) {
            int16_t* mixedSamples16 = reinterpret_cast<int16_t*>(mixedSamples.data());
            int16_t* outSamples16 = reinterpret_cast<int16_t*>(outSamples);
            for (int s=0; s<convOutSamples*outNbChannels; s+=outNbChannels) {
                int myL = mixedSamples16[s];
                int otherL = outSamples16[s];
                int sumL = otherL + ((myL * lvas) >> 16);
                outSamples16[s] = clamptofullsignedrange(sumL, INT16_MIN, INT16_MAX);
                nbSaturate += (sumL < INT16_MIN) || (sumL > INT16_MAX);

                if (outNbChannels == 2) {
                    int myR = mixedSamples16[s+1];
                    int otherR = outSamples16[s+1];
                    int sumR = otherR + ((myR * rvas) >> 16);
                    outSamples16[s+1] = clamptofullsignedrange(sumR, INT16_MIN, INT16_MAX);
                    nbSaturate += (sumR < INT16_MIN) || (sumR > INT16_MAX);
                }
            }
        }
        
        if (nbSaturate > 0)
            debuglogstdio(LCF_SOUND | LCF_WARNING, "Saturation during mixing for %d samples", nbSaturate);        
    }

    /* Reset the audio converter if the source has stopped */
    if (state == SOURCE_STOPPED)
        dirty();

    return convOutSamples;
}

}
