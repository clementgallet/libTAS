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

static int origUsrMaskProcess = 0;
static thread_local int origUsrMaskThread = 0;

/* Override */ sighandler_t signal (int sig, sighandler_t handler) throw()
{
    DEBUGLOGCALL(LCF_SIGNAL);
    LINK_NAMESPACE(signal, nullptr);

    debuglog(LCF_SIGNAL, "    Setting handler ", reinterpret_cast<void*>(handler),
        " for signal ", strsignal(sig));

    if ((sig == SIGUSR1) || (sig == SIGUSR2)) {
        return SIG_IGN;
    }

    return orig::signal(sig, handler);
}

/* Override */ int sigblock (int mask) throw()
{
    DEBUGLOGCALL(LCF_SIGNAL);
    LINK_NAMESPACE(sigblock, nullptr);

    static const int bannedMask = sigmask(SIGUSR1) | sigmask(SIGUSR2);

    /* Remove our signals from the list of blocked signals */
    int oldmask = orig::sigblock(mask & ~bannedMask);

    /* Add which of our signals were blocked */
    oldmask |= origUsrMaskProcess;

    /* Update which of our signals are blocked */
    origUsrMaskProcess |= mask & bannedMask;

    return oldmask;
}

/* Override */ int sigsetmask (int mask) throw()
{
    DEBUGLOGCALL(LCF_SIGNAL);
    LINK_NAMESPACE(sigsetmask, nullptr);

    static const int bannedMask = sigmask(SIGUSR1) | sigmask(SIGUSR2);

    /* Remove our signals from the list of blocked signals */
    int oldmask = orig::sigsetmask(mask & ~bannedMask);

    /* Update which of our signals were blocked */
    oldmask |= origUsrMaskProcess; // logical 'or' works here because the SIGUSRX
                            // mask bits are always cleared in oldmask

    /* Update which of our signals are blocked */
    origUsrMaskProcess = mask & bannedMask;

    return oldmask;
}

/* Override */ int siggetmask (void) throw()
{
    DEBUGLOGCALL(LCF_SIGNAL);
    LINK_NAMESPACE(siggetmask, nullptr);

    static const int bannedMask = sigmask(SIGUSR1) | sigmask(SIGUSR2);

    int oldmask = orig::siggetmask();

    /* Update which of our signals were blocked */
    oldmask |= origUsrMaskProcess; // logical 'or' works here because the SIGUSRX
                            // mask bits are always cleared in oldmask

    return oldmask;
}

/* Override */ int sigprocmask (int how, const sigset_t *set, sigset_t *oset) throw()
{
    DEBUGLOGCALL(LCF_SIGNAL);
    LINK_NAMESPACE(sigprocmask, nullptr);

    sigset_t newset;
    if (set) {
        sigset_t newset = *set;
        sigdelset(&newset, SIGUSR1);
        sigdelset(&newset, SIGUSR2);
    }

    int ret = orig::sigprocmask(how, set?&newset:set, oset);

    if (ret != -1) {
        if (oset) {
            if (origUsrMaskProcess & SIGUSR1)
                sigaddset(oset, SIGUSR1);
            if (origUsrMaskProcess & SIGUSR2)
                sigaddset(oset, SIGUSR2);
        }

        if (set) {
            int mask = 0;
            if (sigismember(set, SIGUSR1) == 1) mask |= sigmask(SIGUSR1);
            if (sigismember(set, SIGUSR2) == 1) mask |= sigmask(SIGUSR2);

            if (how == SIG_BLOCK)
                origUsrMaskProcess |= mask;
            if (how == SIG_UNBLOCK)
                origUsrMaskProcess &= ~mask;
            if (how == SIG_SETMASK)
                origUsrMaskProcess = mask;
        }
    }
    return ret;
}

/* Override */ int sigsuspend (const sigset_t *set)
{
    DEBUGLOGCALL(LCF_SIGNAL | LCF_TODO);
    LINK_NAMESPACE(sigsuspend, nullptr);
    return orig::sigsuspend(set);
}

