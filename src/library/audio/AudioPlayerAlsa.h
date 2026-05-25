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

#ifndef LIBTAS_AUDIOPLAYERALSA_H_INCL
#define LIBTAS_AUDIOPLAYERALSA_H_INCL

#include <alsa/asoundlib.h>
#include <stdint.h>
#include <vector>

namespace libtas {

class AudioContext;

/**
 * @class AudioPlayerAlsa
 * @brief ALSA audio backend for Linux systems.
 *
 * AudioPlayerAlsa is the Linux implementation of the audio output layer,
 * using ALSA (Advanced Linux Sound Architecture) to communicate with the
 * audio hardware. It handles low-level interaction with ALSA, including
 * PCM device initialization, buffer submission, and playback control.
 *
 * This class provides a static interface for initializing the audio device,
 * playing audio frames, and cleaning up resources. All methods are static
 * as this represents a singleton connection to the audio device.
 *
 * The implementation manages:
 * - PCM device connection via snd_pcm_t handle
 * - Audio device state and error handling
 * - Silence buffers for underrun conditions
 *
 * @note This is a platform-specific implementation. Use AudioPlayerAlsa only
 *       on Linux systems with ALSA support.
 *
 * @see AudioContext for the mixing engine that produces audio
 * @see AudioPlayerCoreAudio for macOS implementation
 */
class AudioPlayerAlsa
{
private:
    /**
     * @brief Enumeration of audio player connection states.
     *
     * Tracks the initialization and operational state of the ALSA connection.
     */
    enum APStatus {
        STATUS_ERROR = -1,     /**< Error state - connection failed or error occurred */
        STATUS_UNINIT = 0,     /**< Uninitialized - no connection attempted */
        STATUS_OK = 1,         /**< Operating normally - connection active */
    };

    /**
     * @brief Current status of the ALSA connection.
     * @internal
     * Protected by implicit synchronization of static state.
     */
    static APStatus status;

    /**
     * @brief ALSA PCM device handle.
     * @internal
     * Handle to the audio playback PCM (Pulse Code Modulation) device.
     * Nullptr if not initialized.
     *
     * @see init()
     */
    static snd_pcm_t *phandle;

    /**
     * @brief Silence buffer for underrun handling.
     * @internal
     * Pre-allocated buffer filled with silence samples. Used during underrun
     * conditions when no audio data is available to play, to avoid device errors.
     */
    static std::vector<uint8_t> silence;
    
public:
    /**
     * @brief Initializes connection to ALSA audio device.
     *
     * Establishes connection to the audio hardware through ALSA, configures
     * PCM parameters (sample rate, channels, format), and allocates necessary
     * resources. Should be called once at startup before attempting to play audio.
     *
     * The audio context provides the necessary device configuration:
     * - Sample format and bit depth (outBitDepth)
     * - Channel configuration (outNbChannels)
     * - Sample rate (outFrequency)
     * - Loopback mode detection (isLoopback)
     *
     * @param[in] ac AudioContext reference containing output configuration
     *
     * @return true if initialization succeeded and device is ready for playback
     *         false if initialization failed (device unavailable, incompatible format, etc.)
     *
     * @note This must be called before play()
     * @note Multiple calls to init() without close() may cause errors
     *
     * @see play(), close()
     */
	static APStatus init(const AudioContext& ac);

    /**
     * @brief Plays the current audio frame to the audio device.
     *
     * Submits the mixed audio samples from the AudioContext to the ALSA device
     * for playback. This is called once per audio frame to stream the mixed audio
     * to the hardware.
     *
     * The audio context contains the current frame's mixed audio:
     * - outSamples: the mixed audio buffer
     * - outBytes: size of the buffer in bytes
     *
     * The function handles synchronization with the device and manages underrun
     * conditions where the device runs out of audio data.
     *
     * @param[in] ac AudioContext reference containing current frame's mixed audio
     *
     * @return true if audio was successfully submitted to the device
     *         false if an error occurred during submission
     *
     * @note init() must be called first
     * @note This is typically called every audio frame from the audio thread
     *
     * @see init(), AudioContext::outSamples, AudioContext::outBytes
     */
	static bool play(const AudioContext& ac);

    /**
     * @brief Closes and cleans up the ALSA audio connection.
     *
     * Closes the PCM device connection, releases allocated resources, and
     * resets the player to uninitialized state. Should be called once during
     * shutdown or when audio output is no longer needed.
     *
     * After calling close(), init() must be called again before playing new audio.
     *
     * @see init()
     */
    static void close();
};
}

#endif
