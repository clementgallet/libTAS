/*
    Copyright 2015-2026 Clément Gallet <clement.gallet@ens-lyon.org>

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
#include "DecoderMSADPCM.h"
#include "BinaryIStream.h"

#include "logging.h"

namespace libtas {

AudioBuffer::AudioBuffer(void)
{
    id = 0;
    format = SAMPLE_FMT_S16;
    bitDepth = 16;
    channels = 2;
    alignSize = 4;
    frequency = 0;
    size = 0;
    sampleSize = 0;
    blockSamples = 0;
    blockSize = 0;
    loop_point_beg = 0;
    loop_point_end = 0;
}

int AudioBuffer::formatToSilenceByte(SampleFormat format)
{
    if (format == SAMPLE_FMT_U8) {
        return 0x80;
    }
    return 0;
}

void AudioBuffer::makeSilent() {
    memset(samples.data(), formatToSilenceByte(format), size);
}

int AudioBuffer::formatToBitDepth(SampleFormat format)
{
    switch (format) {
        case SAMPLE_FMT_U8:
            return 8;
        case SAMPLE_FMT_S16:
            return 16;
        case SAMPLE_FMT_S32:
            return 32;
        case SAMPLE_FMT_FLT:
            return 32;
        case SAMPLE_FMT_DBL:
            return 64;
        default:
            return 8;
    }
}

void AudioBuffer::update(void)
{
    bitDepth = formatToBitDepth(format);

    alignSize = channels * bitDepth / 8;

    switch (format) {
        case SAMPLE_FMT_U8:
        case SAMPLE_FMT_S16:
        case SAMPLE_FMT_S32:
        case SAMPLE_FMT_FLT:
        case SAMPLE_FMT_DBL:
            if (alignSize <= 0) {
                LOG(LL_ERROR, LCF_SOUND, "Invalid audio alignment for format %d, channels %d, bit depth %d", format, channels, bitDepth);
                sampleSize = 0;
            }
            else {
                sampleSize = size / alignSize;
            }
            break;
        case SAMPLE_FMT_MSADPCM:

            /* Number of bytes of a block */
            blockSize = channels * (7 + (blockSamples - 2) / 2);

            if (blockSize <= 0 || channels <= 0) {
                LOG(LL_ERROR, LCF_SOUND, "Invalid MSADPCM block geometry, channels %d, block samples %d", channels, blockSamples);
                sampleSize = 0;
            }
            else {
                sampleSize = blockSamples * (size / blockSize);
                if ((size % blockSize) >= (7 * channels))
                    /* We have an incomplete block */
                    sampleSize += 2 + ((size % blockSize)/channels - 7) * 2;
            }
            break;
        default:
            break;
    }
}

bool AudioBuffer::checkSize(void)
{
    switch (format) {
        case SAMPLE_FMT_U8:
        case SAMPLE_FMT_S16:
        case SAMPLE_FMT_S32:
        case SAMPLE_FMT_FLT:
        case SAMPLE_FMT_DBL:
            if (alignSize <= 0)
                return false;
            return (size % alignSize) == 0;
        case SAMPLE_FMT_MSADPCM:
            if (channels <= 0)
                return false;
            return (size % channels) == 0;
        default:
            break;
    }
    return true;
}

int AudioBuffer::getSamples(uint8_t* &outSamples, int nbSamples, int position, bool loopstatic)
{
    /* If the buffer is empty (e.g. for ALSA we push an empty buffer to save
     * params), return immediatly. */
    if (size == 0)
        return 0;

    if (nbSamples <= 0 || position < 0)
        return 0;
        
    /* If the position is at the end of the buffer, return immediatly */
    if (position >= sampleSize)
        return 0;
        
    switch (format) {
        case SAMPLE_FMT_NB:
        case SAMPLE_FMT_UNKNOWN:
            break;
        case SAMPLE_FMT_U8:
        case SAMPLE_FMT_S16:
        case SAMPLE_FMT_S32:
        case SAMPLE_FMT_FLT:
        case SAMPLE_FMT_DBL:
            if (alignSize <= 0)
                return 0;

            if ((position > (size / alignSize)) || (position * alignSize > size))
                return 0;

            /* Simple case, we just return a position in our sample buffer */
            outSamples = &samples[position*alignSize];

            /* Check if we must consider loop points */
            if (loopstatic && (loop_point_end != 0)) {
                if ((loop_point_end - position) >= nbSamples)
                    /* We can return the number of samples asked */
                    return nbSamples;
                else
                    /* We reach the end of the loop point */
                    return loop_point_end - position;
            }

            if ((sampleSize - position) >= nbSamples)
                /* We can return the number of samples asked */
                return nbSamples;
            else
                /* We reach the end of the buffer */
                return (sampleSize - position);
        case SAMPLE_FMT_MSADPCM:

            if (blockSamples <= 0 || blockSize <= 0 || channels <= 0)
                return 0;

            /*** 1. Compute which portion of our buffer we decompress ***/

            /* Number of blocks to read */
            int firstBlock = position / blockSamples;
            int lastBlock = 1 + (position + nbSamples - 1) / blockSamples;

            /* Pointer to the beginning of the first block to read */
            uint8_t* firstSamples = &samples[firstBlock*blockSize];

            /* Size of the portion of compressed buffer to decompress */
            int portionSize = std::min(size - firstBlock*blockSize, (lastBlock-firstBlock)*blockSize);

            /* We wrap this portion into an home-made binary stream */
            BinaryIStream sourceStream(firstSamples, portionSize);

            /*** 2. Prepare the uncompressed buffer ***/

            /* Compute the maximum uncompressed size */
            int rawSize = nbSamples * channels;
            rawSamples.clear();
            rawSamples.reserve(rawSize);

            /*** 3. Call the decompression routine ***/
            DecoderMSADPCM::toPCM(sourceStream, channels, blockSamples, rawSamples);

            /*** 4. Return the proper values ***/
            int rawPosition = position % blockSamples;
            outSamples = reinterpret_cast<uint8_t*>(&rawSamples[rawPosition*channels]);
            int totSamples = nbSamples;
            if ((rawSamples.size()/channels - rawPosition) < static_cast<size_t>(nbSamples))
                totSamples = rawSamples.size()/channels - rawPosition;

            LOG(LL_DEBUG, LCF_SOUND, "   Decompressed %d B -> %d B", portionSize, rawSamples.size());
            return totSamples;
    }
    return 0;
}

}