/* Override */ int sigaction (int sig, const struct sigaction *act,
    struct sigaction *oact) throw()
{
    DEBUGLOGCALL(LCF_SIGNAL);
    LINK_NAMESPACE(sigaction, nullptr);
    if (act != nullptr) {
        debuglog(LCF_SIGNAL, "    Setting handler ", act->sa_sigaction?
                reinterpret_cast<void*>(act->sa_sigaction):
                reinterpret_cast<void*>(act->sa_handler),
            " for signal ", sig, " (", strsignal(sig), ")");
    }
    else if (oact != nullptr) {
        debuglog(LCF_SIGNAL, "    Getting handler ", oact->sa_sigaction?
                reinterpret_cast<void*>(oact->sa_sigaction):
                reinterpret_cast<void*>(oact->sa_handler),
            " for signal ", sig, " (", strsignal(sig), ")");
    }
    if (!GlobalState::isOwnCode() && ((sig == SIGUSR1) || (sig == SIGUSR2))) {
        debuglog(LCF_SIGNAL | LCF_ERROR, "    The game registers a custom signal!");
    }
    return orig::sigaction(sig, act, oact);
}

/* Override */ int sigpending (sigset_t *set) throw()
{
    DEBUGLOGCALL(LCF_SIGNAL | LCF_TODO);
    LINK_NAMESPACE(sigpending, nullptr);
    return orig::sigpending(set);
}

/* Override */ int sigwait (const sigset_t *set, int *sig)
{
    DEBUGLOGCALL(LCF_SIGNAL | LCF_TODO);
    LINK_NAMESPACE(sigwait, nullptr);
    return orig::sigwait(set, sig);
}

/* Override */ int sigwaitinfo (const sigset_t *set, siginfo_t *info)
{
    DEBUGLOGCALL(LCF_SIGNAL | LCF_TODO);
    LINK_NAMESPACE(sigwaitinfo, nullptr);
    return orig::sigwaitinfo(set, info);
}

/* Override */ int sigtimedwait (const sigset_t *set,
    siginfo_t *info, const struct timespec *timeout)
{
    DEBUGLOGCALL(LCF_SIGNAL | LCF_TODO);
    LINK_NAMESPACE(sigtimedwait, nullptr);
    return orig::sigtimedwait(set, info, timeout);
}

/* Override */ int pthread_sigmask (int how, const sigset_t *newmask,
    sigset_t *oldmask) throw()
{
    DEBUGLOGCALL(LCF_SIGNAL | LCF_THREAD);
    LINK_NAMESPACE(pthread_sigmask, nullptr);

    if (newmask) {
        if (how == SIG_BLOCK)
            debuglog(LCF_SIGNAL, "    Blocking signals:");
        if (how == SIG_UNBLOCK)
            debuglog(LCF_SIGNAL, "    Unblocking signals:");
        if (how == SIG_SETMASK)
            debuglog(LCF_SIGNAL, "    Setting signals to block:");
        for (int s=1; s<NSIG; s++) {
            if (sigismember(newmask, s) == 1)
                debuglog(LCF_SIGNAL, "        ", s, " (", strsignal(s), ")");
        }
    }
    else if (oldmask) {
        debuglog(LCF_SIGNAL, "    Getting blocked signals");
    }

    sigset_t tmpmask;
    if (newmask) {
        sigset_t tmpmask = *newmask;
        sigdelset(&tmpmask, SIGUSR1);
        sigdelset(&tmpmask, SIGUSR2);
    }

    int ret = orig::pthread_sigmask(how, newmask?&tmpmask:newmask, oldmask);

    if (ret != -1) {
        if (oldmask) {
            if (origUsrMaskThread & SIGUSR1)
                sigaddset(oldmask, SIGUSR1);
            if (origUsrMaskThread & SIGUSR2)
                sigaddset(oldmask, SIGUSR2);
        }

        if (newmask) {
            int mask = 0;
            if (sigismember(newmask, SIGUSR1) == 1) mask |= sigmask(SIGUSR1);
            if (sigismember(newmask, SIGUSR2) == 1) mask |= sigmask(SIGUSR2);

            if (how == SIG_BLOCK)
                origUsrMaskThread |= mask;
            if (how == SIG_UNBLOCK)
                origUsrMaskThread &= ~mask;
            if (how == SIG_SETMASK)
                origUsrMaskThread = mask;
        }
    }
    return ret;
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
