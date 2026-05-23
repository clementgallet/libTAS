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

#ifndef LIBTAS_AUDIOBUFFER_H_INCL
#define LIBTAS_AUDIOBUFFER_H_INCL

#include <vector>
#include <stdint.h>
#include <istream>
#include <string.h> // memset

namespace libtas {
/**
 * @class AudioBuffer
 * @brief Container for audio sample data and associated metadata.
 *
 * This class represents an audio buffer that stores raw audio samples along with
 * all necessary information to interpret and play those samples. It handles multiple
 * audio formats (PCM, compressed), supports looping, and provides utility functions
 * for accessing and manipulating the audio data.
 *
 * The class maintains both primary parameters (format, channels, frequency, size)
 * and derived parameters (bit depth, sample size, block size) that are automatically
 * computed by the update() method.
 *
 * @see AudioSource for how buffers are queued and played
 * @see AudioContext for buffer lifecycle management
 */
class AudioBuffer
{
    public:
        /**
         * @brief Constructs a new empty AudioBuffer.
         *
         * Initializes the buffer with default values. Call update() after setting
         * the primary parameters to compute derived parameters.
         */
        AudioBuffer();

        /**
         * @brief Validates that the buffer size matches the declared parameters.
         *
         * Verifies that the size of the samples vector is consistent with the
         * declared format, channels, and frequency. This is useful for debugging
         * and verifying buffer integrity.
         *
         * @return true if the buffer size is consistent with parameters, false otherwise
         *
         * @see update()
         */
        bool checkSize(void);

        /**
         * @brief Retrieves audio samples from the buffer, handling looping as needed.
         *
         * Provides access to a segment of audio samples from the buffer. When the
         * source is looping and this is a static buffer, sample access wraps around
         * from the end back to the beginning of the buffer. Supports fractional
         * sample positions.
         *
         * @param[out] outSamples     Reference to pointer to the start of returned samples.
         *                            Will be set to point to internal sample data.
         * @param[in]  outNbSamples   Number of samples to retrieve
         * @param[in]  position       Starting position in samples from which to read
         * @param[in]  loopstatic     Whether source is looping and buffer is static.
         *                            If true, wraps sample reading at buffer end.
         *
         * @return The actual number of samples returned. May be less than outNbSamples
         *         if reading extends beyond the buffer end (when not looping).
         *
         * @note The returned pointer points directly to internal data. Do not modify
         *       the buffer while external code holds this pointer.
         *
         * @see AudioSource::getSamples()
         */
        int getSamples( uint8_t* &outSamples, int outNbSamples, int position, bool loopstatic);

        /**
         * @brief Unique identifier for this buffer.
         *
         * Used by AudioContext and AudioSource to reference buffers across
         * the system. Assigned by AudioContext when buffer is created.
         */
        int id;

        /**
         * @brief Fills the entire buffer with silence.
         *
         * Zeroes out all audio samples in the buffer. The silence value used
         * depends on the format (0x00 for unsigned formats, 0x80 for signed).
         */
        void makeSilent();

        /*** Primary parameters ***/

        /**
         * @brief Enumeration of supported audio sample formats.
         *
         * Defines the encoding format of individual audio samples in the buffer.
         * Each format has a different bit depth and interpretation:
         * - Unsigned/Signed PCM formats: standard linear audio
         * - Floating point formats: normalized samples (-1.0 to 1.0)
         * - MSADPCM: compressed format using Microsoft ADPCM algorithm
         */
        enum SampleFormat {
            SAMPLE_FMT_UNKNOWN = -1,  /**< Unknown or invalid format */
            SAMPLE_FMT_U8,            /**< Unsigned 8-bit samples (0-255) */
            SAMPLE_FMT_S16,           /**< Signed 16-bit samples (-32768 to 32767) */
            SAMPLE_FMT_S32,           /**< Signed 32-bit samples */
            SAMPLE_FMT_FLT,           /**< 32-bit floating point samples (-1.0 to 1.0) */
            SAMPLE_FMT_DBL,           /**< 64-bit floating point samples (-1.0 to 1.0) */
            SAMPLE_FMT_MSADPCM,       /**< Compressed MS-ADPCM format */
            SAMPLE_FMT_NB             /**< Total number of formats */
        };

        /**
         * @brief Sample format of this buffer.
         *
         * Determines how to interpret the raw bytes in the samples vector.
         * All queued buffers in an AudioSource must have the same format.
         *
         * @see formatToBitDepth()
         */
        SampleFormat format;

