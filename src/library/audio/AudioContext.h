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

#ifndef LIBTAS_AUDIOCONTEXT_H_INCL
#define LIBTAS_AUDIOCONTEXT_H_INCL

#include "AudioBuffer.h" // SampleFormat

#include <vector>
#include <memory>
#include <list>
#include <mutex>

namespace libtas {

class AudioBuffer;
class AudioSource;


/**
 * @class AudioContext
 * @brief Central manager for audio resources and mixing.
 *
 * AudioContext is the main audio subsystem class that manages all audio buffers
 * and sources, coordinates audio mixing, and handles audio output. It maintains
 * lifecycle management for both buffers and sources (creation, deletion, reuse),
 * performs real-time audio mixing of all active sources, and communicates the
 * mixed audio to the platform-specific audio player.
 *
 * The class uses object pooling (buffer_pool, sources_pool) to reduce allocation
 * overhead during runtime. It is thread-safe through a mutex protecting all
 * audio object access.
 *
 * Currently, only one AudioContext instance is created per application, even though
 * OpenAL supports multiple contexts and some platforms support multiple audio devices.
 *
 * @see AudioBuffer for audio data representation
 * @see AudioSource for playback control
 * @see AudioConverter for resampling
 */
class AudioContext
{
    public:
        /**
         * @brief Constructs an AudioContext.
         *
         * Initializes the context with default parameters. Call init() to configure
         * the context from the application settings.
         */
        AudioContext();

        /**
         * @brief Retrieves the singleton AudioContext instance.
         *
         * @return Reference to the global AudioContext instance
         */
        static AudioContext& get();

        /**
         * @brief Master volume for all audio output.
         *
         * Multiplier applied to all mixed output. Values greater than 1.0 will
         * amplify the signal but may cause clipping if output exceeds the valid
         * range for the output format. Output is typically clamped to prevent overflow.
         *
         * Range: [0.0, +inf), typical 0.0 to 1.0
         * Default: 1.0
         */
        float volume;

        /**
         * @brief Output audio format.
         *
         * Specifies the audio format of the mixed audio sent to the audio player.
         * Typical values: 8 or 16 bits. Must match the configured output format
         * on the audio device.
         */
        AudioBuffer::SampleFormat format;

        /**
         * @brief Default output audio format.
         *
         * Holds the default audio format when audio drivers request it.
         */
        static AudioBuffer::SampleFormat default_format;

        /**
         * @brief Number of audio channels in the output.
         *
         * Typical values: 1 (mono), 2 (stereo).
         * Defines the channel configuration for the mixed output stream.
         */
        int channels;

        /**
         * @brief Default number of audio channels in the output.
         *
         * Holds the default number of channels when audio drivers request it.
         */
        static int default_channels;

        /**
         * @brief Size of one output sample frame in bytes.
         *
         * Computed as: (bitdepth / 8) * channels
         * Example: stereo 16-bit output has bytes_per_sample = 4 bytes
         * Automatically computed by init().
         *
         * @see bitdepth, channels
         */
        int bytes_per_sample;

        /**
         * @brief Output sample rate in Hz (samples per second).
         *
         * Typical values: 22050, 44100, 48000
         * Frequency at which the mixed audio is sent to the audio device.
         */
        int frequency;

        /**
         * @brief Default output sample rate in Hz (samples per second).
         *
         * Holds the default frequency when audio drivers request it.
         */
        static int default_frequency;

        /**
         * @brief Buffer containing the current frame's mixed audio samples.
         *
         * Filled by mixAllSources() during each audio frame.
         * Sent to the audio player by the AudioPlayer implementations.
         * Size is samples_byte_size bytes per frame.
         *
         * @see mixAllSources(), samples_byte_size
         */
        std::vector<uint8_t> samples_data;

        /**
         * @brief Number of samples in the mixed output for the current frame.
         *
         * Specifies how many individual samples (frames) are in the outSamples buffer.
         * Computed during mixAllSources().
         *
         * @see samples_data, samples_byte_size
         */
        int samples_size;

        /**
         * @brief Size of the mixed output buffer in bytes for the current frame.
         *
         * Computed as: samples_size * bytes_per_sample
         * Specifies the total byte size of samples_data buffer.
         *
         * @see samples_data, samples_count, bytes_per_sample
         */
        int samples_byte_size;

