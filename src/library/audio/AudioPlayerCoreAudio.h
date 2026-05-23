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

#ifndef LIBTAS_AUDIOPLAYERCOREAUDIO_H_INCL
#define LIBTAS_AUDIOPLAYERCOREAUDIO_H_INCL

#include <AudioToolbox/AudioToolbox.h>
#include <vector>
#include <stdint.h>

namespace libtas {

class AudioContext;

/**
 * @class AudioPlayerCoreAudio
 * @brief CoreAudio audio backend for macOS systems.
 *
 * AudioPlayerCoreAudio is the macOS implementation of the audio output layer,
 * using Apple's CoreAudio framework and AudioQueue API to communicate with the
 * audio hardware. It handles low-level interaction with CoreAudio, including
 * audio queue initialization, buffer management, and playback control.
 *
 * This class provides a static interface for initializing the audio device,
 * playing audio frames, and cleaning up resources. All methods are static
 * as this represents a singleton connection to the audio device.
 *
 * The implementation manages:
 * - AudioQueue for handling asynchronous audio playback
 * - Multiple AudioQueue buffers for double/triple-buffering
 * - A cyclic buffer for managing continuous audio stream
 * - Audio device state and error handling
 * - Silence samples for underrun conditions
 *
 * Audio flow:
 * 1. init() creates an AudioQueue and allocates buffers
 * 2. play() copies mixed audio into the cyclic buffer
 * 3. When AudioQueue needs more data, it pulls from the cyclic buffer
 * 4. close() stops playback and releases resources
 *
 * @note This is a platform-specific implementation. Use AudioPlayerCoreAudio only
 *       on macOS systems with CoreAudio support.
 *
 * @see AudioContext for the mixing engine that produces audio
 * @see AudioPlayerAlsa for Linux ALSA implementation
 */
class AudioPlayerCoreAudio
{
public:
    /**
     * @brief Cyclic ring buffer for continuous audio streaming.
     *
     * This structure represents a cyclic (ring) buffer used to store continuous
     * audio samples. The AudioQueue reads from this buffer asynchronously while
     * the mixing engine writes new frames. It prevents the mixing thread and
     * playback thread from interfering with each other.
     *
     * Structure:
     * - beg: read position (oldest data)
     * - end: write position (newest data)
     * - Data wraps around when reaching the end of the buffer
     *
     * @see init(), play()
     */
    struct cyclic_buffer {
        /**
         * @brief Raw audio sample storage.
         *
         * Allocated during init() with capacity `cap`.
         */
        std::vector<uint8_t> data;

        /**
         * @brief Read position (beginning) in the cyclic buffer.
         *
         * Indicates where AudioQueue starts reading samples.
         * Advances as samples are consumed for playback.
         * Wraps around when reaching `cap`.
         */
        int beg;

        /**
         * @brief Write position (end) in the cyclic buffer.
         *
         * Indicates where new samples are written by play().
         * Should not overtake beg (would indicate buffer overflow).
         * Wraps around when reaching `cap`.
         */
        int end;

        /**
         * @brief Number of valid samples currently in the buffer.
         *
         * Derived from beg and end positions. Indicates how much
         * playable audio is currently buffered.
         */
        int size;

        /**
         * @brief Total capacity of the cyclic buffer.
         *
         * Size of the data vector. Positions wrap when reaching this value.
         * Typically sized to hold several frames of audio.
         */
        int cap;

        /**
         * @brief Silence sample value for this format.
         *
         * The sample value representing silence (e.g., 0x00 for unsigned,
         * 0x80 for 8-bit signed). Used for filling buffer during underruns.
         */
        uint8_t silence;
    };
    
