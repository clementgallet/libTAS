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

#ifndef LIBTAS_PULSEAUDIO_INTROSPECT_H_INCL
#define LIBTAS_PULSEAUDIO_INTROSPECT_H_INCL

#include "../../global.h"
#include "pulseaudio_simple.h"
#include "pulseaudio_context.h"
#include "pulseaudio_operation.h"

namespace libtas {

typedef uint32_t pa_volume_t;

/** A structure encapsulating a per-channel volume */
typedef struct pa_cvolume {
    uint8_t channels;                     /**< Number of channels */
    pa_volume_t values[PA_CHANNELS_MAX];  /**< Per-channel volume */
} pa_cvolume;


/** Stores information about sinks. Please note that this structure
 * can be extended as part of evolutionary API updates at any time in
 * any new release. */
typedef struct pa_sink_info {
    const char *name;                  /**< Name of the sink */
    uint32_t index;                    /**< Index of the sink */
    const char *description;           /**< Description of this sink */
    pa_sample_spec sample_spec;        /**< Sample spec of this sink */
    pa_channel_map channel_map;        /**< Channel map */
    uint32_t owner_module;             /**< Index of the owning module of this sink, or PA_INVALID_INDEX. */
    pa_cvolume volume;                 /**< Volume of the sink */
    int mute;                          /**< Mute switch of the sink */
    uint32_t monitor_source;           /**< Index of the monitor source connected to this sink. */
    const char *monitor_source_name;   /**< The name of the monitor source. */
    pa_usec_t latency;                 /**< Length of queued audio in the output buffer. */
    const char *driver;                /**< Driver name */
    // pa_sink_flags_t flags;             /**< Flags */
    // pa_proplist *proplist;             /**< Property list \since 0.9.11 */
    // pa_usec_t configured_latency;      /**< The latency this device has been configured to. \since 0.9.11 */
    // pa_volume_t base_volume;           /**< Some kind of "base" volume that refers to unamplified/unattenuated volume in the context of the output device. \since 0.9.15 */
    // pa_sink_state_t state;             /**< State \since 0.9.15 */
    // uint32_t n_volume_steps;           /**< Number of volume steps for sinks which do not support arbitrary volumes. \since 0.9.15 */
    // uint32_t card;                     /**< Card index, or PA_INVALID_INDEX. \since 0.9.15 */
    // uint32_t n_ports;                  /**< Number of entries in port array \since 0.9.16 */
    // pa_sink_port_info** ports;         /**< Array of available ports, or NULL. Array is terminated by an entry set to NULL. The number of entries is stored in n_ports. \since 0.9.16 */
    // pa_sink_port_info* active_port;    /**< Pointer to active port in the array, or NULL. \since 0.9.16 */
    // uint8_t n_formats;                 /**< Number of formats supported by the sink. \since 1.0 */
    // pa_format_info **formats;          /**< Array of formats supported by the sink. \since 1.0 */
} pa_sink_info;

/** Callback prototype for pa_context_get_sink_info_by_name() and friends */
typedef void (*pa_sink_info_cb_t)(pa_context *c, const pa_sink_info *i, int eol, void *userdata);

/** Get the complete sink list */
OVERRIDE pa_operation* pa_context_get_sink_info_list(pa_context *c, pa_sink_info_cb_t cb, void *userdata);

typedef void pa_source_info;

/** Callback prototype for pa_context_get_source_info_by_name() and friends */
typedef void (*pa_source_info_cb_t)(pa_context *c, const pa_source_info *i, int eol, void *userdata);

/** Get the complete source list */
OVERRIDE pa_operation* pa_context_get_source_info_list(pa_context *c, pa_source_info_cb_t cb, void *userdata);

}

#endif