        /**
         * @brief Whether this context is a loopback device.
         *
         * Loopback devices capture audio output locally instead of sending to
         * actual audio hardware. Useful for debugging and testing.
         *
         * true = loopback device (audio not sent to hardware)
         * false = normal device (audio sent to audio hardware)
         */
        bool is_loopback;

        /**
         * @brief Whether audio playback is paused.
         *
         * When true, audio mixing stops but state is preserved. When resumed
         * (set to false), playback continues from where it paused.
         *
         * true = playback paused (sources not mixed)
         * false = playback active (sources are mixed)
         */
        bool paused;

        /**
         * @brief Initializes the AudioContext with specific values.
         *
         * Sets the context parameters (output format, channels, frequency) to the provided values.
         * Use 0 for any parameter to keep the current value.
         *
         * @param[in] new_format The new audio format to set
         * @param[in] new_channels The new number of audio channels to set
         * @param[in] new_frequency The new audio frequency to set
         *
         * @see bitdepth, channels, frequency, bytes_per_sample
         */
        void initValues(AudioBuffer::SampleFormat new_format, int new_channels, int new_frequency);

        /**
         * @brief Initializes the AudioContext from configuration.
         *
         * Reads audio settings from the application configuration and sets up
         * the context parameters (output format, channels, frequency).
         * This should be called once at startup before creating sources or buffers.
         *
         * @see bitdepth, channels, frequency, bytes_per_sample
         */
        void init(void);

        /**
         * @brief Initializes the AudioContext from configuration or default values
         *
         * If configuration uses auto (0) values, we set the parameters to default
         * values. Used when we need to mix some audio samples, and the audio driver
         * did not configure output audio parameters.
         *
         * @see bitdepth, channels, frequency, bytes_per_sample
         */
        void initDefaults(void);

        /**
         * @brief Returns if the AudioContext parameters were set.
         *
         * Audio parameters may be set from the user configuration or may be auto, 
         * in which case they will be set by the default driver.
         *
         * @return true if the context parameters are initialized, false otherwise
         *
         * @see bitdepth, channels, frequency
         */
        bool isInited(void) const;

        /**
         * @brief Creates a new empty audio buffer.
         *
         * Allocates a new AudioBuffer object managed by this context. The buffer
         * is initially empty with default parameters and should be configured
         * before use. The buffer is added to the internal buffer list.
         *
         * @return Shared pointer to the new AudioBuffer, or nullptr if creation failed
         *
         * @see deleteBuffer(), reuseBufferFromSourceOrCreate()
         */
        std::shared_ptr<AudioBuffer> createBuffer(void);

        /**
         * @brief Reuses a buffer from a processed source or creates a new one.
         *
         * Attempts to recycle a buffer that has been fully processed and removed
         * from the given source. If available, returns that buffer with the same
         * format, channels, and frequency as the source. If no reusable buffer
         * exists, creates a new one.
         *
         * This reduces allocation overhead by reusing buffer objects.
         *
         * @param[in] source The audio source to retrieve a processed buffer from
         *
         * @return Shared pointer to a reusable or new AudioBuffer with matching
         *         source parameters, or nullptr if failed
         *
         * @see createBuffer(), deleteBuffer()
         */
        std::shared_ptr<AudioBuffer> reuseBufferFromSourceOrCreate(AudioSource* source);

        /**
         * @brief Deletes a buffer by its identifier.
         *
         * Removes the buffer with the given ID from the buffer list and optionally
         * moves it to the reuse pool for potential recycling. If the buffer is
         * currently in use by a source, this may cause undefined behavior.
         *
         * @param[in] id The buffer identifier to delete
         *
         * @see createBuffer(), isBuffer()
         */
        void deleteBuffer(int id);

        /**
         * @brief Checks if a buffer with the given ID exists.
         *
         * @param[in] id The buffer identifier to query
         *
         * @return true if a buffer with this ID exists, false otherwise
         *
         * @see getBuffer()
         */
        bool isBuffer(int id) const;

        /**
         * @brief Retrieves a buffer by its identifier.
         *
         * @param[in] id The buffer identifier to retrieve
         *
         * @return Shared pointer to the AudioBuffer with the given ID,
         *         or nullptr if not found
         *
         * @see isBuffer(), createBuffer()
         */
        std::shared_ptr<AudioBuffer> getBuffer(int id) const;

