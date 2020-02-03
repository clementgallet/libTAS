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

#ifndef LIBTAS_AUDIOBUFFER_H_INCL
#define LIBTAS_AUDIOBUFFER_H_INCL

#include <vector>
#include <stdint.h>
#include <istream>
#include <string.h> // memset

namespace libtas {
/* Class storing samples of an audio buffer, and all the related information
 * like sample format, frequency, channels, etc.
 */
class AudioBuffer
{
    public:
        AudioBuffer();

        /* Return if the buffer size is correct regarding buffer parameters */
        bool checkSize(void);

        /* Function to access samples of the buffer.
         * @param [out] outSamples    pointer to the beginning of the
         *                            returned samples
         * @param [in]  outNbSamples  number of samples to get
         * @param [in]  position      position (in samples) from where to
         *                            get samples
         * @param [in]  loopstatic    is the source looping and as a static buffer?
         * @return                    the effective number of samples returned
         */
        int getSamples( uint8_t* &outSamples, int outNbSamples, int position, bool loopstatic);

        /* Identifier of the buffer */
        int id;

        /* Make the whole buffer silent */
        void makeSilent();

        /*** Primary parameters ***/

        /* Sample format */
        enum SampleFormat {
            SAMPLE_FMT_U8,  /* Unsigned 8-bit samples */
            SAMPLE_FMT_S16, /* Signed 16-bit samples */
            SAMPLE_FMT_S32, /* Signed 32-bit samples */
            SAMPLE_FMT_FLT, /* 32-bit floating point samples */
            SAMPLE_FMT_DBL, /* 64-bit floating point samples */

            SAMPLE_FMT_MSADPCM, /* Compressed MS-ADPCM format */
            SAMPLE_FMT_NB
        };
        SampleFormat format;

        /* Number of channels of the buffer */
        int nbChannels;

        /* Frequency of buffer in Hz */
        int frequency;

        /* Size of the buffer in bytes */
        int size;

        /* Audio samples */
        std::vector<uint8_t> samples;

        /* Begin of looping section. 0 means disabled */
        int loop_point_beg;

        /* End of looping section. 0 means disabled */
        int loop_point_end;

        /*** Derived parameters, computed by update function ***/

        /* Update all fields below based on above fields */
        void update(void);

        /* Number of samples in a block for compressed formats */
        int blockSamples;

        /* In the case of compressed audio, temporary uncompressed buffer */
        std::vector<int16_t> rawSamples;

        /* Bit depth of the buffer. Computed from format */
        int bitDepth;

        /* Size of a single sample for uncompressed format.
         * Computed from format and nbChannels
        */
        int alignSize;

        /* Size of the buffer in samples. Computed from size and alignSize */
        int sampleSize;

        /* Size of a block in bytes, for compressed formats.
         * Computed from blockSamples and format.
        */
        int blockSize;
};
}

#endif
