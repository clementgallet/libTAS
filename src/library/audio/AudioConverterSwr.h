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

#ifndef LIBTAS_AUDIOCONVERTERSWR_H_INCL
#define LIBTAS_AUDIOCONVERTERSWR_H_INCL

#include "AudioBuffer.h"
#include "AudioConverter.h"

extern "C" {
#include <libswresample/swresample.h>
}

namespace libtas {
/* Resampler implementation using libswresample library */
class AudioConverterSwr : public AudioConverter
{
public:
    AudioConverterSwr();
    ~AudioConverterSwr();

    bool isAvailable();

    bool isInited();

    void init(AudioBuffer::SampleFormat inFormat, int inChannels, int inFreq, AudioBuffer::SampleFormat outFormat, int outChannels, int outFreq);

    void dirty();

    void queueSamples(const uint8_t* inSamples, int inNbSamples);

    int getSamples(uint8_t* outSamples, int outNbSamples);

private:
    /* Context for resampling audio */
    struct SwrContext *swr;

    /* Version of the linked library */
    unsigned int swr_version;

};
}

#endif
