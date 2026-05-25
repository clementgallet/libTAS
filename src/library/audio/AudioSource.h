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

#ifndef LIBTAS_AUDIOSOURCE_H_INCL
#define LIBTAS_AUDIOSOURCE_H_INCL

#include <vector>
#include <memory>
#include <functional>
#include "AudioConverter.h"

namespace libtas {

/**
 * @class AudioSource
 * @brief Controller for audio playback from one or more audio buffers.
 *
 * AudioSource manages the playback of audio data from one or more AudioBuffer objects.
 * It provides multiple playback modes (static, streaming, callback-based), handles
 * per-source volume and pitch control, manages buffer queuing, and coordinates
 * resampling from source format to output format.
 *
 * Audio data flows through AudioSource as follows:
 * 1. Buffers are queued via queueBuffer()
 * 2. Playback state is controlled (play, pause, stop, rewind)
 * 3. During mixing, mixWith() reads from the queue and resamples as needed
 * 4. Processed buffers can be retrieved via reuseBuffer() for reuse
 *
 * The source maintains its own buffer queue, playback position, and resampling context.
 * Multiple sources are mixed together by AudioContext.
 *
 * @see AudioBuffer for audio data representation
 * @see AudioContext for source lifecycle and mixing coordination
 * @see AudioConverter for resampling
 */
class AudioSource
{
    public:
        /**
         * @brief Constructs a new AudioSource.
         *
         * Initializes the source with default parameters. Call init() to set up
         * the source for use.
         */
        AudioSource();

        /**
         * @brief Unique identifier for this source.
         *
         * Used by AudioContext and audio driver code to reference this source
         * across the system. Assigned by AudioContext when source is created.
         */
        int id;

        /**
         * @brief Audio format of buffers queued in this source.
         *
         * All buffers queued to this source must have matching format.
         * Cannot be changed while source is playing. Set via init() or
         * by directly configuring the source before queuing buffers.
         *
         * @see AudioBuffer::SampleFormat, channels, frequency, init()
         */
        AudioBuffer::SampleFormat format;

        /**
         * @brief Number of audio channels in queued buffers.
         *
         * All buffers queued to this source must have matching channel count.
         * Typical values: 1 (mono), 2 (stereo).
         * Cannot be changed while source is playing.
         *
         * @see format, frequency, init()
         */
        int channels;

        /**
         * @brief Sample rate of queued buffers in Hz.
         *
         * All buffers queued to this source must have matching frequency.
         * Cannot be changed while source is playing. If source frequency differs
         * from output frequency, automatic resampling is performed during mixing.
         *
         * @see format, channels, init()
         */
        int frequency;

        /**
         * @brief Current playback position within the buffer queue in samples.
         *
         * Indicates where in the currently playing buffer(s) playback has advanced.
         * Measured in samples from the start of the first buffer in the queue.
         * Set via setPosition(), read via getPosition().
         *
         * @see setPosition(), getPosition(), samples_frac
         */
        int position;

        /**
         * @brief Fractional part of the playback position.
         *
         * When pitch != 1.0, the number of samples read per output sample
         * may not be an integer. This variable tracks the fractional part
         * of the position to maintain accuracy across multiple mixing operations.
         *
         * Example: if pitch=1.5, reading 100 samples moves position by 150.5,
         * so position advances by 150 and samples_frac accumulates 0.5.
         *
         * @see position, pitch
         */
        int64_t samples_frac;

        /**
         * @brief Playback volume for this source.
         *
         * Multiplier applied to samples from this source before mixing.
         * Values greater than 1.0 amplify the signal. Output is typically
         * clamped to prevent overflow.
         *
         * Range: [0.0, +inf), typical 0.0 to 1.0
         * Default: 1.0
         *
         * @see volume, AudioContext::outVolume for master volume
         */
        float volume;

        /**
         * @brief Enumeration of audio source types.
         *
         * Defines how buffers are queued and played back.
         */
        enum SourceType {
            /**
             * @brief Source type not yet determined.
             *
             * Initial state before configuration.
             */
            SOURCE_UNDETERMINED,

            /**
             * @brief Static source with single buffer.
             *
             * Contains one audio buffer. Playback stops at the end, or loops
             * if looping is enabled. Used by OpenAL-style sources, typically for
             * sound effects and short audio. The audio driver provides the entire
             * buffer at once during setup.
             *
             * @see looping, SOURCE_STREAMING
             */
            SOURCE_STATIC,

