/*
    Copyright 2015-2023 Clément Gallet <clement.gallet@ens-lyon.org>

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

#ifndef LIBTAS_AUDIOCONVERTER_H_INCL
#define LIBTAS_AUDIOCONVERTER_H_INCL

#include "AudioBuffer.h"

namespace libtas {
/* Interface class to resample audio buffers */
class AudioConverter
{
public:
    /* Returns if the resampler available */
    virtual bool isAvailable() = 0;

    /* Returns if the resampler is inited with the current parameters */
    virtual bool isInited() = 0;

    /* Initialize the resampler with the following in and out parameters */
    virtual void init(AudioBuffer::SampleFormat inFormat, int inChannels, int inFreq, AudioBuffer::SampleFormat outFormat, int outChannels, int outFreq) = 0;

    /* Indicate that parameters have changer, so the resampler will need
     * to reinit on the next resampling */
    virtual void dirty() = 0;

    /* Queue input buffer to be resampled */
    virtual void queueSamples(const uint8_t* inSamples, int inNbSamples) = 0;

    /* Get the resampled buffer into `outSamples` that has a size of `outNbSamples` samples.
     * Returns the actual number of samples from the resampler output. */
    virtual int getSamples(uint8_t* outSamples, int outNbSamples) = 0;
};
}

#endif
