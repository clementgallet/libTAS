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

#ifndef LIBTAS_GLIB_H_INCL
#define LIBTAS_GLIB_H_INCL

#include "global.h"

namespace libtas {

/* For now, we define the glib types ourselves */
typedef int gboolean;
typedef void GCond;
typedef void GMutex;
typedef int64_t gint64;

OVERRIDE void g_cond_wait (GCond *cond, GMutex *mutex);
OVERRIDE gboolean g_cond_wait_until(GCond *cond, GMutex *mutex, gint64 end_time);

}

#endif
