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

#include "threads.h"
#include "logging.h"
#include <errno.h>
#include <unistd.h>

/* Original function pointers */
void* (*SDL_CreateThread_real)(int(*fn)(void*),
                       const char*   name,
                       void*         data);
void (*SDL_WaitThread_real)(void* thread, int *status);
//void (*SDL_DetachThread_real)(void * thread);

int (*pthread_create_real) (pthread_t * thread, const pthread_attr_t * attr, void * (* start_routine) (void *), void * arg) = nullptr;
void __attribute__((__noreturn__)) (*pthread_exit_real) (void *retval) = nullptr;
int (*pthread_join_real) (unsigned long int thread, void **thread_return) = nullptr;
int (*pthread_detach_real) (unsigned long int thread) = nullptr;
int (*pthread_getname_np_real)(unsigned long int thread, char *name, size_t len) = nullptr;
int (*pthread_tryjoin_np_real)(unsigned long int thread, void **retval) = nullptr;
int (*pthread_timedjoin_np_real)(unsigned long int thread, void **retval, const struct timespec *abstime) = nullptr;
pthread_t (*pthread_self_real)(void) = nullptr;

/* We keep the identifier of the main thread */
pthread_t mainThread = 0;

/* Get the current thread id */
pthread_t getThreadId(void)
{
    if (pthread_self_real != nullptr)
        return pthread_self_real();
    return 0;
}

/* Indicate that we are running on the main thread */
void setMainThread(void)
{
    if (mainThread != 0)
        /* Main thread was already set */
        return;
    if (pthread_self_real != nullptr)
        mainThread = pthread_self_real();
}

/* 
 * We will often want to know if we are running on the main thread
 * Because only it can advance the deterministic timer, and other stuff 
 */
int isMainThread(void)
{
    if (pthread_self_real != nullptr)
        return (pthread_self_real() == mainThread);

    /* If pthread library is not loaded, it is likely that the game is single threaded */
    return 1;
}

/* Override */ SDL_Thread* SDL_CreateThread(SDL_ThreadFunction fn, const char *name, void *data)
{
    debuglog(LCF_THREAD, "SDL Thread ", name, " was created.");
    return SDL_CreateThread_real(fn, name, data);
}

/* Override */ void SDL_WaitThread(SDL_Thread * thread, int *status)
{
    debuglog(LCF_THREAD, "Waiting for another SDL thread.");
    SDL_WaitThread_real(thread, status);

}
/*
void SDL_DetachThread(SDL_Thread * thread)
{
    debuglog(LCF_THREAD, "Some SDL thread is detaching.\n");
    SDL_DetachThread(thread);
}
*/

void link_pthread(void);
void link_pthread(void)
{
    LINK_SUFFIX(pthread_create, "pthread");
    LINK_SUFFIX(pthread_exit, "pthread");
    LINK_SUFFIX(pthread_join, "pthread");
    LINK_SUFFIX(pthread_detach, "pthread");
    LINK_SUFFIX(pthread_getname_np, "pthread");
    LINK_SUFFIX(pthread_tryjoin_np, "pthread");
    LINK_SUFFIX(pthread_timedjoin_np, "pthread");
    LINK_SUFFIX(pthread_self, "pthread");
}

/* Override */ int pthread_create (pthread_t * thread, const pthread_attr_t * attr, void * (* start_routine) (void *), void * arg) throw()
{
    link_pthread();
    char name[16];
    name[0] = '\0';
    int ret = pthread_create_real(thread, attr, start_routine, arg);
    pthread_getname_np_real(*thread, name, 16);
    std::string thstr = stringify(*thread);
    if (name[0])
        debuglog(LCF_THREAD, "Thread ", thstr, " was created (", name, ").");
    else
        debuglog(LCF_THREAD, "Thread ", thstr, " was created.");
    return ret;
}

/* Override */ void pthread_exit (void *retval)
{
    debuglog(LCF_THREAD, "Thread has exited.");
    pthread_exit_real(retval);
}

/* Override */ int pthread_join (pthread_t thread, void **thread_return)
{
    std::string thstr = stringify(thread);
    debuglog(LCF_THREAD, "Joining thread ", thstr);
    return pthread_join_real(thread, thread_return);
}

/* Override */ int pthread_detach (pthread_t thread) throw()
{
    std::string thstr = stringify(thread);
    debuglog(LCF_THREAD, "Detaching thread ", thstr);
    return pthread_detach_real(thread);
}

/* Override */ int pthread_tryjoin_np(pthread_t thread, void **retval) throw()
{
    std::string thstr = stringify(thread);
    debuglog(LCF_THREAD, "Try to join thread ", thstr);
    int ret = pthread_tryjoin_np_real(thread, retval);
    if (ret == 0) {
        debuglog(LCF_THREAD, "Joining thread ", thstr, " successfully.");
    }
    if (ret == EBUSY) {
        debuglog(LCF_THREAD, "Thread ", thstr, " has not yet terminated.");
    }
    return ret;
}

/* Override */ int pthread_timedjoin_np(pthread_t thread, void **retval, const struct timespec *abstime)
{
    std::string thstr = stringify(thread);
    debuglog(LCF_THREAD, "Try to join thread ", thstr, " in ", 1000*abstime->tv_sec + abstime->tv_nsec/1000000," ms.");
    int ret = pthread_timedjoin_np_real(thread, retval, abstime);
    if (ret == 0) {
        debuglog(LCF_THREAD, "Joining thread ", thstr, " successfully.");
    }
    if (ret == ETIMEDOUT) {
        debuglog(LCF_THREAD, "Call timed out before thread ", thstr, " terminated.");
    }
    return ret;
}

void link_sdlthreads(void)
{
    LINK_SUFFIX_SDLX(SDL_CreateThread);
    LINK_SUFFIX_SDLX(SDL_WaitThread);
}


