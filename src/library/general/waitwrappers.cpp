/*
    Copyright 2015-2023 Clément Gallet <clement.gallet@ens-lyon.org>

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
#include "sleepwrappers.h" // transfer_sleep()

#include "logging.h"
#include "checkpoint/ThreadManager.h"
#include "DeterministicTimer.h"
#include "backtrace.h"
#include "GlobalState.h"
#include "hook.h"
#include "global.h"
#ifdef __linux__
#include "audio/alsa/pcm.h"
#endif

namespace libtas {

DEFINE_ORIG_POINTER(poll)
DEFINE_ORIG_POINTER(ppoll)
DEFINE_ORIG_POINTER(select)
DEFINE_ORIG_POINTER(pselect)
#ifdef __linux__
DEFINE_ORIG_POINTER(epoll_wait)
#endif

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
        if (fds[i].fd == 0xa15a) { // ALSA
            
            /* If timeout is infinite, we must check alternatively the alsa fd
             * and the other ones with finite timeouts, otherwise we may be stuck
             * on one infinite wait while the other one received an event */
            do {
                /* Wait here until our ALSA buffer has enough bytes available */
                int ret = snd_pcm_wait(reinterpret_cast<snd_pcm_t*>(static_cast<intptr_t>(fds[i].revents)), timeout);
                
                if (ret == 1)
                    return ret;
                
                /* Call the poll on the remaining fds if any */
                if (nfds > 1) {
                    fds[i] = fds[nfds-1];
                    ret = orig::poll(fds, nfds-1, (timeout==-1)?100:timeout); // Arbitrary check with 100 ms timeout
                    
                    /* If call returned an event, recover the original fds structure and return */
                    if (ret > 0) {
                        fds[nfds-1] = fds[i];
                        fds[i].fd = 0xa15a;
                        fds[i].events = POLLIN;
                        return ret;
                    }
                    if (ret < 0)
                        return ret;
                }
            } while (timeout == -1);
            
            /* Reaching here means that we timeout */
            return 0;
        }
    }
#endif
    
    int ret = orig::poll(fds, nfds, timeout);

    /* If timeout on main thread, add the timeout amount to the timer */
    if (ret == 0 && timeout > 0) {
        struct timespec ts;
        ts.tv_sec = timeout / 1000;
        ts.tv_nsec = timeout * 1000000;

        transfer_sleep(ts, NULL);        
    }

    return ret;
}

int ppoll (struct pollfd *fds, nfds_t nfds, const struct timespec *timeout, const __sigset_t *ss)
{
    LINK_NAMESPACE_GLOBAL(ppoll);

    if (GlobalState::isNative()) {
        return orig::ppoll(fds, nfds, timeout, ss);
    }

    if (timeout)
        debuglogstdio(LCF_WAIT, "%s call with %d fds and timeout %d.%09d", __func__, nfds, timeout->tv_sec, timeout->tv_nsec);
    else
        debuglogstdio(LCF_WAIT, "%s call with %d fds and infinite timeout", __func__, nfds);
    
    int ret = orig::ppoll(fds, nfds, timeout, ss);

    /* If timeout on main thread, add the timeout amount to the timer */
    if (ret == 0 && timeout) {
        transfer_sleep(*timeout, NULL);        
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

    debuglogstdio(LCF_SLEEP, "%s call - sleep for %d.%06d sec", __func__, timeout->tv_sec, timeout->tv_usec);

    struct timespec ts;
    ts.tv_sec = timeout->tv_sec;
    ts.tv_nsec = timeout->tv_usec * 1000;
    
    transfer_sleep(ts, NULL);
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

    transfer_sleep(*timeout, NULL);
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
        transfer_sleep(ts, NULL);
    }

    return ret;
}
#endif

}
