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

#ifndef LIBTAS_PULSEAUDIOSIMPLE_H_INCL
#define LIBTAS_PULSEAUDIOSIMPLE_H_INCL

#include "../global.h"

/*** OpenAL functions ***/
namespace libtas {

typedef uint64_t pa_usec_t;
typedef void pa_simple;

/** The direction of a pa_stream object */
typedef enum pa_stream_direction {
    PA_STREAM_NODIRECTION,   /**< Invalid direction */
    PA_STREAM_PLAYBACK,      /**< Playback stream */
    PA_STREAM_RECORD,        /**< Record stream */
    PA_STREAM_UPLOAD         /**< Sample upload stream */
} pa_stream_direction_t;


/** Sample format */
typedef enum pa_sample_format {
    PA_SAMPLE_U8,
    /**< Unsigned 8 Bit PCM */

    PA_SAMPLE_ALAW,
    /**< 8 Bit a-Law */

    PA_SAMPLE_ULAW,
    /**< 8 Bit mu-Law */

    PA_SAMPLE_S16LE,
    /**< Signed 16 Bit PCM, little endian (PC) */

    PA_SAMPLE_S16BE,
    /**< Signed 16 Bit PCM, big endian */

    PA_SAMPLE_FLOAT32LE,
    /**< 32 Bit IEEE floating point, little endian (PC), range -1.0 to 1.0 */

    PA_SAMPLE_FLOAT32BE,
    /**< 32 Bit IEEE floating point, big endian, range -1.0 to 1.0 */

    PA_SAMPLE_S32LE,
    /**< Signed 32 Bit PCM, little endian (PC) */

    PA_SAMPLE_S32BE,
    /**< Signed 32 Bit PCM, big endian */

    PA_SAMPLE_S24LE,
    /**< Signed 24 Bit PCM packed, little endian (PC). \since 0.9.15 */

    PA_SAMPLE_S24BE,
    /**< Signed 24 Bit PCM packed, big endian. \since 0.9.15 */

    PA_SAMPLE_S24_32LE,
    /**< Signed 24 Bit PCM in LSB of 32 Bit words, little endian (PC). \since 0.9.15 */

    PA_SAMPLE_S24_32BE,
    /**< Signed 24 Bit PCM in LSB of 32 Bit words, big endian. \since 0.9.15 */

    PA_SAMPLE_MAX,
    /**< Upper limit of valid sample types */

    PA_SAMPLE_INVALID = -1
    /**< An invalid value */
} pa_sample_format_t;

/** A sample format and attribute specification */
typedef struct pa_sample_spec {
    pa_sample_format_t format;
    /**< The sample format */

    uint32_t rate;
    /**< The sample rate. (e.g. 44100) */

    uint8_t channels;
    /**< Audio channels. (1 for mono, 2 for stereo, ...) */
} pa_sample_spec;

typedef void pa_channel_map;
typedef void pa_buffer_attr;

/** Create a new connection to the server. */
OVERRIDE pa_simple* pa_simple_new(
    const char *server,                 /**< Server name, or NULL for default */
    const char *name,                   /**< A descriptive name for this client (application name, ...) */
    pa_stream_direction_t dir,          /**< Open this stream for recording or playback? */
    const char *dev,                    /**< Sink (resp. source) name, or NULL for default */
    const char *stream_name,            /**< A descriptive name for this stream (application name, song title, ...) */
    const pa_sample_spec *ss,           /**< The sample type to use */
    const pa_channel_map *map,          /**< The channel map to use, or NULL for default */
    const pa_buffer_attr *attr,         /**< Buffering attributes, or NULL for default */
    int *error                          /**< A pointer where the error code is stored when the routine returns NULL. It is OK to pass NULL here. */
);

/** Close and free the connection to the server. The connection object becomes invalid when this is called. */
OVERRIDE void pa_simple_free(pa_simple *s);

/** Write some data to the server. */
OVERRIDE int pa_simple_write(pa_simple *s, const void *data, size_t bytes, int *error);

/** Wait until all data already written is played by the daemon. */
OVERRIDE int pa_simple_drain(pa_simple *s, int *error);

/** Read some data from the server. This function blocks until \a bytes amount
 * of data has been received from the server, or until an error occurs.
 * Returns a negative value on failure. */
OVERRIDE int pa_simple_read(
    pa_simple *s, /**< The connection object. */
    void *data,   /**< A pointer to a buffer. */
    size_t bytes, /**< The number of bytes to read. */
    int *error
    /**< A pointer where the error code is stored when the function returns
     * a negative value. It is OK to pass NULL here. */
    );

/** Return the playback or record latency. */
OVERRIDE pa_usec_t pa_simple_get_latency(pa_simple *s, int *error);

/** Flush the playback or record buffer. This discards any audio in the buffer. */
OVERRIDE int pa_simple_flush(pa_simple *s, int *error);

}

#endif