        /**
         * @brief Creates a new empty audio source.
         *
         * Allocates a new AudioSource object managed by this context. The source
         * is initially in SOURCE_INITIAL state with no buffers queued.
         * Configure the source before use (set frequency, format, channels).
         *
         * @return Shared pointer to the new AudioSource, or nullptr if creation failed
         *
         * @see deleteSource(), AudioSource
         */
        std::shared_ptr<AudioSource> createSource(void);

        /**
         * @brief Deletes a source by its identifier.
         *
         * Removes the source with the given ID from the source list and optionally
         * moves it to the reuse pool. Any buffers queued in this source are not
         * automatically deleted.
         *
         * @param[in] id The source identifier to delete
         *
         * @see createSource(), isSource()
         */
        void deleteSource(int id);

        /**
         * @brief Checks if a source with the given ID exists.
         *
         * @param[in] id The source identifier to query
         *
         * @return true if a source with this ID exists, false otherwise
         *
         * @see getSource()
         */
        bool isSource(int id) const;

        /**
         * @brief Retrieves a source by its identifier.
         *
         * @param[in] id The source identifier to retrieve
         *
         * @return Shared pointer to the AudioSource with the given ID,
         *         or nullptr if not found
         *
         * @see isSource(), createSource()
         */
        std::shared_ptr<AudioSource> getSource(int id) const;

        /**
         * @brief Mixes all playing sources for a specified time duration.
         *
         * Combines audio from all active sources into the mixed output buffer
         * (outSamples, outBytes). The number of samples to mix is derived from
         * the time duration. Only sources in SOURCE_PLAYING state are mixed.
         *
         * If playback is paused, this function has no effect.
         *
         * @param[in] ticks Time duration to mix, specified as a timespec structure
         *
         * @see mixAllSources(int), outSamples, paused, AudioSource::mixWith()
         */
        void mixAllSources(struct timespec ticks);

        /**
         * @brief Mixes all playing sources for a specified number of samples.
         *
         * Combines audio from all active sources into the mixed output buffer
         * (outSamples, outBytes). All sources in SOURCE_PLAYING state are mixed
         * to produce the specified number of output samples.
         *
         * If playback is paused, this function has no effect.
         *
         * @param[in] nbSamples Number of samples to generate from mixing
         *
         * @see mixAllSources(struct timespec), outSamples, paused
         */
        void mixAllSources(int nbSamples);

        /**
         * @brief Mutex protecting access to all audio objects.
         *
         * Synchronizes access to buffers, sources, and their pools across
         * multiple threads. Always acquire this mutex before accessing or
         * modifying any audio objects.
         *
         * @see buffer_queue, sources
         */
        std::mutex mutex;

        /**
         * @brief Thread ID of the audio mixing thread.
         *
         * Stores the thread ID of the thread that fills audio buffers.
         * Used for thread synchronization and debugging.
         */
        pthread_t audio_thread;

        /**
         * @brief Retrieves the list of all active buffers for debugging.
         *
         * @return Constant reference to the list of active buffers
         *
         * @see getSourceList()
         */
        const std::list<std::shared_ptr<AudioBuffer>> getBufferList() const {return buffers;}

        /**
         * @brief Retrieves the list of all active sources for debugging.
         *
         * @return Constant reference to the list of active sources
         *
         * @see getBufferList()
         */
        const std::list<std::shared_ptr<AudioSource>> getSourceList() const {return sources;}

    private:
        /**
         * @brief List of active audio buffers.
         * @internal
         */
        std::list<std::shared_ptr<AudioBuffer>> buffers;

        /**
         * @brief List of active audio sources.
         * @internal
         */
        std::list<std::shared_ptr<AudioSource>> sources;

        /**
         * @brief Pool of deleted buffers available for reuse.
         * @internal
         * Reduces allocation overhead by recycling buffer objects.
         */
        std::list<std::shared_ptr<AudioBuffer>> buffers_pool;

        /**
         * @brief Pool of deleted sources available for reuse.
         * @internal
         * Reduces allocation overhead by recycling source objects.
         */
        std::list<std::shared_ptr<AudioSource>> sources_pool;
};

}

#endif
