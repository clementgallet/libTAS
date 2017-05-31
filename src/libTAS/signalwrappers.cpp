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

#include "signalwrappers.h"
#include "logging.h"
#include "GlobalState.h"
#include "hook.h"

namespace orig {
    static sighandler_t (*signal) (int sig, sighandler_t handler) throw();
    static int (*sigblock) (int mask) throw();
    static int (*sigsetmask) (int mask) throw();
    static int (*siggetmask) (void) throw();
    static int (*sigprocmask) (int how, const sigset_t *set,
                sigset_t *oset) throw();
    static int (*sigsuspend) (const sigset_t *set);
    static int (*sigaction) (int sig, const struct sigaction *act,
                  struct sigaction *oact) throw();
    static int (*sigpending) (sigset_t *set) throw();
    static int (*sigwait) (const sigset_t *set, int *sig);
    static int (*sigwaitinfo) (const sigset_t *set,
                siginfo_t *info);
    static int (*sigtimedwait) (const sigset_t *set,
                 siginfo_t *info, const struct timespec *timeout);
    static int (*pthread_sigmask) (int how, const sigset_t *newmask,
                 sigset_t *oldmask) throw();
    static int (*pthread_kill) (pthread_t threadid, int signo) throw();
    static int (*pthread_sigqueue) (pthread_t threadid, int signo,
                  const union sigval value) throw();

}

/* Override */ sighandler_t signal (int sig, sighandler_t handler) throw()
{
    DEBUGLOGCALL(LCF_SIGNAL);
    LINK_NAMESPACE(signal, nullptr);
    return orig::signal(sig, handler);
}

/* Override */ int sigblock (int mask) throw()
{
    DEBUGLOGCALL(LCF_SIGNAL);
    LINK_NAMESPACE(sigblock, nullptr);
    return orig::sigblock(mask);
}

/* Override */ int sigsetmask (int mask) throw()
{
    DEBUGLOGCALL(LCF_SIGNAL);
    LINK_NAMESPACE(sigsetmask, nullptr);
    return orig::sigsetmask(mask);
}

/* Override */ int siggetmask (void) throw()
{
    DEBUGLOGCALL(LCF_SIGNAL);
    LINK_NAMESPACE(siggetmask, nullptr);
    return orig::siggetmask();
}

/* Override */ int sigprocmask (int how, const sigset_t *set, sigset_t *oset) throw()
{
    DEBUGLOGCALL(LCF_SIGNAL);
    LINK_NAMESPACE(sigprocmask, nullptr);
    return orig::sigprocmask(how, set, oset);
}

/* Override */ int sigsuspend (const sigset_t *set)
{
    DEBUGLOGCALL(LCF_SIGNAL);
    LINK_NAMESPACE(sigsuspend, nullptr);
    return orig::sigsuspend(set);
}

/* Override */ int sigaction (int sig, const struct sigaction *act,
    struct sigaction *oact) throw()
{
    DEBUGLOGCALL(LCF_SIGNAL);
    LINK_NAMESPACE(sigaction, nullptr);
    if (act != nullptr) {
        debuglog(LCF_SIGNAL, "Setting handler ", act->sa_sigaction?
                reinterpret_cast<void*>(act->sa_sigaction):
                reinterpret_cast<void*>(act->sa_handler),
            " for signal ", strsignal(sig));
    }
    else if (oact != nullptr) {
        debuglog(LCF_SIGNAL, "Getting handler ", oact->sa_sigaction?
                reinterpret_cast<void*>(oact->sa_sigaction):
                reinterpret_cast<void*>(oact->sa_handler),
            " for signal ", strsignal(sig));
    }
    return orig::sigaction(sig, act, oact);
}

/* Override */ int sigpending (sigset_t *set) throw()
{
    DEBUGLOGCALL(LCF_SIGNAL);
    LINK_NAMESPACE(sigpending, nullptr);
    return orig::sigpending(set);
}

/* Override */ int sigwait (const sigset_t *set, int *sig)
{
    DEBUGLOGCALL(LCF_SIGNAL);
    LINK_NAMESPACE(sigwait, nullptr);
    return orig::sigwait(set, sig);
}

/* Override */ int sigwaitinfo (const sigset_t *set, siginfo_t *info)
{
    DEBUGLOGCALL(LCF_SIGNAL);
    LINK_NAMESPACE(sigwaitinfo, nullptr);
    return orig::sigwaitinfo(set, info);
}

/* Override */ int sigtimedwait (const sigset_t *set,
    siginfo_t *info, const struct timespec *timeout)
{
    DEBUGLOGCALL(LCF_SIGNAL);
    LINK_NAMESPACE(sigtimedwait, nullptr);
    return orig::sigtimedwait(set, info, timeout);
}

/* Override */ int pthread_sigmask (int how, const sigset_t *newmask,
    sigset_t *oldmask) throw()
{
    DEBUGLOGCALL(LCF_SIGNAL | LCF_THREAD);
    LINK_NAMESPACE(pthread_sigmask, nullptr);
    return orig::pthread_sigmask(how, newmask, oldmask);
}

/* Override */ int pthread_kill (pthread_t threadid, int signo) throw()
{
    DEBUGLOGCALL(LCF_SIGNAL | LCF_THREAD);
    LINK_NAMESPACE(pthread_kill, nullptr);
    return orig::pthread_kill(threadid, signo);
}

/* Override */ int pthread_sigqueue (pthread_t threadid, int signo,
                 const union sigval value) throw()
{
    DEBUGLOGCALL(LCF_SIGNAL | LCF_THREAD);
    LINK_NAMESPACE(pthread_sigqueue, nullptr);
    return orig::pthread_sigqueue(threadid, signo, value);
}
