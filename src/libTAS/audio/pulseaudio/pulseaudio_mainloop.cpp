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

#include "pulseaudio_mainloop.h"
#include "../../logging.h"
#include "../../GlobalState.h"
#include "../../hook.h"
#include "../../threadwrappers.h" // isMainThread()

namespace libtas {

namespace orig {
    static pa_mainloop *(*pa_mainloop_new)(void);
    static void (*pa_mainloop_free)(pa_mainloop* m);
    static int (*pa_mainloop_iterate)(pa_mainloop *m, int block, int *retval);
    static pa_mainloop_api* (*pa_mainloop_get_api)(pa_mainloop *m);
}

pa_mainloop *pa_mainloop_new(void)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE(pa_mainloop_new, "libpulse.so");
        return orig::pa_mainloop_new();
    }

    DEBUGLOGCALL(LCF_SOUND);
    return reinterpret_cast<pa_mainloop *>(1);
}

void pa_mainloop_free(pa_mainloop* m)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE(pa_mainloop_free, "libpulse.so");
        return orig::pa_mainloop_free(m);
    }

    DEBUGLOGCALL(LCF_SOUND);
}

int pa_mainloop_iterate(pa_mainloop *m, int block, int *retval)
{
    if (GlobalState::isNative() || !isMainThread()) {
        /* Very hacky, but if this function is not called on the main thread,
         * it means it is running on the PulseAudio thread, so that we must
         * call the original function.
         */
        LINK_NAMESPACE(pa_mainloop_iterate, "libpulse.so");
        return orig::pa_mainloop_iterate(m, block, retval);
    }

    DEBUGLOGCALL(LCF_SOUND | LCF_TODO);
    return 0;
}

pa_mainloop_api* pa_mainloop_get_api(pa_mainloop *m)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE(pa_mainloop_get_api, "libpulse.so");
        return orig::pa_mainloop_get_api(m);
    }

    DEBUGLOGCALL(LCF_SOUND | LCF_TODO);
    static pa_mainloop_api api;
    return &api;
}

}