    /**
     * @brief Initializes connection to CoreAudio audio device.
     *
     * Establishes connection to the audio hardware through CoreAudio, creates
     * an AudioQueue, configures audio format parameters, allocates buffer pool,
     * and starts the audio queue. Should be called once at startup before
     * attempting to play audio.
     *
     * The audio context provides the necessary device configuration:
     * - Sample format and bit depth (outBitDepth)
     * - Channel configuration (outNbChannels)
     * - Sample rate (outFrequency)
     * - Loopback mode detection (isLoopback)
     *
     * Process:
     * 1. Create and configure AudioStreamBasicDescription from context
     * 2. Create AudioQueue with callback for buffer refills
     * 3. Allocate buffer pool (typically 3 buffers)
     * 4. Initialize cyclic buffer for sample storage
     * 5. Start audio queue
     *
     * @param[in] ac AudioContext reference containing output configuration
     *
     * @return true if initialization succeeded and playback is ready
     *         false if initialization failed (device unavailable, incompatible format, etc.)
     *
     * @note This must be called before play()
     * @note Multiple calls to init() without close() may cause errors
     *
     * @see play(), close(), cyclic_buffer
     */
    static bool init(AudioContext& ac);
    
    /**
     * @brief Plays the current audio frame to the audio device.
     *
     * Submits the mixed audio samples from the AudioContext to the cyclic buffer
     * for asynchronous playback. The AudioQueue will pull samples from the cyclic
     * buffer as needed. This is called once per audio frame.
     *
     * The audio context contains the current frame's mixed audio:
     * - outSamples: the mixed audio buffer
     * - outBytes: size of the buffer in bytes
     *
     * The function:
     * 1. Wraps data into the cyclic buffer (handling wrap-around)
     * 2. Updates the write position (end)
     * 3. Handles overflow if write position catches read position
     *
     * @param[in] ac AudioContext reference containing current frame's mixed audio
     *
     * @return true if audio was successfully queued for playback
     *         false if an error occurred (buffer full, queue not started, etc.)
     *
     * @note init() must be called first
     * @note This is typically called every audio frame from the audio thread
     * @note Uses cyclic_buffer for asynchronous buffering
     *
     * @see init(), AudioContext::outSamples, AudioContext::outBytes, cyclic_buffer
     */
    static bool play(AudioContext& ac);
    
    /**
     * @brief Closes and cleans up the CoreAudio connection.
     *
     * Stops the AudioQueue, releases allocated buffers, and resets the player
     * to uninitialized state. Should be called once during shutdown or when
     * audio output is no longer needed.
     *
     * After calling close(), init() must be called again before playing new audio.
     *
     * @see init()
     */
    static void close();

private:
    /**
     * @brief Enumeration of audio player connection states.
     *
     * Tracks the initialization and operational state of the CoreAudio connection.
     */
    enum APStatus {
        STATUS_ERROR = -1,     /**< Error state - connection failed or error occurred */
        STATUS_UNINIT = 0,     /**< Uninitialized - no connection attempted */
        STATUS_OK = 1,         /**< Operating normally - connection active */
    };

    /**
     * @brief Current status of the CoreAudio connection.
     * @internal
     * Protected by implicit synchronization of static state.
     *
     * @see APStatus
     */
    static APStatus status;

    /**
     * @brief Cyclic buffer for continuous audio streaming.
     * @internal
     * Stores mixed audio samples as a ring buffer. The mixing thread writes
     * to the end, and the AudioQueue thread reads from the beginning.
     *
     * @see cyclic_buffer, play()
     */
    static cyclic_buffer cyclicBuffer;
    
    /**
     * @brief Handle to the AudioQueue for playback.
     * @internal
     * Created by init(), used to manage asynchronous audio playback.
     * Nullptr if not initialized.
     *
     * @see init()
     */
    static AudioQueueRef audioQueue;

    /**
     * @brief Pool of allocated AudioQueue buffers.
     * @internal
     * Vector of buffer references allocated from the AudioQueue.
     * Typically contains 3 buffers for triple-buffering.
     * Used to avoid repeated allocation/deallocation during playback.
     *
     * @see init()
     */
    static std::vector<AudioQueueBufferRef> audioQueueBuffers;

};
}

#endif
