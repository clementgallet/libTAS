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

#ifndef LIBTAS_AUDIOCONVERTER_H_INCL
#define LIBTAS_AUDIOCONVERTER_H_INCL

#include "AudioBuffer.h"

namespace libtas {

/**
 * @class AudioConverter
 * @brief Abstract interface for audio resampling and format conversion.
 *
 * AudioConverter defines the interface for converting audio samples from one
 * format/frequency/channels to another. This is used to resample audio sources
 * to match the output device's configuration.
 *
 * Different backend implementations (e.g., libswresample, CoreAudio on macOS)
 * provide concrete implementations of this interface. The specific implementation
 * used depends on the platform and available libraries.
 *
 * The conversion process involves:
 * 1. Initializing with input and output format parameters
 * 2. Queuing input samples
 * 3. Retrieving converted output samples
 *
 * @see AudioConverterSwr for libswresample implementation
 * @see AudioConverterCoreAudio for macOS CoreAudio implementation
 * @see AudioSource for how converters are used
 */
class AudioConverter
{
public:
    /**
     * @brief Virtual destructor.
     */
    virtual ~AudioConverter() = default;

    /**
     * @brief Checks if the resampler backend is available on this platform.
     *
     * Returns whether the underlying resampling library/API is available and
     * can be used. Useful for checking if a specific implementation is available
     * before attempting to create or use an instance.
     *
     * @return true if the resampler is available, false otherwise
     *
     * @see isInited()
     */
    virtual bool isAvailable() = 0;

    /**
     * @brief Checks if the resampler is initialized with current parameters.
     *
     * Returns whether the resampler has been initialized with the current
     * input/output format parameters. If parameters change, the resampler must
     * be reinitialized by calling init() again or dirty().
     *
     * @return true if initialized and ready to resample, false otherwise
     *
     * @see init(), dirty()
     */
    virtual bool isInited() = 0;

    /**
     * @brief Initializes the resampler with input and output parameters.
     *
     * Configures the resampler to convert audio from the specified input format
     * to the specified output format. The resampler state is reset during this
     * call. Subsequent calls to queueSamples() and getSamples() will use these
     * parameters.
     *
     * Any samples previously queued are discarded when init() is called.
     *
     * @param[in] inFormat  Format of input samples (sample format enum)
     * @param[in] inChannels Number of channels in input samples
     * @param[in] inFreq    Sample rate of input samples in Hz
     * @param[in] outFormat Format of output samples (sample format enum)
     * @param[in] outChannels Number of channels in output samples
     * @param[in] outFreq   Sample rate of output samples in Hz
     *
     * @see AudioBuffer::SampleFormat
     * @see isInited(), dirty()
     */
    virtual void init(AudioBuffer::SampleFormat inFormat, int inChannels, int inFreq,
                      AudioBuffer::SampleFormat outFormat, int outChannels, int outFreq) = 0;

    /**
     * @brief Marks the resampler as dirty, requiring reinitialization.
     *
     * Signals that input or output parameters have changed (e.g., frequency or
     * channel count). The resampler will reinitialize on the next resampling
     * operation. Use this when audio properties change but you want to defer
     * actual reinitialization until needed.
     *
     * @see init(), isInited()
     */
    virtual void dirty() = 0;

    /**
     * @brief Queues input audio samples for conversion.
     *
     * Buffers the input samples for processing. The samples are stored internally
     * in the resampler's input buffer. Multiple calls to queueSamples() accumulate
     * samples in the buffer. Retrieved output via getSamples().
     *
     * @param[in] inSamples Pointer to input audio samples
     * @param[in] inNbSamples Number of samples (frames) to queue
     *
     * @note Samples must be in the input format/channels/frequency specified in init()
     *
     * @see getSamples(), init()
     */
    virtual void queueSamples(const uint8_t* inSamples, int inNbSamples) = 0;

    /**
     * @brief Retrieves resampled output audio samples.
     *
     * Converts queued input samples to the output format and returns them.
     * Attempts to fill the output buffer with the requested number of samples.
     * Returns the actual number of samples produced, which may be less than
     * requested if insufficient input data is available.
     *
     * The caller must provide an output buffer large enough to hold outNbSamples
     * in the output format (outNbSamples * outChannels * (outBitDepth / 8) bytes).
     *
     * @param[out] outSamples Pointer to output buffer to fill with converted samples
     * @param[in] outNbSamples Number of samples (frames) to produce
     *
     * @return The actual number of samples written to outSamples. May be less than
     *         outNbSamples if the resampler doesn't have enough input data to
     *         produce that many output samples.
     *
     * @see queueSamples(), init()
     */
    virtual int getSamples(uint8_t* outSamples, int outNbSamples) = 0;
};
};

#endif
