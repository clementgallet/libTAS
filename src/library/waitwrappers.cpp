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

#include "waitwrappers.h"
#include "logging.h"
#include "checkpoint/ThreadManager.h"
#include "DeterministicTimer.h"
#include "backtrace.h"
#include "GlobalState.h"
#include "hook.h"
#ifdef __linux__
#include "audio/alsa/pcm.h"
#endif

namespace libtas {

DEFINE_ORIG_POINTER(poll)
DEFINE_ORIG_POINTER(select)
DEFINE_ORIG_POINTER(pselect)
#ifdef __linux__
DEFINE_ORIG_POINTER(epoll_wait)
#endif

/* Advance time when sleep call, depending on config and main thread.
 * Returns if the call was transfered.
 */
static bool transfer_sleep(const struct timespec &ts)
{
    if (ts.tv_sec == 0 && ts.tv_nsec == 0)
        return false;

    switch (shared_config.sleep_handling) {
        case SharedConfig::SLEEP_NEVER:
            return false;
        case SharedConfig::SLEEP_MAIN:
            if (!ThreadManager::isMainThread())
                return false;
        case SharedConfig::SLEEP_ALWAYS:
            detTimer.addDelay(ts);
            NATIVECALL(sched_yield());
            return true;        
    }
    return true;
}

/* Override */ int poll (struct pollfd *fds, nfds_t nfds, int timeout)
{
    LINK_NAMESPACE_GLOBAL(poll);

    if (GlobalState::isNative()) {
        return orig::poll(fds, nfds, timeout);
    }

    debuglogstdio(LCF_WAIT, "%s call with %d fds and timeout %d", __func__, nfds, timeout);

#ifdef __linux__
    /* Check for the fd used by ALSA */
    for (nfds_t i = 0; i < nfds; i++) {
        if (fds[i].fd == 0xa15a) {
            /* Wait here until our ALSA buffer has enough bytes available */
            int ret = snd_pcm_wait(reinterpret_cast<snd_pcm_t*>(static_cast<intptr_t>(fds[i].revents)), timeout);

            if (ret == 1) return ret;

            /* Call the poll on the remaining fds if any */
            if (nfds > 1) {
                fds[i] = fds[nfds-1];
                ret = orig::poll(fds, nfds-1, timeout);

                /* If call returned an event, don't handle the ALSA fd */
                if (ret > 0)
                    return ret;
            }

            return ret;
        }
    }
#endif
    
    int ret = orig::poll(fds, nfds, timeout);

    /* If timeout on main thread, add the timeout amount to the timer */
    if (ret == 0 && timeout > 0) {
        struct timespec ts;
        ts.tv_sec = timeout / 1000;
        ts.tv_nsec = timeout * 1000000;

        transfer_sleep(ts);        
    }

    return ret;
}

/* Override */ int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout)
{
    LINK_NAMESPACE_GLOBAL(select);

    /* select can be used to sleep the cpu if feed with all null parameters
     * except for timeout. In this case we replace it with what we did for
     * other sleep functions. Otherwise, we call the original function.
     */
    if ((nfds != 0) || (readfds != nullptr) || (writefds != nullptr) || (exceptfds != nullptr))
    {
        return orig::select(nfds, readfds, writefds, exceptfds, timeout);
    }

    if (GlobalState::isNative()) {
        return orig::select(nfds, readfds, writefds, exceptfds, timeout);
    }

    debuglogstdio(LCF_SLEEP, "%s call - sleep for %d.%09d sec", __func__, timeout->tv_sec, timeout->tv_usec);

    struct timespec ts;
    ts.tv_sec = timeout->tv_sec;
    ts.tv_nsec = timeout->tv_usec * 1000;
    
    if (!transfer_sleep(ts))
        return orig::select(nfds, readfds, writefds, exceptfds, timeout);

    return 0;
}

/* Override */ int pselect (int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
	const struct timespec *timeout, const sigset_t *sigmask)
{
    LINK_NAMESPACE_GLOBAL(pselect);

    /* select can be used to sleep the cpu if feed with all null parameters
     * except for timeout. In this case we replace it with what we did for
     * other sleep functions. Otherwise, we call the original function.
     */
    if ((nfds != 0) || (readfds != nullptr) || (writefds != nullptr) || (exceptfds != nullptr))
    {
        return orig::pselect(nfds, readfds, writefds, exceptfds, timeout, sigmask);
    }

    if (GlobalState::isNative()) {
        return orig::pselect(nfds, readfds, writefds, exceptfds, timeout, sigmask);
    }

    debuglogstdio(LCF_SLEEP, "%s call - sleep for %d.%09d sec", __func__, timeout->tv_sec, timeout->tv_nsec);

    if (!transfer_sleep(*timeout))
        return orig::pselect(nfds, readfds, writefds, exceptfds, timeout, sigmask);

    return 0;
}

#ifdef __linux__
/* Override */ int epoll_wait (int epfd, struct epoll_event *events, int maxevents, int timeout)
{
    LINK_NAMESPACE_GLOBAL(epoll_wait);

    if (GlobalState::isNative()) {
        return orig::epoll_wait(epfd, events, maxevents, timeout);
    }

    debuglogstdio(LCF_SLEEP, "%s call with timeout %d", __func__, timeout);

    int ret = orig::epoll_wait(epfd, events, maxevents, timeout);

    /* If the function was called from the main thread and the function timed out,
     * advance the timer. */
    if ((timeout != -1) && (ret == 0)) {
        struct timespec ts;
        ts.tv_sec = timeout / 1000;
        ts.tv_nsec = 1000000 * (timeout % 1000);
        transfer_sleep(ts);
    }

    return ret;
}
#endif

}
