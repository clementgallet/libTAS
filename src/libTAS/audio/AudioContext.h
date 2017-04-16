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

#ifndef LIBTAS_AUDIOCONTEXT_H_INCL
#define LIBTAS_AUDIOCONTEXT_H_INCL

#include <vector>
#include <forward_list>
#include <mutex>
#include "AudioBuffer.h"
#include "AudioSource.h"

/* This class stores a set of audio sources and audio buffers, and
 * is in charge of creating or deleting them.
 * It makes the mixing of all sources that are playing, and
 * eventually send the mixed samples to the audio player
 *
 * For now, only one object is created, even if openAL can
 * deal with multiple contexts, or SDL can open multiple devices.
 */
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

        /* Size of a sample (chan * bitdepth / 8) */
        int outAlignSize;

        /* Frequency of buffer in Hz */
        int outFrequency;

        /* Mixed buffer during a frame */
        std::vector<uint8_t> outSamples;

        /* Size of the mixed buffer in samples */
        int outNbSamples;

        /* Size of the mixed buffer in bytes */
        int outBytes;

        /* Init parameters from the config */
        void init(void);

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

        /* Mutex to protect access to all audio objects */
        std::mutex mutex;

    private:
        std::forward_list<AudioBuffer*> buffers;
        std::forward_list<AudioSource*> sources;
};

extern AudioContext audiocontext;

#endif