            /**
             * @brief Streaming source with queue of buffers.
             *
             * Contains a queue of buffers. When current buffer finishes,
             * playback moves to next buffer. Fully-read buffers can be dequeued
             * via reuseBuffer() and new buffers queued via queueBuffer().
             * Source stops when queue is empty.
             *
             * Used by OpenAL, usually for music.
             *
             * @see SOURCE_STREAMING_CONTINUOUS
             */
            SOURCE_STREAMING,

            /**
             * @brief Continuous streaming source.
             *
             * Like SOURCE_STREAMING, but does not stop when queue is empty.
             * Instead, waits for new data to be pushed. Can call a callback
             * function to allow the audio driver to push more buffers.
             *
             * Used by SDL and ALSA.
             *
             * @see SOURCE_STREAMING, SOURCE_CALLBACK
             */
            SOURCE_STREAMING_CONTINUOUS,

            /**
             * @brief Callback-based source.
             *
             * Contains one buffer. When playback reaches the end, the callback
             * function is invoked to allow the audio driver to refill the buffer.
             * 
             * Used by SDL and cubeb.
             *
             * @see callback
             */
            SOURCE_CALLBACK,
        };

        /**
         * @brief Type of this audio source.
         *
         * Determines how buffers are managed and played back.
         *
         * @see SourceType, init()
         */
        SourceType source;

        /**
         * @brief Whether this source loops at the end.
         *
         * If true, playback loops back to the beginning when reaching the end.
         * Only meaningful for SOURCE_STATIC sources. For SOURCE_STREAMING and
         * SOURCE_STREAMING_CONTINUOUS, behavior depends on buffer availability.
         *
         * @see loop_point_beg, loop_point_end in AudioBuffer
         */
        bool looping;

        /**
         * @brief Pitch/tempo multiplier for this source.
         *
         * Multiplier applied to the playback rate. Values > 1.0 play faster
         * (higher pitch), values < 1.0 play slower (lower pitch).
         *
         * Implementation: affects how many input samples are read per output sample.
         * A pitch of 2.0 reads twice as many samples per output sample, effectively
         * doubling playback speed.
         *
         * Range: (0.0, +inf), typical 0.5 to 2.0
         * Default: 1.0
         *
         * Note: Resampling is required to produce correct output.
         *
         * @see audioConverter, samples_frac
         */
        float pitch;

        /**
         * @brief Enumeration of audio source playback states.
         *
         * Defines the current playback state of the source.
         */
        enum SourceState {
            /**
             * @brief Source has been created but not configured.
             *
             * Initial state. Transition to PREPARED after configuration completed
             */
            SOURCE_INITIAL,

            /**
             * @brief Source is ready to play but not currently playing.
             *
             * Mostly used by ALSA.
             * Can transition to PLAYING by calling play or starting playback.
             */
            SOURCE_PREPARED,

            /**
             * @brief Source is actively playing.
             *
             * Buffers are being read and mixed into the output.
             * Position advances with each audio frame.
             *
             * @see mixWith()
             */
            SOURCE_PLAYING,

            /**
             * @brief Source has stopped and will not play further.
             *
             * Playback reached the end without looping, or was explicitly stopped.
             * Position is retained for potential resume or seeking.
             */
            SOURCE_STOPPED,

            /**
             * @brief Source is paused and will resume from current position.
             *
             * Playback is temporarily suspended. Position is retained.
             * Can resume from the same position by transitioning back to PLAYING.
             */
            SOURCE_PAUSED,

            /**
             * @brief Buffer underrun condition.
             *
             * More audio was requested than is available. Source has insufficient
             * data to fill the requested output. State typically transitions to
             * STOPPED or PLAYING depending on recovery.
             */
            SOURCE_UNDERRUN,
        };

        /**
         * @brief Current playback state of this source.
         *
         * Indicates the current state of playback. Control playback state
         * through appropriate methods like init(), rewind(), etc.
         *
         * @see SourceState
         */
        SourceState state;

        /**
         * @brief Callback function for refilling buffer in SOURCE_CALLBACK mode.
         *
         * In SOURCE_CALLBACK mode, when the source finishes reading the buffer,
         * this callback is invoked to allow the audio driver to refill the buffer
         * with new audio data. The callback receives a reference to the buffer to fill.
         *
         * Function signature: void callback(AudioBuffer& buf)
         *
         * @see callback_data, SOURCE_CALLBACK
         */
        std::function<void(AudioBuffer&)> callback;

