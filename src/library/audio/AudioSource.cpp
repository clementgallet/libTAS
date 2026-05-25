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

#include "AudioSource.h"
#include "AudioConverter.h"
#include "AudioBuffer.h"
#ifdef __unix__
#include "AudioConverterSwr.h"
#elif defined(__APPLE__) && defined(__MACH__)
#include "AudioConverterCoreAudio.h"
#endif

#include "logging.h"
#include "global.h" // Global::shared_config
#include "DeterministicTimer.h" // detTimer.fakeAdvanceTimer()

#include <stdlib.h>
#include <stdint.h>

namespace libtas {

/* Helper function to convert ticks into a number of bytes in the audio buffer */
int AudioSource::ticksToSamples(struct timespec ticks, int frequency)
{
    if (frequency <= 0)
        return 0;

    uint64_t nsecs = static_cast<uint64_t>(ticks.tv_sec) * 1000000000ULL + ticks.tv_nsec;
    uint64_t samples = (nsecs * frequency) / 1000000000ULL;
    samples_frac += (nsecs * frequency) % 1000000000ULL;
    if (samples_frac >= 500000000LL) {
        samples_frac -= 1000000000LL;
        samples++;
    }
    return static_cast<int>(samples);
}

AudioSource::AudioSource(void)
{
#ifdef __unix__
    audio_converter = std::unique_ptr<AudioConverter>(new AudioConverterSwr());
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
    callback = nullptr;
    callback_data = nullptr;
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
    audio_converter->dirty();
}

int AudioSource::frameToByteRatio()
{
    return channels * AudioBuffer::formatToBitDepth(format) / 8;
}

int AudioSource::nbQueue() const
{
    return buffer_queue.size();
}

int AudioSource::nbQueueProcessed() const
{
    return queue_index;
}

const std::shared_ptr<AudioBuffer> AudioSource::buffer(int index) const
{
    if (index < 0 || index >= static_cast<int>(buffer_queue.size()))
        return nullptr;
    return buffer_queue[index];
}

int AudioSource::queueSize() const
{
    int totalSize = 0;
    for (auto& buffer : buffer_queue) {
        totalSize += buffer->sampleSize;
    }
    return totalSize;
}

int AudioSource::getPosition() const
{
    int totalPos = 0;
    int processed = std::min(queue_index, static_cast<int>(buffer_queue.size()));
    for (int i=0; i<processed; i++) {
        totalPos += buffer_queue[i]->sampleSize;
    }
    totalPos += position;

    return totalPos;
}

void AudioSource::setPosition(int pos)
{
    if (looping) {
        int s = queueSize();
        if (s == 0)
            pos = 0;
        else
            pos %= s;
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

int AudioSource::queueBuffer(std::shared_ptr<AudioBuffer> buffer)
{
    if (!buffer) {
        LOG(LL_ERROR, LCF_SOUND, "Invalid buffer");
        return -1;
    }

    if ((!buffer_queue.empty()) && (buffer->format != format || buffer->channels != channels || buffer->frequency != frequency)) {
        LOG(LL_WARN, LCF_SOUND, "Buffer format mismatch for source %d", id);
    }
    format = buffer->format;
    channels = buffer->channels;
    frequency = buffer->frequency;

    buffer_queue.push_back(buffer);
    return 0;
}

int AudioSource::unqueueBuffer()
{
    if (!buffer_queue.empty()) {
        /* Removing first buffer */
        std::shared_ptr<AudioBuffer> ab = buffer_queue[0];
        buffer_queue.erase(buffer_queue.begin());
        return ab->id;
    }
    return -1;
}

std::shared_ptr<AudioBuffer> AudioSource::reuseBuffer()
{
    if (nbQueueProcessed() > 0 && !buffer_queue.empty()) {
        /* Removing first buffer */
        std::shared_ptr<AudioBuffer> ab = buffer_queue[0];
        buffer_queue.erase(buffer_queue.begin());
        if (queue_index > 0)
            queue_index--;
        return ab;
    }
    return nullptr;
}

void AudioSource::clearBuffers()
{
    buffer_queue.clear();
    queue_index = 0;
    position = 0;
    samples_frac = 0;
}

bool AudioSource::willOutput() const
{
    if (state != SOURCE_PLAYING)
        return false;

    if (buffer_queue.empty())
        return false;

    return true;
}

bool AudioSource::willEnd(struct timespec ticks) const
{
    if (state != SOURCE_PLAYING)
        return false;

    if (buffer_queue.empty())
        return true;

    if (looping)
        return false;

    std::shared_ptr<AudioBuffer> curBuf = buffer_queue[queue_index];

    /* Number of samples to advance in the buffer. We don't use `ticksToSamples()`
     * because it keeps track of the fractional part, and thus must be called
     * exactly once. */
    uint64_t nsecs = static_cast<uint64_t>(ticks.tv_sec) * 1000000000ULL + ticks.tv_nsec;
    int inNbSamples = (nsecs * curBuf->frequency * pitch) / 1000000000ULL;

    int size = queueSize();
    int pos = getPosition();
    return ((pos + inNbSamples) > size);
}


int AudioSource::mixWith( struct timespec ticks, uint8_t* samples_data_out, int samples_byte_size_out, AudioBuffer::SampleFormat format_out, int channels_out, int frequency_out, float volume_out)
{
    if (!willOutput())
        return -1;

    LOG(LL_DEBUG, LCF_SOUND, "Start mixing source %d", id);

    bool skip_mixing = (!audio_converter->isAvailable()) || 
                        (!Global::shared_config.av_dumping && 
                            (Global::shared_config.audio_mute ||
                                (Global::shared_config.fastforward && 
                                    (Global::shared_config.fastforward_mode & SharedConfig::FF_MIXING))));

    std::shared_ptr<AudioBuffer> current_buffer = buffer_queue[queue_index];

    if (current_buffer->frequency <= 0) {
        LOG(LL_ERROR, LCF_SOUND, "Invalid buffer frequency %d for source %d", current_buffer->frequency, id);
        return -1;
    }

    if (!skip_mixing) {
        /* Check if audio converter is initialized.
         * If not, set parameters and init it */
        if (! audio_converter->isInited()) {
            audio_converter->init(current_buffer->format, current_buffer->channels, static_cast<int>(current_buffer->frequency*pitch), format_out, channels_out, frequency_out);
        }
    }

    /* Number of samples to advance in the buffer. */
    int samples_size_in = ticksToSamples(ticks, static_cast<int>(current_buffer->frequency*pitch));

    int position_old = position;
    int position_new = position + samples_size_in;

    uint8_t* buffer_samples;
    int available_samples = current_buffer->getSamples(buffer_samples, samples_size_in, position_old, (source == SOURCE_STATIC) && looping);

    if (available_samples == samples_size_in) {
        /* We did not reach the end of the buffer, easy case */

        position = position_new;
        LOG(LL_DEBUG, LCF_SOUND, "  Buffer %d in read in range %d - %d", current_buffer->id, position_old, position);
        if (!skip_mixing) {
            audio_converter->queueSamples(buffer_samples, samples_size_in);
        }
    }
    else {
        /* We reached the end of the buffer */
        LOG(LL_DEBUG, LCF_SOUND, "  Buffer %d is read from %d to its end %d", current_buffer->id, position_old, current_buffer->sampleSize);
        if (!skip_mixing) {
            if (available_samples > 0)
                audio_converter->queueSamples(buffer_samples, available_samples);
        }

        int remaining_samples = samples_size_in - available_samples;
        if (source == SOURCE_CALLBACK) {
            /* We refill our buffer using the callback function,
             * until we got enough bytes for this frame
             */
            while (remaining_samples > 0) {
                /* Before doing the callback, we must fake that the timer has
                 * advanced by the number of samples already read
                 */
                int64_t extra_ticks = static_cast<int64_t>(1000000000) * (-remaining_samples);
                extra_ticks /= current_buffer->frequency;
                DeterministicTimer& detTimer = DeterministicTimer::get();
                detTimer.fakeAdvanceTimer({static_cast<time_t>(extra_ticks / 1000000000), static_cast<long>(extra_ticks % 1000000000)});
                callback(*current_buffer);
                detTimer.fakeAdvanceTimer({0, 0});
                available_samples = current_buffer->getSamples(buffer_samples, remaining_samples, 0, false);
                if (!skip_mixing) {
                    audio_converter->queueSamples(buffer_samples, available_samples);
                }

                LOG(LL_DEBUG, LCF_SOUND, "  Buffer %d is read again from 0 to %d", current_buffer->id, available_samples);
                if (remaining_samples == available_samples)
                    position = available_samples;
                remaining_samples -= available_samples;
            }
        }
        else {
            int queue_size = buffer_queue.size();
            int final_index = queue_index;
            int final_pos = position_old + available_samples;

            /* Our for loop conditions are different if we are looping or not */
            if (looping) {
                for (int i=(queue_index+1)%queue_size; remaining_samples>0; i=(i+1)%queue_size) {
                    std::shared_ptr<AudioBuffer> loopbuf = buffer_queue[i];
                    available_samples = loopbuf->getSamples(buffer_samples, remaining_samples, loopbuf->loop_point_beg, (source == SOURCE_STATIC) && looping);
                    LOG(LL_DEBUG, LCF_SOUND, "  Buffer %d in read in range %d - %d", loopbuf->id, loopbuf->loop_point_beg, available_samples);

                    if (!skip_mixing) {
                        audio_converter->queueSamples(buffer_samples, available_samples);
                    }

                    final_index = i;
                    final_pos = loopbuf->loop_point_beg + available_samples;
                    remaining_samples -= available_samples;
                }
            }
            else {
                for (int i=queue_index+1; (remaining_samples>0) && (i<queue_size); i++) {
                    std::shared_ptr<AudioBuffer> loopbuf = buffer_queue[i];
                    available_samples = loopbuf->getSamples(buffer_samples, remaining_samples, 0, false);
                    LOG(LL_DEBUG, LCF_SOUND, "  Buffer %d in read in range 0 - %d", loopbuf->id, available_samples);

                    if (!skip_mixing) {
                        audio_converter->queueSamples(buffer_samples, available_samples);
                    }

                    final_index = i;
                    final_pos = available_samples;
                    remaining_samples -= available_samples;
                }
            }

            if (remaining_samples > 0) {
                /* We reached the end of the buffer queue */
                LOG(LL_DEBUG, LCF_SOUND, "  End of the queue reached");
                if (source == SOURCE_STREAMING_CONTINUOUS) {
                    /* Update the position in the buffer */
                    queue_index = final_index;
                    position = final_pos;
                    
                    /* The callback may push more buffers */
                    if (callback) {
                        LOG(LL_DEBUG, LCF_SOUND, "  Callback");
                        callback(*current_buffer); // buffer argument unused
                        
                        int queue_size = buffer_queue.size();
                        for (int i=queue_index+1; (remaining_samples>0) && (i<queue_size); i++) {
                            std::shared_ptr<AudioBuffer> loopbuf = buffer_queue[i];
                            available_samples = loopbuf->getSamples(buffer_samples, remaining_samples, 0, false);
                            LOG(LL_DEBUG, LCF_SOUND, "  Buffer %d in read in range 0 - %d", loopbuf->id, available_samples);

                            if (!skip_mixing) {
                                audio_converter->queueSamples(buffer_samples, available_samples);
                            }

                            final_index = i;
                            final_pos = available_samples;
                            remaining_samples -= available_samples;
                        }

                        queue_index = final_index;
                        position = final_pos;
                    }
                    
                    if (remaining_samples > 0) {
                        state = SOURCE_UNDERRUN;
                    }
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
                queue_index = final_index;
                position = final_pos;
            }
        }
    }
    
    int converter_samples_size = 0;

    if (!skip_mixing) {
        /* Allocate the mixed audio array */
        int samples_size_out = samples_byte_size_out / (channels_out * AudioBuffer::formatToBitDepth(format_out) / 8);
        mixed_samples.resize(samples_byte_size_out);

        /* Get the converter samples */
        converter_samples_size = audio_converter->getSamples(mixed_samples.data(), samples_size_out);

        #define clamptofullsignedrange(x,lo,hi) ((static_cast<unsigned int>((x)-(lo))<=static_cast<unsigned int>((hi)-(lo)))?(x):(((x)<0)?(lo):(hi)))

        int saturate_count = 0;

        /* Mixing source volume and master volume.
         * Taken from openAL doc:
         * "The implementation is free to clamp the total gain (effective gain
         * per-source multiplied by the listener gain) to one to prevent overflow."
         */
        float result_volume = volume * volume_out * Global::shared_config.audio_gain;
        if (result_volume > 1.0f)
            result_volume = 1.0f;

        int result_volume_16bit = (int)(result_volume * 65536.0f);

        /* Add mixed source to the output buffer */
        if (format_out == AudioBuffer::SAMPLE_FMT_U8) {
            for (int s=0; s<converter_samples_size*channels_out; s+=channels_out) {
                int my_left = mixed_samples[s];
                int other_left = samples_data_out[s];
                int sum_left = other_left + ((my_left * result_volume_16bit) >> 16) - 256;
                samples_data_out[s] = clamptofullsignedrange(sum_left, 0, UINT8_MAX);
                saturate_count += (sum_left < 0) || (sum_left > UINT8_MAX);

                if (channels_out == 2) {
                    int my_right = mixed_samples[s+1];
                    int other_right = samples_data_out[s+1];
                    int sum_right = other_right + ((my_right * result_volume_16bit) >> 16) - 256;
                    samples_data_out[s+1] = clamptofullsignedrange(sum_right, 0, UINT8_MAX);
                    saturate_count += (sum_right < 0) || (sum_right > UINT8_MAX);
                }
            }
        }

        else if (format_out == AudioBuffer::SAMPLE_FMT_S16) {
            int16_t* mixed_samples_16 = reinterpret_cast<int16_t*>(mixed_samples.data());
            int16_t* samples_data_16 = reinterpret_cast<int16_t*>(samples_data_out);
            for (int s=0; s<converter_samples_size*channels_out; s+=channels_out) {
                int my_left = mixed_samples_16[s];
                int other_left = samples_data_16[s];
                int sum_left = other_left + ((my_left * result_volume_16bit) >> 16);
                samples_data_16[s] = clamptofullsignedrange(sum_left, INT16_MIN, INT16_MAX);
                saturate_count += (sum_left < INT16_MIN) || (sum_left > INT16_MAX);

                if (channels_out == 2) {
                    int my_right = mixed_samples_16[s+1];
                    int other_right = samples_data_16[s+1];
                    int sum_right = other_right + ((my_right * result_volume_16bit) >> 16);
                    samples_data_16[s+1] = clamptofullsignedrange(sum_right, INT16_MIN, INT16_MAX);
                    saturate_count += (sum_right < INT16_MIN) || (sum_right > INT16_MAX);
                }
            }
        }

        else if (format_out == AudioBuffer::SAMPLE_FMT_FLT) {
            float* mixed_samples_flt = reinterpret_cast<float*>(mixed_samples.data());
            float* samples_data_flt = reinterpret_cast<float*>(samples_data_out);
            for (int s=0; s<converter_samples_size*channels_out; s+=channels_out) {
                float my_left = mixed_samples_flt[s];
                float other_left = samples_data_flt[s];
                float sum_left = other_left + my_left * result_volume;
                if (sum_left > 1.0f) {
                    samples_data_flt[s] = 1.0f;
                    saturate_count++;
                }
                else if (sum_left < -1.0f) {
                    samples_data_flt[s] = -1.0f;
                    saturate_count++;
                }
                else
                    samples_data_flt[s] = sum_left;

                if (channels_out == 2) {
                    float my_right = mixed_samples_flt[s+1];
                    float other_right = samples_data_flt[s+1];
                    float sum_right = other_right + my_right * result_volume;

                    if (sum_right > 1.0f) {
                        samples_data_flt[s+1] = 1.0f;
                        saturate_count++;
                    }
                    else if (sum_right < -1.0f) {
                        samples_data_flt[s+1] = -1.0f;
                        saturate_count++;
                    }
                    else
                        samples_data_flt[s+1] = sum_right;
                }
            }
        }

        else {
            LOG(LL_ERROR, LCF_SOUND, "Unsupported format %d during mixing", format_out);        
        }
        
        if (saturate_count > 0)
            LOG(LL_WARN, LCF_SOUND, "Saturation during mixing for %d samples", saturate_count);        
    }

    /* Reset the audio converter if the source has stopped */
    if (state == SOURCE_STOPPED)
        dirty();

    return converter_samples_size;
}

}
