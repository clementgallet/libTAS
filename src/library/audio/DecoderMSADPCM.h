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

/* Code originally written by Ethan "flibitijibibo" Lee
 * http://www.flibitijibibo.com/
 */

#ifndef LIBTAS_DECODERMSADPCM_H_INCL
#define LIBTAS_DECODERMSADPCM_H_INCL

#include <vector>
#include "BinaryIStream.h"

namespace libtas {
namespace DecoderMSADPCM
{
    /**
     * Decodes MSADPCM data to signed 16-bit PCM data.
     * @param source     [in]  source stream containing the compressed samples
     * @param nbChannels [in]  number of channels
     * @param blockAlign [in]  size (in samples!) of a single ADPCM block
     * @param pcmOut     [out] destination buffer
     */
    void toPCM(BinaryIStream &source, int nbChannels, int sampleAlign, std::vector<int16_t> &pcmOut);

    /**
     * Calculates PCM samples based on previous samples and a nibble input.
     * @param nibble A parsed MSADPCM sample
     * @param predictor The predictor we get from the MSADPCM block's preamble
     * @param sample_1 The first sample we use to predict the next sample
     * @param sample_2 The second sample we use to predict the next sample
     * @param delta Used to calculate the final sample
     * @return The calculated PCM sample
     */
    int16_t calculateSample(uint8_t nibble, uint8_t predictor, int16_t& sample1, int16_t& sample2, int16_t& delta);
}
}

#endif
