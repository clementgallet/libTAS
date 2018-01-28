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

#ifndef LIBTAS_PULSEAUDIO_CONTEXT_H_INCL
#define LIBTAS_PULSEAUDIO_CONTEXT_H_INCL

#include "../../global.h"
#include "pulseaudio_mainloop_api.h"

namespace libtas {

/** The state of a connection context */
typedef enum pa_context_state {
    PA_CONTEXT_UNCONNECTED,    /**< The context hasn't been connected yet */
    PA_CONTEXT_CONNECTING,     /**< A connection is being established */
    PA_CONTEXT_AUTHORIZING,    /**< The client is authorizing itself to the daemon */
    PA_CONTEXT_SETTING_NAME,   /**< The client is passing its application name to the daemon */
    PA_CONTEXT_READY,          /**< The connection is established, the context is ready to execute operations */
    PA_CONTEXT_FAILED,         /**< The connection failed or was disconnected */
    PA_CONTEXT_TERMINATED      /**< The connection was terminated cleanly */
} pa_context_state_t;

/** Some special flags for contexts. */
typedef enum pa_context_flags {
    PA_CONTEXT_NOFLAGS = 0x0000U,
    /**< Flag to pass when no specific options are needed (used to avoid casting)  \since 0.9.19 */
    PA_CONTEXT_NOAUTOSPAWN = 0x0001U,
    /**< Disabled autospawning of the PulseAudio daemon if required */
    PA_CONTEXT_NOFAIL = 0x0002U
    /**< Don't fail if the daemon is not available when pa_context_connect() is called, instead enter PA_CONTEXT_CONNECTING state and wait for the daemon to appear.  \since 0.9.15 */
} pa_context_flags_t;

/** An opaque connection context to a daemon */
typedef void pa_context;

/** Instantiate a new connection context with an abstract mainloop API
 * and an application name. It is recommended to use pa_context_new_with_proplist()
 * instead and specify some initial properties.*/
OVERRIDE pa_context *pa_context_new(pa_mainloop_api *mainloop, const char *name);

/** Decrease the reference counter of the context by one */
OVERRIDE void pa_context_unref(pa_context *c);

/** Generic notification callback prototype */
typedef void (*pa_context_notify_cb_t)(pa_context *c, void *userdata);

/** Set a callback function that is called whenever the context status changes */
OVERRIDE void pa_context_set_state_callback(pa_context *c, pa_context_notify_cb_t cb, void *userdata);

/** A property list object. Basically a dictionary with ASCII strings
 * as keys and arbitrary data as values. \since 0.9.11 */
typedef struct pa_proplist pa_proplist;

/** A callback for asynchronous meta/policy event messages. The set
 * of defined events can be extended at any time. Also, server modules
 * may introduce additional message types so make sure that your
 * callback function ignores messages it doesn't know. \since
 * 0.9.15 */
typedef void (*pa_context_event_cb_t)(pa_context *c, const char *name, pa_proplist *p, void *userdata);

/** Set a callback function that is called whenever a meta/policy
 * control event is received. \since 0.9.15 */
OVERRIDE void pa_context_set_event_callback(pa_context *p, pa_context_event_cb_t cb, void *userdata);

/** Return the current context status */
OVERRIDE pa_context_state_t pa_context_get_state(pa_context *c);

/** Connect the context to the specified server. If server is NULL,
connect to the default server. This routine may but will not always
return synchronously on error. Use pa_context_set_state_callback() to
be notified when the connection is established. If flags doesn't have
PA_CONTEXT_NOAUTOSPAWN set and no specific server is specified or
accessible a new daemon is spawned. If api is non-NULL, the functions
specified in the structure are used when forking a new child
process. */
OVERRIDE int pa_context_connect(pa_context *c, const char *server, pa_context_flags_t flags, /*const pa_spawn_api*/void *api);

/** Terminate the context connection immediately */
OVERRIDE void pa_context_disconnect(pa_context *c);

}

#endif