        /**
         * @brief User data pointer passed to callback function.
         *
         * Opaque pointer available to the callback function via callback_data.
         * Can store any state needed by the callback.
         *
         * @see callback
         */
        void* callback_data;

        /**
         * @brief Returns the number of bytes per audio frame.
         *
         * Computed as: (bitDepth / 8) * channels
         * This is the size of one audio frame across all channels.
         *
         * Example: stereo 16-bit audio returns 4 bytes per frame.
         *
         * @return Bytes per audio frame, or 0 if format is invalid
         *
         * @see format, channels, frequency
         */
        int frameToByteRatio();

        /**
         * @brief Converts a time duration to a number of audio samples.
         *
         * Given a time duration as a timespec structure and a sample frequency,
         * calculates the number of audio samples (frames) that would be read
         * during that time period.
         *
         * Useful for converting timing information into sample-based offsets.
         *
         * @param[in] ticks Time duration as timespec (seconds + nanoseconds)
         * @param[in] frequency Sample rate in Hz
         *
         * @return Number of samples for the given duration at the given frequency
         *
         * @see position, mixWith()
         */
        int ticksToSamples(struct timespec ticks, int frequency);

        /**
         * @brief Initializes the source for use.
         *
         * Sets up the source with default configuration values (format, channels,
         * frequency, source type, etc.) and clears buffers. Automatically called 
         * on source creation, but can be called again when reusing a source.
         *
         * @see format, channels, frequency, source
         */
        void init();

        /**
         * @brief Rewinds playback to the beginning of the first buffer.
         *
         * Resets the playback position to the start of the buffer queue.
         * Does not change playback state; typically used before resuming playback
         * or when seeking to the beginning.
         *
         * @see position, setPosition()
         */
        void rewind();

        /**
         * @brief Marks source parameters as changed, requiring resampler reinitialization.
         *
         * Signals that audio parameters (frequency, channels, or format) may have
         * changed. The resampler will reinitialize on the next mixing operation.
         * Use this after modifying format, channels, or frequency.
         *
         * @see format, channels, frequency, audioConverter
         */
        void dirty();

        /**
         * @brief Returns the number of buffers currently queued.
         *
         * @return The number of AudioBuffer objects in the queue
         *
         * @see nbQueueProcessed(), queueSize(), buffer()
         */
        int nbQueue() const;

        /**
         * @brief Returns the number of queued buffers that have been fully processed.
         *
         * Processed buffers are those that have been completely read through and
         * can be safely dequeued and reused. This is useful for the audio driver
         * to know when buffers are available for recycling.
         *
         * @return Number of buffers in queue that have been fully read
         *
         * @see reuseBuffer(), nbQueue()
         */
        int nbQueueProcessed() const;

        /**
         * @brief Retrieves a buffer from the queue by index.
         *
         * @param[in] index Index into the buffer queue (0 = first buffer)
         *
         * @return Shared pointer to the buffer at the given index, or nullptr if index is invalid
         *
         * @see nbQueue(), queueBuffer()
         */
        const std::shared_ptr<AudioBuffer> buffer(int index) const;

        /**
         * @brief Returns the total size of all queued buffers in samples.
         *
         * Sums the sample size (number of frames) of all buffers currently in
         * the queue. Useful for understanding available playback time remaining.
         *
         * @return Total number of samples across all queued buffers
         *
         * @see nbQueue(), getPosition()
         */
        int queueSize() const;

        /**
         * @brief Gets the current playback position within the buffer queue.
         *
         * Returns the position relative to the beginning of the first buffer.
         * Measured in audio samples (frames).
         *
         * @return Current playback position in samples
         *
         * @see setPosition(), position
         */
        int getPosition() const;

        /**
         * @brief Sets the playback position within the buffer queue.
         *
         * Seeks to a new position relative to the beginning of the queue.
         *
         * @param[in] pos New playback position in samples, relative to queue start
         *
         * @pre pos >= 0
         *
         * @see getPosition(), position, queueSize()
         */
        void setPosition(int pos);

        /**
         * @brief Adds a buffer to the playback queue.
         *
         * Queues an audio buffer for playback, but only if its format, channels,
         * and frequency match the source configuration. Used for streaming sources
         * to push new data as it becomes available.
         *
         * Returns error code indicating success or reason for failure.
         *
         * @param[in] buffer Shared pointer to the AudioBuffer to queue
         *
         * @return 0 on success, or error code if buffer specs don't match source specs
         *
         * @see unqueueBuffer(), reuseBuffer(), clearBuffers()
         */
        int queueBuffer(std::shared_ptr<AudioBuffer> buffer);

