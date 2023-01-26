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

#include "signalwrappers.h"
#include "logging.h"
#include "GlobalState.h"
#include "hook.h"
#include "checkpoint/ThreadSync.h"
#include "checkpoint/SaveStateManager.h" // checkpoint signals

#include <cstring>
#include <csignal>

namespace libtas {

DEFINE_ORIG_POINTER(signal)
DEFINE_ORIG_POINTER(sigblock)
DEFINE_ORIG_POINTER(sigsetmask)
DEFINE_ORIG_POINTER(siggetmask)
DEFINE_ORIG_POINTER(sigprocmask)
DEFINE_ORIG_POINTER(sigsuspend)
DEFINE_ORIG_POINTER(sigaction)
DEFINE_ORIG_POINTER(sigpending)
DEFINE_ORIG_POINTER(sigwait)
DEFINE_ORIG_POINTER(sigwaitinfo)
DEFINE_ORIG_POINTER(sigtimedwait)
DEFINE_ORIG_POINTER(sigaltstack)
DEFINE_ORIG_POINTER(pthread_sigmask)
DEFINE_ORIG_POINTER(pthread_kill)
DEFINE_ORIG_POINTER(pthread_sigqueue)

static int origUsrMaskProcess = 0;
static thread_local int origUsrMaskThread = 0;

/* Override */ sighandler_t signal (int sig, sighandler_t handler) __THROW
{
    DEBUGLOGCALL(LCF_SIGNAL);
    LINK_NAMESPACE_GLOBAL(signal);

    /* Our checkpoint code uses signals, so we must prevent the game from
     * signaling threads at the same time.
     */
    ThreadSync::wrapperExecutionLockLock();

    debuglogstdio(LCF_SIGNAL, "    Setting handler %p for signal %s", reinterpret_cast<void*>(handler), strsignal(sig));

    if ((sig == SaveStateManager::sigSuspend()) || (sig == SaveStateManager::sigCheckpoint())) {
        return SIG_IGN;
    }

    sighandler_t ret = orig::signal(sig, handler);

    ThreadSync::wrapperExecutionLockUnlock();

    return ret;
}
    
/* Override */ int sigblock (int mask) __THROW
{
    DEBUGLOGCALL(LCF_SIGNAL);
    LINK_NAMESPACE_GLOBAL(sigblock);

    static const int bannedMask = sigmask(SaveStateManager::sigSuspend()) | sigmask(SaveStateManager::sigCheckpoint());

    /* Remove our signals from the list of blocked signals */
    int oldmask = orig::sigblock(mask & ~bannedMask);

    /* Add which of our signals were blocked */
    oldmask |= origUsrMaskProcess;

    /* Update which of our signals are blocked */
    origUsrMaskProcess |= mask & bannedMask;

    return oldmask;
}

/* Override */ int sigsetmask (int mask) __THROW
{
    DEBUGLOGCALL(LCF_SIGNAL);
    LINK_NAMESPACE_GLOBAL(sigsetmask);

    static const int bannedMask = sigmask(SaveStateManager::sigSuspend()) | sigmask(SaveStateManager::sigCheckpoint());

    /* Remove our signals from the list of blocked signals */
    int oldmask = orig::sigsetmask(mask & ~bannedMask);

    /* Update which of our signals were blocked */
    oldmask |= origUsrMaskProcess; // logical 'or' works here because the SIGUSRX
                            // mask bits are always cleared in oldmask

    /* Update which of our signals are blocked */
    origUsrMaskProcess = mask & bannedMask;

    return oldmask;
}

/* Override */ int siggetmask (void) __THROW
{
    DEBUGLOGCALL(LCF_SIGNAL);
    LINK_NAMESPACE_GLOBAL(siggetmask);

    int oldmask = orig::siggetmask();

    /* Update which of our signals were blocked */
    oldmask |= origUsrMaskProcess; // logical 'or' works here because the SIGUSRX
                            // mask bits are always cleared in oldmask

    return oldmask;
}

/* Override */ int sigprocmask (int how, const sigset_t *set, sigset_t *oset) __THROW
{
    DEBUGLOGCALL(LCF_SIGNAL);
    LINK_NAMESPACE_GLOBAL(sigprocmask);

    if (GlobalState::isNative())
        return orig::sigprocmask(how, set, oset);

    sigset_t newset;
    if (set) {
        newset = *set;
        sigdelset(&newset, SaveStateManager::sigSuspend());
        sigdelset(&newset, SaveStateManager::sigCheckpoint());
    }

    int ret = orig::sigprocmask(how, set?&newset:set, oset);

    if (ret != -1) {
        if (oset) {
            if (origUsrMaskProcess & SaveStateManager::sigSuspend())
                sigaddset(oset, SaveStateManager::sigSuspend());
            if (origUsrMaskProcess & SaveStateManager::sigCheckpoint())
                sigaddset(oset, SaveStateManager::sigCheckpoint());
        }

        if (set) {
            int mask = 0;
            if (sigismember(set, SaveStateManager::sigSuspend()) == 1) mask |= sigmask(SaveStateManager::sigSuspend());
            if (sigismember(set, SaveStateManager::sigCheckpoint()) == 1) mask |= sigmask(SaveStateManager::sigCheckpoint());

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
    LINK_NAMESPACE_GLOBAL(sigsuspend);

    sigset_t tmp;
    if (set) {
        tmp = *set;
        sigdelset(&tmp, SaveStateManager::sigSuspend());
        sigdelset(&tmp, SaveStateManager::sigCheckpoint());
        set = &tmp;
    }

    return orig::sigsuspend(set);
}

/* Override */ int sigaction (int sig, const struct sigaction *act,
    struct sigaction *oact) __THROW
{
    LINK_NAMESPACE_GLOBAL(sigaction);

    if (GlobalState::isNative()) {
        return orig::sigaction(sig, act, oact);
    }

    DEBUGLOGCALL(LCF_SIGNAL);

    /* Our checkpoint code uses signals, so we must prevent the game from
     * signaling threads at the same time.
     */
    ThreadSync::wrapperExecutionLockLock();

    /* Save the original handlers for signals that we will skip */
    struct sigaction act_suspend, act_checkpoint;
    act_suspend.sa_handler = SIG_DFL;
    act_checkpoint.sa_handler = SIG_DFL;

    if (sig == SaveStateManager::sigSuspend()) {
        debuglogstdio(LCF_SIGNAL, "    Skipping because libTAS uses that signal for suspend");
        
        if (oact != nullptr)
            *oact = act_suspend;

        if (act != nullptr)
            act_suspend = *act;
            
        ThreadSync::wrapperExecutionLockUnlock();
        return 0;
    }

    if (sig == SaveStateManager::sigCheckpoint()) {
        debuglogstdio(LCF_SIGNAL, "    Skipping because libTAS uses that signal for checkpoint");
        
        if (oact != nullptr)
            *oact = act_checkpoint;

        if (act != nullptr)
            act_checkpoint = *act;
            
        ThreadSync::wrapperExecutionLockUnlock();
        return 0;
    }
    
    if (oact != nullptr) {
        debuglogstdio(LCF_SIGNAL, "    Getting handler %p for signal %d (%s)", (oact->sa_flags & SA_SIGINFO)?
            reinterpret_cast<void*>(oact->sa_sigaction):
            reinterpret_cast<void*>(oact->sa_handler), sig, strsignal(sig));
    }
    if (act != nullptr) {
        debuglogstdio(LCF_SIGNAL, "    Setting handler %p for signal %d (%s)", (act->sa_flags & SA_SIGINFO)?
                reinterpret_cast<void*>(act->sa_sigaction):
                reinterpret_cast<void*>(act->sa_handler), sig, strsignal(sig));
    }

    int ret = orig::sigaction(sig, act, oact);

    ThreadSync::wrapperExecutionLockUnlock();

    return ret;
}

/* Override */ int sigpending (sigset_t *set) __THROW
{
    DEBUGLOGCALL(LCF_SIGNAL | LCF_TODO);
    LINK_NAMESPACE_GLOBAL(sigpending);
    return orig::sigpending(set);
}

/* Override */ int sigwait (const sigset_t *set, int *sig)
{
    DEBUGLOGCALL(LCF_SIGNAL | LCF_TODO);
    LINK_NAMESPACE_GLOBAL(sigwait);
    return orig::sigwait(set, sig);
}

/* Override */ int sigwaitinfo (const sigset_t *set, siginfo_t *info)
{
    DEBUGLOGCALL(LCF_SIGNAL | LCF_TODO);
    LINK_NAMESPACE_GLOBAL(sigwaitinfo);
    return orig::sigwaitinfo(set, info);
}

/* Override */ int sigtimedwait (const sigset_t *set,
    siginfo_t *info, const struct timespec *timeout)
{
    DEBUGLOGCALL(LCF_SIGNAL | LCF_TODO);
    LINK_NAMESPACE_GLOBAL(sigtimedwait);
    return orig::sigtimedwait(set, info, timeout);
}

/* Override */ int sigaltstack (const stack_t *ss, stack_t *oss) __THROW
{
    LINK_NAMESPACE_GLOBAL(sigaltstack);

    if (GlobalState::isNative()) {
        return orig::sigaltstack(ss, oss);
    }

    DEBUGLOGCALL(LCF_SIGNAL);

    if (ss) {
        debuglogstdio(LCF_SIGNAL, "    Setting altstack with base address %p and size %d", ss->ss_sp, ss->ss_size);
    }
    else if (oss) {
        debuglogstdio(LCF_SIGNAL, "    Getting altstack with base address %p and size %d", oss->ss_sp, oss->ss_size);
    }

    int ret = orig::sigaltstack(ss, oss);

    return ret;
}

/* Override */ int pthread_sigmask (int how, const sigset_t *newmask,
    sigset_t *oldmask) __THROW
{
    DEBUGLOGCALL(LCF_SIGNAL | LCF_THREAD);
    LINK_NAMESPACE_GLOBAL(pthread_sigmask);

    /* This is a bit of a workaround. We still want native threads
     * (like pulseaudio thread) to be able to be suspended, but we also want
     * threads to unblock SIGUSR1 and SIGUSR2, so we only allow native threads
     * to unblock.
     */
    if (GlobalState::isNative() && (how == SIG_UNBLOCK))
        return orig::pthread_sigmask(how, newmask, oldmask);

    if (newmask) {
        if (how == SIG_BLOCK)
            debuglogstdio(LCF_SIGNAL | LCF_THREAD, "    Blocking signals:");
        if (how == SIG_UNBLOCK)
            debuglogstdio(LCF_SIGNAL | LCF_THREAD, "    Unblocking signals:");
        if (how == SIG_SETMASK)
            debuglogstdio(LCF_SIGNAL | LCF_THREAD, "    Setting signals to block:");
        // for (int s=1; s<NSIG; s++) {
        //     if (sigismember(newmask, s) == 1)
                /* I encountered a deadlock here when using strsignal() to print
                 * the signal name with the following pattern:
                 * malloc() -> acquires lock -> signal handler called ->
                 * pthread_sigmask() -> strsignal() -> malloc() ->
                 * acquires lock -> deadlock
                 *
                 * So I don't use any function that is making memory allocation.
                 */
                // debuglogstdio(LCF_SIGNAL | LCF_THREAD, "        %d", s);
        // }
    }
    else if (oldmask) {
        debuglogstdio(LCF_SIGNAL | LCF_THREAD, "    Getting blocked signals");
    }

    sigset_t tmpmask;
    if (newmask) {
        tmpmask = *newmask;
        sigdelset(&tmpmask, SaveStateManager::sigSuspend());
        sigdelset(&tmpmask, SaveStateManager::sigCheckpoint());
    }

    int ret = orig::pthread_sigmask(how, (newmask==nullptr)?nullptr:&tmpmask, oldmask);

    if (ret != -1) {
        if (oldmask) {
#if defined(__APPLE__) && defined(__MACH__)
            if (!Global::is_inited) {
                if (origUsrMaskProcess & SaveStateManager::sigSuspend())
                    sigaddset(oldmask, SaveStateManager::sigSuspend());
                if (origUsrMaskProcess & SaveStateManager::sigCheckpoint())
                    sigaddset(oldmask, SaveStateManager::sigCheckpoint());
            } else {
#endif
            if (origUsrMaskThread & SaveStateManager::sigSuspend())
                sigaddset(oldmask, SaveStateManager::sigSuspend());
            if (origUsrMaskThread & SaveStateManager::sigCheckpoint())
                sigaddset(oldmask, SaveStateManager::sigCheckpoint());
#if defined(__APPLE__) && defined(__MACH__)
            }
#endif
        }

        if (newmask) {
            int mask = 0;
            if (sigismember(newmask, SaveStateManager::sigSuspend()) == 1) mask |= sigmask(SaveStateManager::sigSuspend());
            if (sigismember(newmask, SaveStateManager::sigCheckpoint()) == 1) mask |= sigmask(SaveStateManager::sigCheckpoint());

#if defined(__APPLE__) && defined(__MACH__)
            if (!Global::is_inited) {
                if (how == SIG_BLOCK)
                    origUsrMaskProcess |= mask;
                if (how == SIG_UNBLOCK)
                    origUsrMaskProcess &= ~mask;
                if (how == SIG_SETMASK)
                    origUsrMaskProcess = mask;
            } else {
#endif
            if (how == SIG_BLOCK)
                origUsrMaskThread |= mask;
            if (how == SIG_UNBLOCK)
                origUsrMaskThread &= ~mask;
            if (how == SIG_SETMASK)
                origUsrMaskThread = mask;
#if defined(__APPLE__) && defined(__MACH__)
            }
#endif
        }
    }

    return ret;
}

/* Override */ int pthread_kill (pthread_t threadid, int signo) __THROW
{
    LINK_NAMESPACE_GLOBAL(pthread_kill);

    if (GlobalState::isNative())
        return orig::pthread_kill(threadid, signo);

    debuglogstdio(LCF_SIGNAL | LCF_THREAD, "%s called with thread %p and signo %d", __func__, threadid, signo);

    /* Our checkpoint code uses signals, so we must prevent the game from
     * signaling threads at the same time.
     */
    ThreadSync::wrapperExecutionLockLock();

    int ret = orig::pthread_kill(threadid, signo);

    ThreadSync::wrapperExecutionLockUnlock();

    return ret;
}

/* Override */ int pthread_sigqueue (pthread_t threadid, int signo,
                 const union sigval value) __THROW
{
    DEBUGLOGCALL(LCF_SIGNAL | LCF_THREAD);
    LINK_NAMESPACE_GLOBAL(pthread_sigqueue);
    return orig::pthread_sigqueue(threadid, signo, value);
}

}
