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

#ifndef LIBTAS_SYSTEM_H_INCL
#define LIBTAS_SYSTEM_H_INCL

#include "hook.h"

#include <unistd.h>
#include <sched.h>

namespace libtas {

/* Get the process ID of the calling process.  */
OVERRIDE pid_t getpid (void) __THROWNL;

/* Clone the calling process, creating an exact copy.
   Return -1 for errors, 0 to the new process,
   and the process ID of the new process to the old process.  */
OVERRIDE pid_t fork(void) __THROWNL;

/* Get index of currently used CPU.  */
OVERRIDE int sched_getcpu (void) __THROW;

/* Get the CPU affinity for a task */
OVERRIDE int sched_getaffinity (pid_t pid, size_t cpusetsize, cpu_set_t *cpuset) __THROW;

/* Get the value of the system variable NAME.  */
OVERRIDE long int sysconf (int name) __THROW;


}

#endif
