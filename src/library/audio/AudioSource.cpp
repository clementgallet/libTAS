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
#include <iterator>     // std::back_inserter
#include <algorithm>    // std::copy
#include "../logging.h"
#include "../global.h" // shared_config
#include "hook.h"
#include <stdlib.h>
#include "../DeterministicTimer.h" // detTimer.fakeAdvanceTimer()

namespace libtas {

/* Link dynamically to avutil/swresample functions, because there are different
 * library versions depending on your distro.
 */
DEFINE_ORIG_POINTER(swr_alloc);
DEFINE_ORIG_POINTER(swr_free);
DEFINE_ORIG_POINTER(swr_is_initialized);
DEFINE_ORIG_POINTER(swr_close);
DEFINE_ORIG_POINTER(swr_init);
DEFINE_ORIG_POINTER(swr_alloc_set_opts);
DEFINE_ORIG_POINTER(swr_convert);

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
    /* Some systems don't create the unversionned symlinks when the libraries
     * are installed, so we add a link with the major version. */
    LINK_NAMESPACE(swr_alloc, "swresample");
    link_function((void**)&orig::swr_alloc, "swr_alloc", "libswresample.so.3");
    link_function((void**)&orig::swr_alloc, "swr_alloc", "libswresample.so.2");

    /* We link to swr_free here, because linking during destructor can softlock */
    LINK_NAMESPACE(swr_free, "swresample");

    swr = orig::swr_alloc();

    volume = 1.0f;
    pitch = 1.0f;
    init();
}

AudioSource::~AudioSource(void)
{
    orig::swr_free(&swr);
}

