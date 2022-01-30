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

#ifndef LIBTAS_PTHREADS_H_INCL
#define LIBTAS_PTHREADS_H_INCL

#include "global.h"
#include <pthread.h> // pthread_t
#include <semaphore.h>
#if defined(__APPLE__) && defined(__MACH__)
//#include <pthread_workqueue.h> // pthread_workqueue_t
#endif


namespace libtas {

/* Create a new thread, starting with execution of START-ROUTINE
   getting passed ARG.  Creation attributed come from ATTR.  The new
   handle is stored in *NEWTHREAD.  */
OVERRIDE int pthread_create (pthread_t * thread, const pthread_attr_t * attr, void * (* start_routine) (void *), void * arg) __THROW;

/* Terminate calling thread.

   The registered cleanup handlers are called via exception handling
   so we cannot mark this function with __THROW.*/
OVERRIDE void pthread_exit (void *retval);

/* Make calling thread wait for termination of the thread TH.  The
   exit status of the thread is stored in *THREAD_RETURN, if THREAD_RETURN
   is not NULL.

   This function is a cancellation point and therefore not marked with
   __THROW.  */
OVERRIDE int pthread_join (pthread_t thread, void **thread_return);

/* Indicate that the thread TH is never to be joined with PTHREAD_JOIN.
   The resources of TH will therefore be freed immediately when it
   terminates, instead of waiting for another thread to perform PTHREAD_JOIN
   on it.  */
OVERRIDE int pthread_detach (pthread_t thread) __THROW;

/* Check whether thread TH has terminated.  If yes return the status of
   the thread in *THREAD_RETURN, if THREAD_RETURN is not NULL.  */
OVERRIDE int pthread_tryjoin_np(pthread_t thread, void **retval) __THROW;

/* Make calling thread wait for termination of the thread TH, but only
   until TIMEOUT.  The exit status of the thread is stored in
   *THREAD_RETURN, if THREAD_RETURN is not NULL.

   This function is a cancellation point and therefore not marked with
   __THROW.  */
OVERRIDE int pthread_timedjoin_np(pthread_t thread, void **retval, const struct timespec *abstime);

/* Initialize condition variable COND using attributes ATTR, or use
   the default values if later is NULL.  */
OVERRIDE int pthread_cond_init (pthread_cond_t *cond, const pthread_condattr_t *cond_attr) __THROW;

/* Wake up one thread waiting for condition variable COND.  */
OVERRIDE int pthread_cond_signal (pthread_cond_t *cond) __THROW;

/* Wake up all threads waiting for condition variables COND.  */
OVERRIDE int pthread_cond_broadcast (pthread_cond_t *__cond) __THROW;

/* Wait for condition variable COND to be signaled or broadcast.
   MUTEX is assumed to be locked before.

   This function is a cancellation point and therefore not marked with
   __THROW.  */
OVERRIDE int pthread_cond_wait (pthread_cond_t *cond, pthread_mutex_t *mutex);

/* Wait for condition variable COND to be signaled or broadcast until
   ABSTIME.  MUTEX is assumed to be locked before.  ABSTIME is an
   absolute time specification; zero is the beginning of the epoch
   (00:00:00 GMT, January 1, 1970).

   This function is a cancellation point and therefore not marked with
   __THROW.  */
OVERRIDE int pthread_cond_timedwait (pthread_cond_t *cond, pthread_mutex_t *mutex, const struct timespec *abstime);

/* Set cancelability state of current thread to STATE, returning old
   state in *OLDSTATE if OLDSTATE is not NULL.  */
OVERRIDE int pthread_setcancelstate (int state, int *oldstate);

/* Set cancellation state of current thread to TYPE, returning the old
   type in *OLDTYPE if OLDTYPE is not NULL.  */
OVERRIDE int pthread_setcanceltype (int type, int *oldtype);

/* Cancel THREAD immediately or at the next possibility.  */
OVERRIDE int pthread_cancel (pthread_t th);

/* Test for pending cancellation for the current thread and terminate
   the thread as per pthread_exit(PTHREAD_CANCELED) if it has been
   cancelled.  */
OVERRIDE void pthread_testcancel (void);

/* Wait for SEM being posted.

   This function is a cancellation point and therefore not marked with
   __THROW.  */
OVERRIDE int sem_wait (sem_t *sem);

/* Similar to `sem_wait' but wait only until ABSTIME.

   This function is a cancellation point and therefore not marked with
   __THROW.  */
OVERRIDE int sem_timedwait (sem_t *sem, const struct timespec *abstime);

/* Test whether SEM is posted.  */
OVERRIDE int sem_trywait (sem_t *sem) __THROW;

/* Post SEM.  */
OVERRIDE int sem_post (sem_t *sem) __THROW;

OVERRIDE int pthread_attr_setstack(pthread_attr_t *attr, void *stackaddr, size_t stacksize) __THROW;

OVERRIDE int pthread_condattr_setclock(pthread_condattr_t *attr, clockid_t clock_id) __THROW;

OVERRIDE int pthread_condattr_getclock(const pthread_condattr_t *attr, clockid_t *clock_id) __THROW;

/* Set thread name visible in the kernel and its interfaces.  */
#ifdef __unix__
OVERRIDE int pthread_setname_np (pthread_t target_thread, const char *name) __THROW;
#elif defined(__APPLE__) && defined(__MACH__)
OVERRIDE int pthread_setname_np (const char *name);
#endif

#if defined(__APPLE__) && defined(__MACH__)
typedef unsigned long pthread_priority_t;
typedef void (*pthread_workqueue_function2_t)(pthread_priority_t priority);
typedef void (*pthread_workqueue_function_kevent_t)(void **events, int *nevents);
typedef void (*pthread_workqueue_function_workloop_t)(uint64_t *workloop_id, void **events, int *nevents);
OVERRIDE int _pthread_workqueue_init_with_workloop(pthread_workqueue_function2_t queue_func, pthread_workqueue_function_kevent_t kevent_func, pthread_workqueue_function_workloop_t workloop_func, int offset, int flags);
#endif

}

#endif
