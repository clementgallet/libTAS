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
#include <stdint.h>

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

#endif