        /**
         * @brief Removes and returns the first buffer from the queue.
         *
         * Dequeues the first (oldest) buffer in the queue regardless of whether
         * it has been fully processed.
         *
         * @return The ID of the dequeued buffer, or -1 if queue is empty
         *
         * @see queueBuffer(), reuseBuffer()
         */
        int unqueueBuffer();

        /**
         * @brief Retrieves and removes a fully processed buffer from the queue.
         *
         * Removes the first buffer only if playback has read through it entirely.
         * Useful for the audio subsystem to recover buffers that are no longer
         * needed, allowing reuse or deallocation.
         *
         * @return Shared pointer to a processed buffer, or nullptr if the first
         *         buffer has not been fully read or the queue is empty
         *
         * @see unqueueBuffer(), queueBuffer(), AudioContext::reuseBufferFromSourceOrCreate()
         */
        std::shared_ptr<AudioBuffer> reuseBuffer();

        /**
         * @brief Clears all queued buffers and resets playback position.
         *
         * Removes all buffers from the queue and resets position to 0.
         * Useful when stopping playback and needing to clean up.
         *
         * @see queueBuffer()
         */
        void clearBuffers();

        /**
         * @brief Checks if source will output some samples.
         *
         * Checks if source is playing, and audio samples are ready to be mixed out.
         * buffer queue) if the given number of ticks are played. Used for predicting
         * overrun during mixing, and waiting a bit to give the game a chance to push
         * back audio data. 
         *
         * @return true if source will output samples, false otherwise
         *
         * @see position, queueSize(), state
         */
        bool willOutput() const;

        /**
         * @brief Checks if playback will reach the end during a given time interval.
         *
         * Predicts whether the source will finish playing (reach the end of the
         * buffer queue) if the given number of ticks are played. Used for predicting
         * overrun during mixing, and waiting a bit to give the game a chance to push
         * back audio data. 
         *
         * @param[in] ticks Time duration to check
         *
         * @return true if source will end during this time period, false otherwise
         *
         * @see position, queueSize(), ticksToSamples()
         */
        bool willEnd(struct timespec ticks) const;

        /**
         * @brief Mixes this source's audio into an external output buffer.
         *
         * Reads audio from the queued buffers, applies resampling if needed,
         * applies per-source volume, and mixes into the provided output buffer.
         * Advances the playback position based on the duration specified.
         *
         * This is the core mixing operation that AudioContext calls for each
         * active source during each frame.
         *
         * Process:
         * 1. Determine samples to read based on ticks and output frequency
         * 2. Read samples from current buffer position
         * 3. Resample from source format to output format if needed
         * 4. Apply per-source volume
         * 5. Mix into output buffer (add to existing values)
         * 6. Update position, handling buffer queue transitions
         *
         * @param[in] ticks Duration to mix (in timespec format)
         * @param[out] samples_data_out Output buffer to mix into (must be pre-allocated)
         * @param[in] samples_byte_size_out Size of output buffer in bytes
         * @param[in] format_out Format of output
         * @param[in] channels_out Number of channels in output
         * @param[in] frequency_out Sample rate of output
         * @param[in] volume_out Master volume to apply
         *
         * @return Number of samples written to output buffer
         *
         * @note Output buffer is modified in-place (mixed with existing data, not replaced)
         *
         * @see AudioContext::mixAllSources(), volume, pitch, audioConverter
         */
        int mixWith( struct timespec ticks, uint8_t* samples_data_out, int samples_byte_size_out, AudioBuffer::SampleFormat format_out, int channels_out, int frequency_out, float volume_out);

    private:
        /**
         * @brief Queue of audio buffers to be played sequentially.
         * @internal
         */
        std::vector<std::shared_ptr<AudioBuffer>> buffer_queue;

        /**
         * @brief Index of the currently playing buffer in the queue.
         * @internal
         */
        int queue_index;

        /**
         * @brief Audio converter for resampling from source to output format.
         * @internal
         * Created lazily on first use. Resamples source audio if source frequency
         * or format differs from output frequency or format.
         */
        std::unique_ptr<AudioConverter> audio_converter;

        /**
         * @brief Temporary buffer for mixed source samples.
         * @internal
         * Stores resampled samples before mixing into the final output buffer.
         * Used to apply per-source processing before final mixing.
         */
        std::vector<uint8_t> mixed_samples;


};
}

#endif
