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

#include "ThreadInfo.h"
#include <sys/ptrace.h>
#include <errno.h>
#include <stdio.h>

void ThreadInfo::saveRegisters(void)
{
    long r;

    if (needattach) {
        /* Attach to thread */
        do {
            r = ptrace(PTRACE_ATTACH, tid, nullptr, nullptr);
        } while (r == -1L && (errno == EBUSY || errno == EFAULT || errno == ESRCH));

        if (r == -1L) {
            fprintf(stderr, "Could not attach to tid %d\n", tid);
        }
    }

    /* Copy thread registers */
    do {
        r = ptrace(PTRACE_GETREGS, tid, &regs, &regs);
    } while (r == -1L && errno == ESRCH);

    if (r == -1L) {
        fprintf(stderr, "Could not copy tid registers\n");
    }

    if (needattach) {
        /* Detach from thread */
        do {
            r = ptrace(PTRACE_DETACH, tid, nullptr, nullptr);
        } while (r == -1 && (errno == EBUSY || errno == EFAULT || errno == ESRCH));
    }
}

void ThreadInfo::loadRegisters(void)
{
    long r;

    if (needattach) {
        /* Attach to thread */
        do {
            r = ptrace(PTRACE_ATTACH, tid, nullptr, nullptr);
        } while (r == -1L && (errno == EBUSY || errno == EFAULT || errno == ESRCH));

        if (r == -1L) {
            fprintf(stderr, "Could not attach to tid %d\n", tid);
        }
    }

    /* Copy thread registers */
    do {
        r = ptrace(PTRACE_SETREGS, tid, &regs, &regs);
    } while (r == -1L && errno == ESRCH);

    if (r == -1L) {
        fprintf(stderr, "Could not load tid registers\n");
    }

    if (needattach) {
        /* Detach from thread */
        do {
            r = ptrace(PTRACE_DETACH, tid, nullptr, nullptr);
        } while (r == -1 && (errno == EBUSY || errno == EFAULT || errno == ESRCH));
    }
}

