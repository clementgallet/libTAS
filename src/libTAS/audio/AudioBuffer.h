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

#ifndef LIBTAS_AUDIOBUFFER_H_INCL
#define LIBTAS_AUDIOBUFFER_H_INCL

#include <vector>
#include <stdint.h>

enum SampleFormat {
    SAMPLE_FMT_U8,  /* Unsigned 8-bit samples */
    SAMPLE_FMT_S16, /* Signed 16-bit samples */ 
    SAMPLE_FMT_S32, /* Signed 32-bit samples */ 
    SAMPLE_FMT_FLT, /* 32-bit floating point samples */ 
    SAMPLE_FMT_DBL, /* 64-bit floating point samples */ 

    SAMPLE_FMT_MSADPCM, /* Compressed MS-ADPCM format */ 
    SAMPLE_FMT_NB
};

/* Class storing samples of an audio buffer, and all the related information
 * like sample format, frequency, channels, etc.
 */
class AudioBuffer
{
    public:
        AudioBuffer();

        /* Update fields (bitDepth, alignSize, sampleSize) based on sample format */
        void update(void);

        /* Return if the buffer size is correct regarding buffer parameters */
        bool checkSize(void);

        /* Function to access samples of the buffer.
         * @param [out] outSamples    pointer to the beginning of the
         *                            returned samples
         * @param [in]  outNbSamples  number of samples to get
         * @param [in]  position      position (in samples) from where to
         *                            get samples
         * @return                    the effective number of samples returned
         */
        int getSamples( uint8_t* &outSamples, int outNbSamples, int position);

        /* Identifier of the buffer */
        int id;

        /* Sample format */
        SampleFormat format;

        /* Bit depth of the buffer */
        int bitDepth;

        /* Number of channels of the buffer */
        int nbChannels;

        /* Size of a single sample (chan * bitdepth / 8) */
        int alignSize;

        /* Frequency of buffer in Hz */
        int frequency;

        /* Size of the buffer in bytes */
        int size;

        /* Size of the buffer in samples */
        int sampleSize;

        /* Audio samples */
        std::vector<uint8_t> samples;

        /* Indicate if a buffer has been read entirely */
        bool processed;

        /* Number of samples in a block for compressed formats
         * -1: default value
         */
        int blockSamples = -1;

};

#endif

