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

#include "DecoderMSADPCM.h"
#include "../logging.h"

namespace libtas {

int16_t DecoderMSADPCM::calculateSample(uint8_t nibble, uint8_t predictor, int16_t& sample1, int16_t& sample2, int16_t& delta)
{
    /*
     * Get a signed number out of the nibble. We need to retain the
     * original nibble value for when we access AdaptionTable[].
     */
    int8_t signedNibble = static_cast<int8_t>(nibble);
    if ((signedNibble & 0x8) == 0x8) {
        signedNibble -= 0x10;
    }

    static const int adaptionTable[] = {
        230, 230, 230, 230, 307, 409, 512, 614,
        768, 614, 512, 409, 307, 230, 230, 230
    };

    static const int adaptCoeff_1[] = {
        256, 512, 0, 192, 240, 460, 392
    };
    static const int adaptCoeff_2[] = {
        0, -256, 0, 64, 0, -208, -232
    };

    /* Calculate new sample */
    int sampleInt = (
            (	(sample1 * adaptCoeff_1[predictor]) +
                (sample2 * adaptCoeff_2[predictor])
            ) /256
            );
    sampleInt += signedNibble * delta;

    /* Clamp result to 16-bit */
    int16_t sample;
    if (sampleInt < INT16_MIN) {
        sample = INT16_MIN;
    } else if (sampleInt > INT16_MAX) {
        sample = INT16_MAX;
    } else {
        sample = static_cast<int16_t>(sampleInt);
    }

    /* Shuffle samples, get new delta */
    sample2 = sample1;
    sample1 = sample;
    delta = static_cast<int16_t>(adaptionTable[nibble] * delta / 256);

    /* Saturate the delta to a lower bound of 16 */
    if (delta < 16)
        delta = 16;

    return sample;
}

void DecoderMSADPCM::toPCM(BinaryIStream &source, int nbChannels, int sampleAlign, std::vector<int16_t> &pcmOut)
{
    /* Mono or Stereo? */
    if (nbChannels == 1) {
        uint8_t predictor;

        /* Read to the end of the buffer */
        while (source >> predictor) {

            int16_t sample1, sample2, delta;

            /* Read block preamble */
            source >> delta >> sample1 >> sample2;

            /* Send the initial samples straight to PCM out. */
            pcmOut.push_back(sample2);
            pcmOut.push_back(sample1);

            /* Go through the bytes in this MSADPCM block. */
            for (int bi = 2; bi < sampleAlign; bi += 2) {
                /* Each sample is one half of a nibbleBlock. */
                uint8_t sampleByte;
                source >> sampleByte;
                if (!source) break;
                pcmOut.push_back(calculateSample(sampleByte >>  4, predictor, sample1, sample2, delta));
                pcmOut.push_back(calculateSample(sampleByte & 0xF, predictor, sample1, sample2, delta));
            }

        }
    } else if (nbChannels == 2) {
        uint8_t lpredictor, rpredictor;

        /* Read to the end of the buffer */
        while (source >> lpredictor) {
            int16_t lsample1, lsample2, ldelta;
            int16_t rsample1, rsample2, rdelta;

            /* Read block preamble */
            source >> rpredictor;
            source >> ldelta >> rdelta;
            source >> lsample1 >> rsample1;
            source >> lsample2 >> rsample2;

            /* Send the initial samples straight to PCM out. */
            pcmOut.push_back(lsample2);
            pcmOut.push_back(rsample2);
            pcmOut.push_back(lsample1);
            pcmOut.push_back(rsample1);

            /* Go through the bytes in this MSADPCM block. */
            for (int bi = 4; bi < (sampleAlign*2); bi += 2) {
                uint8_t sampleByte;
                source >> sampleByte;
                if (!source) break;
                /* Each sample is one half of a nibbleBlock. */
                pcmOut.push_back(calculateSample(sampleByte >>  4, lpredictor, lsample1, lsample2, ldelta));
                pcmOut.push_back(calculateSample(sampleByte & 0xF, rpredictor, rsample1, rsample2, rdelta));
            }
        }
    } else {
        debuglogstdio(LCF_SOUND | LCF_ERROR, "MSADPCM data is not mono or stereo");
    }
}

}
