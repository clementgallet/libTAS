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

AudioBuffer::AudioBuffer(void)
{
    id = 0;
    format = SAMPLE_FMT_S16;
    bitDepth = 16;
    nbChannels = 2;
    alignSize = 4;
    frequency = 0;
    size = 0;
    processed = false;
}

void AudioBuffer::update(void)
{
    switch (format) {
        case SAMPLE_FMT_U8:
            bitDepth = 8;
            break;
        case SAMPLE_FMT_S16:
            bitDepth = 16;
            break;
        case SAMPLE_FMT_S32:
            bitDepth = 32;
            break;
        case SAMPLE_FMT_FLT:
            bitDepth = 32;
            break;
        case SAMPLE_FMT_DBL:
            bitDepth = 64;
            break;
        default:
            break;
    }

    alignSize = nbChannels * bitDepth / 8;

    switch (format) {
        case SAMPLE_FMT_U8:
        case SAMPLE_FMT_S16:
        case SAMPLE_FMT_S32:
        case SAMPLE_FMT_FLT:
        case SAMPLE_FMT_DBL:
            sampleSize = size / alignSize;
    }
}

