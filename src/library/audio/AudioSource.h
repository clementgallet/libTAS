/*
    Copyright 2015-2024 Clément Gallet <clement.gallet@ens-lyon.org>

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

#ifndef LIBTAS_AUDIOSOURCE_H_INCL
#define LIBTAS_AUDIOSOURCE_H_INCL

#include <vector>
#include <memory>
#include <functional>
#include "AudioConverter.h"

namespace libtas {

class AudioBuffer;

/* Class storing an audio source, whose role is to control the playback
 * of an audio buffer or a queue of audio buffers.
 * It is also in charge of eventually resample the buffer(s) and mix them
 * with an extern sample buffer.
 */
class AudioSource
{
    public:
        AudioSource();

        /* Identifier of the buffer */
        int id;

        /* Position inside the buffer, in samples */
        int position;

        /* Because we might read a non-integer number of samples,
         * we must keep track of the fractional part
         */
        int64_t samples_frac;

        /* Volume of the source.
         * Can be larger than 1 but output volume will be clamped to one */
        float volume;

        /* Is it a static source (we got the entire buffer at once)
         * or a streaming source (we continuously get buffers)
         */
        enum SourceType {
            SOURCE_UNDETERMINED,

            /* Source does only store one audio buffer, and stops at the end
            * of it, or loop back.
            */
            SOURCE_STATIC,

            /* Source stores a queue of buffers and read the next one when the
            * current one is finish reading.
            * The game can pull buffers from the queue that have been read
            * entirely, and push more buffers
            */
            SOURCE_STREAMING,

            /* Same as streaming, but the source does not stop when reaching
            * the end, it keeps waiting for new data. If set, it can call a 
            * callback that may push more buffers to the source.
            */
            SOURCE_STREAMING_CONTINUOUS,

            /* Source stores only one audio buffer. When it is finished reading,
            * it calls a callback function that refills the audio buffer
            */
            SOURCE_CALLBACK,
        };
        SourceType source;

        /* Is the audio buffer looping? */
        bool looping;

        /* Pitch multiplier of the played audio */
        float pitch;

        /* State of the source */
        enum SourceState {
            SOURCE_INITIAL,
            SOURCE_PREPARED,
            SOURCE_PLAYING,
            SOURCE_STOPPED,
            SOURCE_PAUSED,
            SOURCE_UNDERRUN,
        };
        SourceState state;

        /* A queue of buffers to play */
        std::vector<std::shared_ptr<AudioBuffer>> buffer_queue;

        /* Indicate the current position in the buffer queue */
        int queue_index;

        /* Object for resampling audio */
        std::unique_ptr<AudioConverter> audioConverter;

        /* Temporary array of mixed samples */
        std::vector<uint8_t> mixedSamples;

        /* In case of callback type, callback function.
         * We send as an argument a pointer to the buffer to refill.
         */
        std::function<void(AudioBuffer&)> callback;

        /* Callback data avaible to the callback function */
        void* callback_data;

        /* Helper function to convert ticks into a number of samples
         * in the audio buffer
         */
        int ticksToSamples(struct timespec ticks, int frequency);

        /* Init parameters */
        void init();

        /* Rewind source to the beginning of the first buffer */
        void rewind();

        /* Some parameters have changed, so we must set a new resample context */
        void dirty();

        /* Returns the number of buffers in its queue */
        int nbQueue();

        /* Returns the number of buffers in its queue
         * that are read until the end.
         */
        int nbQueueProcessed();

        /* Returns the sum of the sizes of each queued buffer (in samples) */
        int queueSize();

        /* Get the position of playback inside a queue of buffers (in samples).
         * The position is relate to the beginning of the first buffer in queue.
         */
        int getPosition();

        /* Set the position of playback inside a queue of buffers (in samples).
         * The position is relate to the beginning of the first buffer in queue.
         *
         * If pos is greater than the size of the buffer queue,
         * the source is stopped.
         *
         * Argument pos is expressed in samples
         */
        void setPosition(int pos);

        /* Check if reading a number of ticks will reach the end of the source */
        bool willEnd(struct timespec ticks);

        /* Mix the buffer with an external buffer of the given format.
         * The number of samples to mix correspond to the number of ticks given.
         * The function returns the number of samples written in the output buffer.
         */
        int mixWith( struct timespec ticks, uint8_t* outSamples, int outBytes, int outBitDepth, int outNbChannels, int outFrequency, float outVolume);
};
}

#endif
