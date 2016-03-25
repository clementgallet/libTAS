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
};

class AudioBuffer
{
    public:
        AudioBuffer();

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

        /* Indicate if a buffer has been read entirely */
        bool processed;
};

class AudioSource
{
    public:
        AudioSource();

        /* Identifier of the buffer */
        int id;

        /* Position inside the buffer, in bytes */
        int position;

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

        /* A queue of buffers to play */
        /* TODO: choose another struct like forward_list ? */
        std::vector<AudioBuffer*> buffer_queue;

        /* Indicate the current position in the buffer queue */
        int queue_index;

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

        /* Mix the buffer with an external buffer of the given format.
         * The number of samples to mix correspond to the number of ticks given.
         * This function is just preparing the buffer to be mixed,
         * the actual mixing is done in an external function.
         */
        void mixWith( struct timespec ticks, uint8_t* outSamples, int outBytes, int outBitDepth, int outNbChannels, int outFrequency, float outVolume);
};


class AudioContext
{
    public:
        AudioContext();

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

        /* Create a new buffer object and return an id of the buffer or -1 if it failed */
        int createBuffer(void);

        /* Delete buffers that have a corresponding id */
        void deleteBuffer(int id);

        /* Returns if a buffer id correspond to an existing buffer */
        bool isBuffer(int id);

        /* Return the buffer of requested id, or nullptr if not exists */
        AudioBuffer* getBuffer(int id);

        /* Create a new source object and return an id of the source or -1 if it failed */
        int createSource(void);

        /* Delete source that have a corresponding id */
        void deleteSource(int id);

        /* Returns if a source id correspond to an existing source */
        bool isSource(int id);

        /* Return the source of requested id, or nullptr if not exists */
        AudioSource* getSource(int id);

        /* Mix all source that are playing */
        void mixAllSources(struct timespec ticks);

        /*** Temporary!!! WAV output ***/
        SndfileHandle file;

    private:
        std::forward_list<AudioBuffer*> buffers;
        std::forward_list<AudioSource*> sources;
};

extern AudioContext audiocontext;

#endif

