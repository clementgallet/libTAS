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
namespace orig {
    static void* (*SDL_CreateThread)(int(*fn)(void*),
            const char*   name,
            void*         data);
    static void (*SDL_WaitThread)(void* thread, int *status);

    static int (*pthread_create) (pthread_t * thread, const pthread_attr_t * attr, void * (* start_routine) (void *), void * arg) = nullptr;
    static void __attribute__((__noreturn__)) (*pthread_exit) (void *retval) = nullptr;
    static int (*pthread_join) (unsigned long int thread, void **thread_return) = nullptr;
    static int (*pthread_detach) (unsigned long int thread) = nullptr;
    static int (*pthread_getname_np)(unsigned long int thread, char *name, size_t len) = nullptr;
    static int (*pthread_tryjoin_np)(unsigned long int thread, void **retval) = nullptr;
    static int (*pthread_timedjoin_np)(unsigned long int thread, void **retval, const struct timespec *abstime) = nullptr;
    static pthread_t (*pthread_self)(void) = nullptr;
}

/* We keep the identifier of the main thread */
pthread_t mainThread = 0;

/* Get the current thread id */
pthread_t getThreadId(void)
{
    if (orig::pthread_self != nullptr)
        return orig::pthread_self();
    return 0;
}

/* Indicate that we are running on the main thread */
void setMainThread(void)
{
    if (mainThread != 0)
        /* Main thread was already set */
        return;
    if (orig::pthread_self != nullptr)
        mainThread = orig::pthread_self();
}

/* 
 * We will often want to know if we are running on the main thread
 * Because only it can advance the deterministic timer, and other stuff 
 */
int isMainThread(void)
{
    if (orig::pthread_self != nullptr)
        return (orig::pthread_self() == mainThread);

    /* If pthread library is not loaded, it is likely that the game is single threaded */
    return 1;
}

/* Override */ SDL_Thread* SDL_CreateThread(SDL_ThreadFunction fn, const char *name, void *data)
{
    debuglog(LCF_THREAD, "SDL Thread ", name, " was created.");
    return orig::SDL_CreateThread(fn, name, data);
}

/* Override */ void SDL_WaitThread(SDL_Thread * thread, int *status)
{
    DEBUGLOGCALL(LCF_THREAD);
    orig::SDL_WaitThread(thread, status);
}

void link_pthread(void)
{
    LINK_NAMESPACE(pthread_create, "pthread");
    LINK_NAMESPACE(pthread_exit, "pthread");
    LINK_NAMESPACE(pthread_join, "pthread");
    LINK_NAMESPACE(pthread_detach, "pthread");
    LINK_NAMESPACE(pthread_getname_np, "pthread");
    LINK_NAMESPACE(pthread_tryjoin_np, "pthread");
    LINK_NAMESPACE(pthread_timedjoin_np, "pthread");
    LINK_NAMESPACE(pthread_self, "pthread");
}

/* Override */ int pthread_create (pthread_t * thread, const pthread_attr_t * attr, void * (* start_routine) (void *), void * arg) throw()
{
    link_pthread();
    char name[16];
    name[0] = '\0';
    int ret = orig::pthread_create(thread, attr, start_routine, arg);
    orig::pthread_getname_np(*thread, name, 16);
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
    orig::pthread_exit(retval);
}

/* Override */ int pthread_join (pthread_t thread, void **thread_return)
{
    std::string thstr = stringify(thread);
    debuglog(LCF_THREAD, "Joining thread ", thstr);
    return orig::pthread_join(thread, thread_return);
}

/* Override */ int pthread_detach (pthread_t thread) throw()
{
    std::string thstr = stringify(thread);
    debuglog(LCF_THREAD, "Detaching thread ", thstr);
    return orig::pthread_detach(thread);
}

/* Override */ int pthread_tryjoin_np(pthread_t thread, void **retval) throw()
{
    std::string thstr = stringify(thread);
    debuglog(LCF_THREAD, "Try to join thread ", thstr);
    int ret = orig::pthread_tryjoin_np(thread, retval);
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
    int ret = orig::pthread_timedjoin_np(thread, retval, abstime);
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
    LINK_NAMESPACE_SDLX(SDL_CreateThread);
    LINK_NAMESPACE_SDLX(SDL_WaitThread);
}