        /**
         * @brief Converts an audio sample format to its bit depth.
         *
         * @param f The sample format to query
         *
         * @return The number of bits per sample:
         *         - 8 for SAMPLE_FMT_U8
         *         - 16 for SAMPLE_FMT_S16
         *         - 32 for SAMPLE_FMT_S32, SAMPLE_FMT_FLT, or SAMPLE_FMT_MSADPCM
         *         - 64 for SAMPLE_FMT_DBL
         *         - -1 for SAMPLE_FMT_UNKNOWN
         *
         * @see format, alignSize, bitDepth
         */
        static int formatToBitDepth(SampleFormat f);

        /**
         * @brief Number of audio channels in this buffer.
         *
         * Typical values: 1 (mono), 2 (stereo).
         * All queued buffers in an AudioSource must have the same channel count.
         *
         * @see alignSize
         */
        int channels;

        /**
         * @brief Sample rate of this buffer in Hz (samples per second).
         *
         * Typical values: 22050, 44100, 48000.
         * All queued buffers in an AudioSource must have the same frequency.
         *
         * @see AudioSource::frequency
         */
        int frequency;

        /**
         * @brief Size of the audio data in bytes.
         *
         * This is the actual size of the samples vector. Computed from the
         * number of sample frames and the sample size (format * channels).
         *
         * @see alignSize, sampleSize
         */
        int size;

        /**
         * @brief Raw audio sample data.
         *
         * Contains the actual audio samples encoded according to the format,
         * channels, and frequency parameters. The interpretation of this data
         * depends on the sample format.
         *
         * @see format, channels, frequency, size
         */
        std::vector<uint8_t> samples;

        /**
         * @brief Start position of the loop region in samples, or 0 if looping disabled.
         *
         * If both loop_point_beg and loop_point_end are non-zero, playback will
         * jump back to loop_point_beg when reaching loop_point_end.
         * Only used for static audio sources (single buffer).
         *
         * @see loop_point_end, AudioSource::looping
         */
        int loop_point_beg;

        /**
         * @brief End position of the loop region in samples, or 0 if looping disabled.
         *
         * If both loop_point_beg and loop_point_end are non-zero, playback will
         * jump back to loop_point_beg when reaching this position.
         * Only used for static audio sources (single buffer).
         *
         * @see loop_point_beg, AudioSource::looping
         */
        int loop_point_end;

        /*** Derived parameters, computed by update() function ***/

        /**
         * @brief Recomputes all derived parameters based on primary parameters.
         *
         * After setting format, channels, frequency, and size, call this method
         * to recalculate the derived parameters (bitDepth, alignSize, sampleSize, etc.).
         * This is typically called by AudioContext when creating or modifying buffers.
         *
         * @see bitDepth, alignSize, sampleSize, blockSize, blockSamples
         */
        void update(void);

        /**
         * @brief Number of samples in a compressed audio block (for MSADPCM).
         *
         * For compressed formats like MSADPCM, audio is organized into fixed-size
         * blocks. This value specifies how many samples are contained in each block.
         * Zero for uncompressed formats.
         *
         * @see blockSize, format
         */
        int blockSamples;

        /**
         * @brief Temporary buffer for decompressed audio data.
         *
         * Used internally when dealing with compressed audio formats (e.g., MSADPCM)
         * to store decompressed samples in 16-bit signed PCM format during processing.
         * Not used for uncompressed formats.
         *
         * @see format, samples
         */
        std::vector<int16_t> rawSamples;

        /**
         * @brief Bit depth of individual audio samples.
         *
         * The number of bits per sample, derived from the format.
         * Examples: 8, 16, 32, or 64 bits.
         * Automatically computed by update() based on the format.
         *
         * @see format, formatToBitDepth()
         */
        int bitDepth;

        /**
         * @brief Size of one audio frame in bytes (all channels for one sample period).
         *
         * Computed as: (bitDepth / 8) * channels
         * Example: stereo 16-bit PCM has alignSize = 4 bytes
         * Automatically computed by update() based on format and channels.
         *
         * @see format, channels, bitDepth, sampleSize
         */
        int alignSize;

        /**
         * @brief Total number of audio samples (frames) in the buffer.
         *
         * Computed as: size / alignSize
         * This is the total number of complete frames across all channels.
         * Automatically computed by update() based on size and alignSize.
         *
         * @see size, alignSize
         */
        int sampleSize;

        /**
         * @brief Size of one compressed audio block in bytes (for MSADPCM).
         *
         * For compressed formats like MSADPCM, audio blocks are fixed-size chunks.
         * This specifies the size in bytes of each block.
         * Zero for uncompressed formats.
         * Automatically computed by update() based on blockSamples and format.
         *
         * @see blockSamples, format
         */
        int blockSize;
};
}

#endif
