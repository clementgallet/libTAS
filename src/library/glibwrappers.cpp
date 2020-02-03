/*
    Copyright 2015-2020 Clément Gallet <clement.gallet@ens-lyon.org>

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

#include "glibwrappers.h"
#include "logging.h"
#include "checkpoint/ThreadManager.h"
#include "DeterministicTimer.h"
#include "hook.h"

#include <errno.h>

namespace libtas {

DEFINE_ORIG_POINTER(g_cond_wait);
DEFINE_ORIG_POINTER(g_cond_wait_until);

/* Override */ void g_cond_wait (GCond *cond, GMutex *mutex)
{
    DEBUGLOGCALL(LCF_THREAD | LCF_WAIT);
    LINK_NAMESPACE(g_cond_wait, "glib-2.0");
    return orig::g_cond_wait(cond, mutex);
}

/* Override */ gboolean g_cond_wait_until(GCond *cond, GMutex *mutex, gint64 end_time)
{
    debuglog(LCF_THREAD | LCF_WAIT, __func__, " called with end_time ", end_time);
    LINK_NAMESPACE(g_cond_wait_until, "glib-2.0");

    /* If not main thread, do not change the behavior */
    if (!ThreadManager::isMainThread())
        return orig::g_cond_wait_until(cond, mutex, end_time);

    if (shared_config.wait_timeout == SharedConfig::WAIT_NATIVE)
        return orig::g_cond_wait_until(cond, mutex, end_time);

    TimeHolder now = detTimer.getTicks();

    if (shared_config.wait_timeout == SharedConfig::WAIT_FINITE) {
        /* Wait for 0.1 sec, arbitrary */
        gint64 new_end_time = (static_cast<gint64>(now.tv_sec) * 1000000) + (now.tv_nsec / 1000) + 100*1000;
        gboolean ret = orig::g_cond_wait_until(cond, mutex, new_end_time);
        if (ret)
            return ret;
    }

    if ((shared_config.wait_timeout == SharedConfig::WAIT_FULL_INFINITE) ||
        (shared_config.wait_timeout == SharedConfig::WAIT_FINITE)) {
        /* Transfer time to our deterministic timer */
        TimeHolder end;
        end.tv_sec = end_time / (1000*1000);
        end.tv_nsec = (end_time % (1000*1000)) * 1000;
        TimeHolder delay = end - now;
        detTimer.addDelay(delay);
    }

    if (shared_config.wait_timeout == SharedConfig::WAIT_FINITE) {
        /* Wait again for 0.1 sec, arbitrary */
        now = detTimer.getTicks();
        gint64 new_end_time = (static_cast<gint64>(now.tv_sec) * 1000000) + (now.tv_nsec / 1000) + 100*1000;
        return orig::g_cond_wait_until(cond, mutex, new_end_time);
    }

    /* Infinite wait */
    LINK_NAMESPACE(g_cond_wait, "glib-2.0");
    orig::g_cond_wait(cond, mutex);
    return 1;
}

}
