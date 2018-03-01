/*
    Copyright 2015-2018 Clément Gallet <clement.gallet@ens-lyon.org>

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

#include "pulseaudio_context.h"
#include "../../logging.h"
#include "../../GlobalState.h"
#include "../../hook.h"

namespace libtas {

DEFINE_ORIG_POINTER(pa_context_new);
DEFINE_ORIG_POINTER(pa_context_unref);
DEFINE_ORIG_POINTER(pa_context_get_state);
DEFINE_ORIG_POINTER(pa_context_connect);
DEFINE_ORIG_POINTER(pa_context_disconnect);

DEFINE_ORIG_POINTER(pa_context_set_state_callback);
DEFINE_ORIG_POINTER(pa_context_set_event_callback);

pa_context *pa_context_new(pa_mainloop_api *mainloop, const char *name)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE(pa_context_new, "libpulse.so");
        return orig::pa_context_new(mainloop, name);
    }

    DEBUGLOGCALL(LCF_SOUND);
    return reinterpret_cast<pa_context *>(1);
}

void pa_context_unref(pa_context *c)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE(pa_context_unref, "libpulse.so");
        return orig::pa_context_unref(c);
    }

    DEBUGLOGCALL(LCF_SOUND);
}

void pa_context_set_state_callback(pa_context *c, pa_context_notify_cb_t cb, void *userdata)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE(pa_context_set_state_callback, "libpulse.so");
        return orig::pa_context_set_state_callback(c, cb, userdata);
    }

    DEBUGLOGCALL(LCF_SOUND);
}

void pa_context_set_event_callback(pa_context *p, pa_context_event_cb_t cb, void *userdata)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE(pa_context_set_event_callback, "libpulse.so");
        return orig::pa_context_set_event_callback(p, cb, userdata);
    }

    DEBUGLOGCALL(LCF_SOUND);
}

pa_context_state_t pa_context_get_state(pa_context *c)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE(pa_context_get_state, "libpulse.so");
        return orig::pa_context_get_state(c);
    }

    DEBUGLOGCALL(LCF_SOUND | LCF_TODO);
    return PA_CONTEXT_READY;
}

int pa_context_connect(pa_context *c, const char *server, pa_context_flags_t flags, /*const pa_spawn_api*/void *api)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE(pa_context_connect, "libpulse.so");
        return orig::pa_context_connect(c, server, flags, api);
    }

    DEBUGLOGCALL(LCF_SOUND | LCF_TODO);
    return 0;
}

void pa_context_disconnect(pa_context *c)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE(pa_context_disconnect, "libpulse.so");
        return orig::pa_context_disconnect(c);
    }

    DEBUGLOGCALL(LCF_SOUND | LCF_TODO);
}

}