void AudioSource::init(void)
{
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
    LINK_NAMESPACE(swr_is_initialized, "swresample");
    LINK_NAMESPACE(swr_close, "swresample");
    if (orig::swr_is_initialized(swr))
        orig::swr_close(swr);
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
    LINK_NAMESPACE(swr_is_initialized, "swresample");
    LINK_NAMESPACE(swr_init, "swresample");
    LINK_NAMESPACE(swr_convert, "swresample");
    LINK_NAMESPACE(swr_alloc_set_opts, "swresample");

    if (state != SOURCE_PLAYING)
        return -1;

    if (buffer_queue.empty())
        return -1;

    debuglog(LCF_SOUND, "Start mixing source ", id);

    bool skipMixing = !shared_config.av_dumping && (shared_config.audio_mute ||
        (shared_config.fastforward && (shared_config.fastforward_mode & SharedConfig::FF_MIXING)));

    std::shared_ptr<AudioBuffer> curBuf = buffer_queue[queue_index];

    if (!skipMixing) {
        /* Check if SWR context is initialized.
         * If not, set parameters and init it
         */
        if (! orig::swr_is_initialized(swr)) {
            /* Get the sample format */
            AVSampleFormat inFormat = AV_SAMPLE_FMT_U8;
            AVSampleFormat outFormat = AV_SAMPLE_FMT_U8;
            switch (curBuf->format) {
                case AudioBuffer::SAMPLE_FMT_U8:
                    inFormat = AV_SAMPLE_FMT_U8;
                    break;
                case AudioBuffer::SAMPLE_FMT_S16:
                case AudioBuffer::SAMPLE_FMT_MSADPCM:
                    inFormat = AV_SAMPLE_FMT_S16;
                    break;
                case AudioBuffer::SAMPLE_FMT_S32:
                    inFormat = AV_SAMPLE_FMT_S32;
                    break;
                case AudioBuffer::SAMPLE_FMT_FLT:
                    inFormat = AV_SAMPLE_FMT_FLT;
                    break;
                case AudioBuffer::SAMPLE_FMT_DBL:
                    inFormat = AV_SAMPLE_FMT_DBL;
                    break;
                default:
                    debuglog(LCF_SOUND | LCF_ERROR, "Unknown sample format");
                    break;
            }
            if (outBitDepth == 8)
                outFormat = AV_SAMPLE_FMT_U8;
            if (outBitDepth == 16)
                outFormat = AV_SAMPLE_FMT_S16;

            /* Get the channel layout */
            int64_t in_ch_layout = 0;
            int64_t out_ch_layout = 0;

            if (curBuf->nbChannels == 1) {
                in_ch_layout = AV_CH_LAYOUT_MONO;
            }
            if (curBuf->nbChannels == 2) {
                in_ch_layout = AV_CH_LAYOUT_STEREO;
            }
            if (outNbChannels == 1) {
                out_ch_layout = AV_CH_LAYOUT_MONO;
            }
            if (outNbChannels == 2) {
                out_ch_layout = AV_CH_LAYOUT_STEREO;
            }

            MYASSERT(nullptr != orig::swr_alloc_set_opts(swr, out_ch_layout, outFormat, outFrequency, in_ch_layout, inFormat, static_cast<int>(curBuf->frequency*pitch), 0, nullptr));

            /* Open the context */
            if (orig::swr_init(swr) < 0) {
                debuglog(LCF_SOUND | LCF_ERROR, "Error initializing swr context");
                return 0;
            }
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

    /* Allocate the mixed audio array */
    int outNbSamples = outBytes / (outNbChannels * outBitDepth / 8);
    mixedSamples.resize(outBytes);
    uint8_t* begMixed = mixedSamples.data();

    int convOutSamples = 0;
    uint8_t* begSamples;
    int availableSamples = curBuf->getSamples(begSamples, inNbSamples, oldPosition, (source == SOURCE_STATIC) && looping);

    if (availableSamples == inNbSamples) {
        /* We did not reach the end of the buffer, easy case */

        position = newPosition;
        debuglog(LCF_SOUND, "  Buffer ", curBuf->id, " in read in range ", oldPosition, " - ", position);
        if (!skipMixing) {
            convOutSamples = orig::swr_convert(swr, &begMixed, outNbSamples, const_cast<const uint8_t**>(&begSamples), inNbSamples);
        }
    }
    else {
        /* We reached the end of the buffer */
        debuglog(LCF_SOUND, "  Buffer ", curBuf->id, " is read from ", oldPosition, " to its end ", curBuf->sampleSize);
        if (!skipMixing) {
            if (availableSamples > 0)
                orig::swr_convert(swr, nullptr, 0, const_cast<const uint8_t**>(&begSamples), availableSamples);
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
                    orig::swr_convert(swr, nullptr, 0, const_cast<const uint8_t**>(&begSamples), availableSamples);
                }

                debuglog(LCF_SOUND, "  Buffer ", curBuf->id, " is read again from 0 to ", availableSamples);
                if (remainingSamples == availableSamples)
                    position = availableSamples;
                remainingSamples -= availableSamples;
            }

            if (!skipMixing) {
                /* Get the mixed samples */
                convOutSamples = orig::swr_convert(swr, &begMixed, outNbSamples, nullptr, 0);
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
                    debuglog(LCF_SOUND, "  Buffer ", loopbuf->id, " in read in range ", loopbuf->loop_point_beg, " - ", availableSamples);

                    if (!skipMixing) {
                        orig::swr_convert(swr, nullptr, 0, const_cast<const uint8_t**>(&begSamples), availableSamples);
                    }

                    if (remainingSamples == availableSamples) {
                        finalIndex = i;
                        finalPos = loopbuf->loop_point_beg + availableSamples;
                    }
                    remainingSamples -= availableSamples;
                }
            }
            else {
                for (int i=queue_index+1; (remainingSamples>0) && (i<queue_size); i++) {
                    std::shared_ptr<AudioBuffer> loopbuf = buffer_queue[i];
                    availableSamples = loopbuf->getSamples(begSamples, remainingSamples, 0, false);
                    debuglog(LCF_SOUND, "  Buffer ", loopbuf->id, " in read in range 0 - ", availableSamples);

                    if (!skipMixing) {
                        orig::swr_convert(swr, nullptr, 0, const_cast<const uint8_t**>(&begSamples), availableSamples);
                    }

                    if (remainingSamples == availableSamples) {
                        finalIndex = i;
                        finalPos = availableSamples;
                    }
                    remainingSamples -= availableSamples;
                }
            }

            if (!skipMixing) {
                /* Get the mixed samples */
                convOutSamples = orig::swr_convert(swr, &begMixed, outNbSamples, nullptr, 0);
            }

            if ((remainingSamples > 0) && (source != SOURCE_STREAMING_CONTINUOUS)) {
                /* We reached the end of the buffer queue */
                rewind();
                state = SOURCE_STOPPED;
                debuglog(LCF_SOUND, "  End of the queue reached");
            }
            else {
                /* Update the position in the buffer */
                queue_index = finalIndex;
                position = finalPos;
            }
        }

    }

    if (!skipMixing) {
        #define clamptofullsignedrange(x,lo,hi) ((static_cast<unsigned int>((x)-(lo))<=static_cast<unsigned int>((hi)-(lo)))?(x):(((x)<0)?(lo):(hi)))

        /* Add mixed source to the output buffer */
        if (outBitDepth == 8) {
            for (int s=0; s<convOutSamples*outNbChannels; s+=outNbChannels) {
                int myL = mixedSamples[s];
                int otherL = outSamples[s];
                int sumL = otherL + ((myL * lvas) >> 16) - 256;
                outSamples[s] = clamptofullsignedrange(sumL, 0, UINT8_MAX);
                if ((sumL < 0) || (sumL > UINT8_MAX))
                    debuglog(LCF_SOUND | LCF_WARNING, "Saturation during mixing");

                if (outNbChannels == 2) {
                    int myR = mixedSamples[s+1];
                    int otherR = outSamples[s+1];
                    int sumR = otherR + ((myR * rvas) >> 16) - 256;
                    outSamples[s+1] = clamptofullsignedrange(sumR, 0, UINT8_MAX);
                    if ((sumR < 0) || (sumR > UINT8_MAX))
                        debuglog(LCF_SOUND | LCF_WARNING, "Saturation during mixing");
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
                if ((sumL < INT16_MIN) || (sumL > INT16_MAX))
                    debuglog(LCF_SOUND | LCF_WARNING, "Saturation during mixing");

                if (outNbChannels == 2) {
                    int myR = mixedSamples16[s+1];
                    int otherR = outSamples16[s+1];
                    int sumR = otherR + ((myR * rvas) >> 16);
                    outSamples16[s+1] = clamptofullsignedrange(sumR, INT16_MIN, INT16_MAX);
                    if ((sumR < INT16_MIN) || (sumR > INT16_MAX))
                        debuglog(LCF_SOUND | LCF_WARNING, "Saturation during mixing");
                }
            }
        }
    }

    return convOutSamples;
}

}
