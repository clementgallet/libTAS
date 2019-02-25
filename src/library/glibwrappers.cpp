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

#include "glibwrappers.h"
#include "logging.h"
#include "checkpoint/ThreadManager.h"
#include "DeterministicTimer.h"
#include "hook.h"

#include <errno.h>

namespace libtas {

DEFINE_ORIG_POINTER(g_cond_wait_until);

/* Override */ gboolean g_cond_wait_until(GCond *cond, GMutex *mutex, gint64 end_time)
{
    debuglog(LCF_THREAD, __func__, " called with end_time ", end_time);
    LINK_NAMESPACE(g_cond_wait_until, "glib-2.0");

    /* If not main thread, do not change the behavior */
    if (!ThreadManager::isMainThread()) {
        return orig::g_cond_wait_until(cond, mutex, end_time);
    }

    return orig::g_cond_wait_until(cond, mutex, end_time);
}

}
