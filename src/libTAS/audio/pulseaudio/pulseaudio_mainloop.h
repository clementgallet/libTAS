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

#ifndef LIBTAS_PULSEAUDIO_MAINLOOP_H_INCL
#define LIBTAS_PULSEAUDIO_MAINLOOP_H_INCL

#include "../../global.h"
#include "pulseaudio_mainloop_api.h"

namespace libtas {

/** An opaque main loop object */
typedef void pa_mainloop;

/** Allocate a new main loop object */
OVERRIDE pa_mainloop *pa_mainloop_new(void);

/** Free a main loop object */
OVERRIDE void pa_mainloop_free(pa_mainloop* m);

/** Run a single iteration of the main loop. This is a convenience function
for pa_mainloop_prepare(), pa_mainloop_poll() and pa_mainloop_dispatch().
Returns a negative value on error or exit request. If block is nonzero,
block for events if none are queued. Optionally return the return value as
specified with the main loop's quit() routine in the integer variable retval points
to. On success returns the number of sources dispatched in this iteration. */
OVERRIDE int pa_mainloop_iterate(pa_mainloop *m, int block, int *retval);

/** Run unlimited iterations of the main loop object until the main loop's quit() routine is called. */
OVERRIDE int pa_mainloop_run(pa_mainloop *m, int *retval);


/** Return the abstract main loop abstraction layer vtable for this
    main loop. No need to free the API as it is owned by the loop
    and is destroyed when the loop is freed. */
OVERRIDE pa_mainloop_api* pa_mainloop_get_api(pa_mainloop *m);

/** Interrupt a running poll (for threaded systems) */
OVERRIDE void pa_mainloop_wakeup(pa_mainloop *m);

/** Generic prototype of a poll() like function */
typedef int (*pa_poll_func)(struct pollfd *ufds, unsigned long nfds, int timeout, void*userdata);

/** Change the poll() implementation */
OVERRIDE void pa_mainloop_set_poll_func(pa_mainloop *m, pa_poll_func poll_func, void *userdata);


}

#endif
