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

#ifndef LIBTAS_SIGNALWRAPPERS_H_INCL
#define LIBTAS_SIGNALWRAPPERS_H_INCL

#include "global.h"
#include <signal.h> // for all signal structs

namespace libtas {

/* Set the handler for the signal SIG to HANDLER, returning the old
   handler, or SIG_ERR on error.
   By default `signal' has the BSD semantic.  */
OVERRIDE sighandler_t signal (int sig, sighandler_t handler) throw();

/* None of the following functions should be used anymore.  They are here
   only for compatibility.  A single word (`int') is not guaranteed to be
   enough to hold a complete signal mask and therefore these functions
   simply do not work in many situations.  Use `sigprocmask' instead.  */

/* Block signals in MASK, returning the old mask.  */
OVERRIDE int sigblock (int mask) throw();

/* Set the mask of blocked signals to MASK, returning the old mask.  */
OVERRIDE int sigsetmask (int mask) throw();

/* Return currently selected signal mask.  */
OVERRIDE int siggetmask (void) throw();

/* Get and/or change the set of blocked signals.  */
OVERRIDE int sigprocmask (int how, const sigset_t *set,
            sigset_t *oset) throw();

/* Change the set of blocked signals to SET,
   wait until a signal arrives, and restore the set of blocked signals.

   This function is a cancellation point and therefore not marked with
   __THROW.  */
OVERRIDE int sigsuspend (const sigset_t *set);

/* Get and/or set the action for signal SIG.  */
OVERRIDE int sigaction (int sig, const struct sigaction *act,
              struct sigaction *oact) throw();

/* Put in SET all signals that are blocked and waiting to be delivered.  */
OVERRIDE int sigpending (sigset_t *set) throw();

/* Select any of pending signals from SET or wait for any to arrive.

   This function is a cancellation point and therefore not marked with
   __THROW.  */
OVERRIDE int sigwait (const sigset_t *set, int *sig);

/* Select any of pending signals from SET and place information in INFO.

   This function is a cancellation point and therefore not marked with
   __THROW.  */
OVERRIDE int sigwaitinfo (const sigset_t *set,
            siginfo_t *info);

/* Select any of pending signals from SET and place information in INFO.
   Wait the time specified by TIMEOUT if no signal is pending.

   This function is a cancellation point and therefore not marked with
   __THROW.  */
OVERRIDE int sigtimedwait (const sigset_t *set,
             siginfo_t *info, const struct timespec *timeout);

/* Alternate signal handler stack interface.
    This interface should always be preferred over `sigstack'.  */
OVERRIDE int sigaltstack (const stack_t *ss, stack_t *oss) throw();

/* Modify the signal mask for the calling thread.  The arguments have
   the same meaning as for sigprocmask(2). */
OVERRIDE int pthread_sigmask (int how, const sigset_t *newmask,
                sigset_t *oldmask) throw();

/* Send signal SIGNO to the given thread. */
OVERRIDE int pthread_kill (pthread_t threadid, int signo) throw();

/* Queue signal and data to a thread.  */
OVERRIDE int pthread_sigqueue (pthread_t threadid, int signo,
                 const union sigval value) throw();

}

#endif
