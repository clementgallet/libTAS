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

#ifndef AUDIOBUFFER_H_INCL
#define AUDIOBUFFER_H_INCL

#include <vector>
#include <forward_list>
#include <sndfile.hh>

enum SourceType {
    SOURCE_UNDETERMINED,
    SOURCE_STATIC,
    SOURCE_STREAMING,
};

enum SourceState {
    SOURCE_INITIAL,
    SOURCE_PLAYING,
    SOURCE_STOPPED,
    SOURCE_PAUSED,
    /* We add another state here, when a buffer was newly played
     * and must not be played a second time on the same frame.
     * This prevent multiple play on queued buffers.
     * This state is changed to SOURCE_PLAYING after the mix.
     */
    SOURCE_PLAYED,
};

class AudioBuffer
{
    public:
        AudioBuffer();

        /*** Buffer parameters ***/

        /* Identifier of the buffer */
        int id;

        /* Bit depth of the buffer (usually 8 or 16) */
        int bitDepth;

        /* Number of channels of the buffer */
        int nbChannels;

        /* Frequency of buffer in Hz */
        int frequency;

        /* Size of the buffer in bytes */
        int size;

        /* Audio samples */
        std::vector<uint8_t> samples;

        /* Position inside the buffer, in bytes */
        int position;

        /*** Source parameters ***/

        /* Volume of the source.
         * Can be larger than 1 but output volume will be clamped to one */
        float volume;

        /* Is it a static source (we got the entire buffer at once)
         * or a streaming source (we continuously get buffers)
         */
        SourceType source;

        /* Is the audio buffer looping? */
        bool looping;

        /* Is the source playing? */
        SourceState state;

        /* If multiple buffers must be played in sequence,
         * we store a pointer to the next buffer to play,
         * or nullptr if it is the last buffer.
         *
         * On OpenAL, buffers and sources are separated,
         * so we consider the source as the primary buffer
         * with no samples, which points to the actual buffer with samples
         */
        AudioBuffer* nextBuffer;

        /* Indicate if a buffer has been read entirely */
        bool processed;

        /* Returns the number of buffers in its queue
         * that were not processed (not read until the end),
         * not counting itself.
         */
        int nbQueue();

        /* Returns the number of buffers in its queue
         * not counting itself.
         */
        int nbQueueProcessed();

        /* Returns the sum of the sizes of each queued buffer */
        int queueSize();

        /* Get the position of playback inside a queue of buffers
         * The position is relate to the beginning of the first buffer in queue.
         */
        int getPosition();

        /* Set the position of playback inside a queue of buffers.
         * The position is relate to the beginning of the first buffer in queue.
         * Buffers that are traversed by position set are marked as processed.
         * Buffers that are not entirely traversed are marked as non processed.
         *
         * If pos is greater than the size of the buffer queue,
         * all buffers are stopped and traversed.
         *
         * Argument pos is expressed in bytes
         */
        void setPosition(int pos);

        /* Change the state of the buffer.
         * This is not a trivial operation, because if we have a queue of buffers,
         * we need to change the state of the first buffer that is not processed.
         */
        void changeState(SourceState newstate);

        /* Get the state of the buffer.
         * Same as above concerning queued buffers
         */
        SourceState getState();

        /* Mix the buffer with an external buffer of the given format.
         * The number of samples to mix correspond to the number of ticks given.
         * This function is just preparing the buffer to be mixed,
         * the actual mixing is done in an external function.
         */
        void mixWith( struct timespec ticks, uint8_t* outSamples, int outBytes, int outBitDepth, int outNbChannels, int outFrequency, float outVolume);
};


class AudioBufferList
{
    public:
        AudioBufferList();

        /* Master volume.
         * Can be larger than 1 but output volume will be clamped to one */
        float outVolume;

        /* Bit depth of the buffer (usually 8 or 16) */
        int outBitDepth;

        /* Number of channels of the buffer */
        int outNbChannels;

        /* Frequency of buffer in Hz */
        int outFrequency;

        /* Mixed buffer during a frame */
        std::vector<uint8_t> outSamples;

        /* Create a new buffer object and return an id of the buffer or -1 if it failed 
         * Optionally fill abp with a pointer to the created AudioBuffer.
         */
        int createBuffer(AudioBuffer** abp);

        /* Delete buffers that have a corresponding id */
        void deleteBuffer(int id);

        /* Returns if a buffer id correspond to an existing buffer */
        bool isBuffer(int id);

        /* Return the buffer of requested id, or nullptr if not exists */
        AudioBuffer* getBuffer(int id);

        /* Mix all buffers that are playing */
        void mixAllBuffers(struct timespec ticks);

        /*** Temporary!!! WAV output ***/
        SndfileHandle file;

    private:
        std::forward_list<AudioBuffer*> buffers;
};

extern AudioBufferList bufferList;

#endif

