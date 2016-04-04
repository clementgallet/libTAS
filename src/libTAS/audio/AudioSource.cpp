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

#include "AudioSource.h"
#include <iterator>     // std::back_inserter
#include <algorithm>    // std::copy
#include "../logging.h"
#if defined(LIBTAS_ENABLE_AVDUMPING) || defined(LIBTAS_ENABLE_SOUNDPLAYBACK)
extern "C" {
    #include <libavutil/opt.h>
    #include <libavutil/channel_layout.h>
}
#endif
#include <stdlib.h>

/* Helper function to convert ticks into a number of bytes in the audio buffer */
int AudioSource::ticksToBytes(struct timespec ticks, int alignSize, int frequency)
{
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

AudioSource::AudioSource(void)
{
    id = 0;
    position = 0;
    samples_frac = 0;
    volume = 1.0f;
    source = SOURCE_UNDETERMINED;
    looping = false;
    state = SOURCE_INITIAL;
    queue_index = 0;

#if defined(LIBTAS_ENABLE_AVDUMPING) || defined(LIBTAS_ENABLE_SOUNDPLAYBACK)
    swr = swr_alloc();
#endif
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
            localPos -= localPos % ab->alignSize;
            position = localPos;
            samples_frac = 0;
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

int AudioSource::mixWith( struct timespec ticks, uint8_t* outSamples, int outBytes, int outBitDepth, int outNbChannels, int outFrequency, float outVolume)
{
    if (state != SOURCE_PLAYING)
        return -1;

    if (buffer_queue.empty())
        return -1;

    debuglog(LCF_SOUND | LCF_FRAME, "Start mixing source ", id);

    AudioBuffer* curBuf = buffer_queue[queue_index];

#if defined(LIBTAS_ENABLE_AVDUMPING) || defined(LIBTAS_ENABLE_SOUNDPLAYBACK)
    /* Get the sample format */
    AVSampleFormat inFormat, outFormat;
    if (curBuf->bitDepth == 8)
        inFormat = AV_SAMPLE_FMT_U8;
    if (curBuf->bitDepth == 16)
        inFormat = AV_SAMPLE_FMT_S16;
    if (outBitDepth == 8)
        outFormat = AV_SAMPLE_FMT_U8;
    if (outBitDepth == 16)
        outFormat = AV_SAMPLE_FMT_S16;

    /* Check if SWR context is initialized.
     * If not, set parameters and init it
     */
    if (! swr_is_initialized(swr)) {
        /* Set channel layout */
        if (curBuf->nbChannels == 1)
            av_opt_set_int(swr, "in_channel_layout", AV_CH_LAYOUT_MONO, 0);
        if (curBuf->nbChannels == 2)
            av_opt_set_int(swr, "in_channel_layout", AV_CH_LAYOUT_STEREO, 0);
        if (outNbChannels == 1)
            av_opt_set_int(swr, "out_channel_layout", AV_CH_LAYOUT_MONO, 0);
        if (outNbChannels == 2)
            av_opt_set_int(swr, "out_channel_layout", AV_CH_LAYOUT_STEREO, 0);

        /* Set sample format */
        av_opt_set_sample_fmt(swr, "in_sample_fmt", inFormat, 0);
        av_opt_set_sample_fmt(swr, "out_sample_fmt", outFormat, 0);

        /* Set sampling frequency */
        av_opt_set_int(swr, "in_sample_rate", curBuf->frequency, 0);
        av_opt_set_int(swr, "out_sample_rate", outFrequency, 0);

        /* Open the context */
        if (swr_init(swr) < 0) {
            debuglog(LCF_SOUND | LCF_FRAME | LCF_ERROR, "Error initializing swr context");
            return 0;
        }
    }
#endif

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

    /* Number of bytes to advance in the buffer samples.
     * This number is automatically aligned inside the function.
     */
    int inBytes = ticksToBytes(ticks, curBuf->alignSize, curBuf->frequency);

    int oldPosition = position;
    int newPosition = position + inBytes;

    /* Allocate the mixed audio array */
    int inNSamples = inBytes / curBuf->alignSize;
    int outAlignSize = (outNbChannels * outBitDepth / 8);
    int outNbSamples = outBytes / outAlignSize;
#if defined(LIBTAS_ENABLE_AVDUMPING) || defined(LIBTAS_ENABLE_SOUNDPLAYBACK)
    mixedSamples.resize(outBytes);
    uint8_t* begMixed = &mixedSamples[0];
#endif

    int convOutSamples = 0;
    uint8_t* begSamples = &curBuf->samples[oldPosition];
    if (newPosition <= curBuf->size) {
        /* We did not reach the end of the buffer, easy case */

        position = newPosition;
        debuglog(LCF_SOUND | LCF_FRAME, "Source ", id, " plays buffer ", curBuf->id, " in range ", oldPosition, " - ", position);
#if defined(LIBTAS_ENABLE_AVDUMPING) || defined(LIBTAS_ENABLE_SOUNDPLAYBACK)
        convOutSamples = swr_convert(swr, &begMixed, outNbSamples, (const uint8_t**)&begSamples, inNSamples);
#endif
    }
    else {
        /* We reached the end of the buffer */
        debuglog(LCF_SOUND | LCF_FRAME, "Buffer ", curBuf->id, " is read from ", oldPosition, " to its end ", curBuf->size);
#if defined(LIBTAS_ENABLE_AVDUMPING) || defined(LIBTAS_ENABLE_SOUNDPLAYBACK)
        int nSamples = (curBuf->size - oldPosition) / curBuf->alignSize;
        if (nSamples > 0)
            swr_convert(swr, nullptr, 0, (const uint8_t**)&begSamples, nSamples);
#endif

        int remainingBytes = inBytes - (curBuf->size - oldPosition);
        if (source == SOURCE_CALLBACK) {
            /* We refill our buffer using the callback function,
             * until we got enough bytes for this frame
             */
            while (remainingBytes > 0) {
                callback(curBuf);
                begSamples = &curBuf->samples[0];
                if (remainingBytes > curBuf->size) {
#if defined(LIBTAS_ENABLE_AVDUMPING) || defined(LIBTAS_ENABLE_SOUNDPLAYBACK)
                    nSamples = curBuf->size / curBuf->alignSize;
                    swr_convert(swr, nullptr, 0, (const uint8_t**)&begSamples, nSamples);
#endif
                    remainingBytes -= curBuf->size;
                }
                else {
#if defined(LIBTAS_ENABLE_AVDUMPING) || defined(LIBTAS_ENABLE_SOUNDPLAYBACK)
                    nSamples = remainingBytes / curBuf->alignSize;
                    swr_convert(swr, nullptr, 0, (const uint8_t**)&begSamples, nSamples);
#endif
                    position = remainingBytes;
                    remainingBytes = 0;
                }
            }

#if defined(LIBTAS_ENABLE_AVDUMPING) || defined(LIBTAS_ENABLE_SOUNDPLAYBACK)
            /* Get the mixed samples */
            convOutSamples = swr_convert(swr, &begMixed, outNbSamples, nullptr, 0);
#endif
        }
        else {
            int queue_size = buffer_queue.size();
            int finalIndex;
            int finalPos;

            /* Our for loop conditions are different if we are looping or not */
            if (looping) {
                for (int i=(queue_index+1)%queue_size; remainingBytes>0; i=(i+1)%queue_size) {
                    AudioBuffer* loopbuf = buffer_queue[i];
                    begSamples = &loopbuf->samples[0];
                    if (remainingBytes > loopbuf->size) {
#if defined(LIBTAS_ENABLE_AVDUMPING) || defined(LIBTAS_ENABLE_SOUNDPLAYBACK)
                        nSamples = loopbuf->size / loopbuf->alignSize;
                        swr_convert(swr, nullptr, 0, (const uint8_t**)&begSamples, nSamples);
#endif
                        loopbuf->processed = true; // Are buffers in a loop ever processed??
                        remainingBytes -= loopbuf->size;
                    }
                    else {
#if defined(LIBTAS_ENABLE_AVDUMPING) || defined(LIBTAS_ENABLE_SOUNDPLAYBACK)
                        nSamples = remainingBytes / loopbuf->alignSize;
                        swr_convert(swr, nullptr, 0, (const uint8_t**)&begSamples, nSamples);
#endif
                        finalIndex = i;
                        finalPos = remainingBytes;
                        remainingBytes = 0;
                    }
                }
            }
            else {
                for (int i=queue_index+1; (remainingBytes>0) && (i<queue_size); i++) {
                    AudioBuffer* loopbuf = buffer_queue[i];
                    begSamples = &loopbuf->samples[0];
                    if (remainingBytes > loopbuf->size) {
#if defined(LIBTAS_ENABLE_AVDUMPING) || defined(LIBTAS_ENABLE_SOUNDPLAYBACK)
                        nSamples = loopbuf->size / loopbuf->alignSize;
                        swr_convert(swr, nullptr, 0, (const uint8_t**)&begSamples, nSamples);
#endif
                        loopbuf->processed = true;
                        remainingBytes -= loopbuf->size;
                    }
                    else {
#if defined(LIBTAS_ENABLE_AVDUMPING) || defined(LIBTAS_ENABLE_SOUNDPLAYBACK)
                        nSamples = remainingBytes / loopbuf->alignSize;
                        swr_convert(swr, nullptr, 0, (const uint8_t**)&begSamples, nSamples);
#endif
                        finalIndex = i;
                        finalPos = remainingBytes;
                        remainingBytes = 0;
                    }
                }
            }

#if defined(LIBTAS_ENABLE_AVDUMPING) || defined(LIBTAS_ENABLE_SOUNDPLAYBACK)
            /* Get the mixed samples */
            convOutSamples = swr_convert(swr, &begMixed, outNbSamples, nullptr, 0);
#endif

            if (remainingBytes > 0) {
                /* We reached the end of the buffer queue */
                position = 0;
                samples_frac = 0;
                queue_index = 0;
                state = SOURCE_STOPPED;
#if defined(LIBTAS_ENABLE_AVDUMPING) || defined(LIBTAS_ENABLE_SOUNDPLAYBACK)
                swr_close(swr);
#endif
                debuglog(LCF_SOUND | LCF_FRAME, "Source ", id, " plays from buffer ", curBuf->id, " until the end of the queue");
            }
            else {
                /* Update the position in the buffer */
                queue_index = finalIndex;
                position = finalPos;
                debuglog(LCF_SOUND | LCF_FRAME, "Source ", id, " plays from buffer ", curBuf->id, " to some other buffers");
            }
        }

    }

#if defined(LIBTAS_ENABLE_AVDUMPING) || defined(LIBTAS_ENABLE_SOUNDPLAYBACK)

#define clamptofullsignedrange(x,lo,hi) (((unsigned int)((x)-(lo))<=(unsigned int)((hi)-(lo)))?(x):(((x)<0)?(lo):(hi)))

    /* Add mixed source to the output buffer */
    if (outBitDepth == 8) {
        for (int s=0; s<convOutSamples*outNbChannels; s+=outNbChannels) {
            int myL = ((uint8_t*)&mixedSamples[0])[s];
            int otherL = ((uint8_t*)outSamples)[s];
            int sumL = otherL + ((myL * lvas) >> 16) - 256;
            ((uint8_t*)outSamples)[s] = clamptofullsignedrange(sumL, 0, (1<<8)-1);

            if (outNbChannels == 2) {
                int myR = ((uint8_t*)&mixedSamples[0])[s+1];
                int otherR = ((uint8_t*)outSamples)[s+1];
                int sumR = otherR + ((myR * rvas) >> 16);
                ((uint8_t*)outSamples)[s+1] = clamptofullsignedrange(sumR, 0, (1<<8)-1);
            }
        }
    }

    if (outBitDepth == 16) {
        for (int s=0; s<convOutSamples*outNbChannels; s+=outNbChannels) {
            int myL = ((int16_t*)&mixedSamples[0])[s];
            int otherL = ((int16_t*)outSamples)[s];
            int sumL = otherL + ((myL * lvas) >> 16);
            ((int16_t*)outSamples)[s] = clamptofullsignedrange(sumL, -(1<<15), (1<<15)-1);

            if (outNbChannels == 2) {
                int myR = ((int16_t*)&mixedSamples[0])[s+1];
                int otherR = ((int16_t*)outSamples)[s+1];
                int sumR = otherR + ((myR * rvas) >> 16);
                ((int16_t*)outSamples)[s+1] = clamptofullsignedrange(sumR, -(1<<15), (1<<15)-1);
            }
        }
    }
#endif

    return convOutSamples;
}

