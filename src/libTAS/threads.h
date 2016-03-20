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

#ifndef THREADS_H_INCL
#define THREADS_H_INCL

#include "hook.h"
#include "global.h"

typedef unsigned long int pthread_t;

extern pthread_t (*pthread_self_real)(void);

typedef int (*SDL_ThreadFunction) (void *data);
typedef void SDL_Thread; // For now

pthread_t getThreadId(void);
void setMainThread(void);
int isMainThread(void);

OVERRIDE SDL_Thread* SDL_CreateThread(SDL_ThreadFunction fn, const char *name, void *data);
OVERRIDE void SDL_WaitThread(SDL_Thread * thread, int *status);
//void SDL_DetachThread(SDL_Thread * thread);

OVERRIDE int pthread_create (pthread_t * thread, const pthread_attr_t * attr, void * (* start_routine) (void *), void * arg) throw();
OVERRIDE void pthread_exit (void *retval);
OVERRIDE int pthread_join (pthread_t thread, void **thread_return);
OVERRIDE int pthread_detach (pthread_t thread) throw();
OVERRIDE int pthread_tryjoin_np(pthread_t thread, void **retval) throw();
OVERRIDE int pthread_timedjoin_np(pthread_t thread, void **retval, const struct timespec *abstime);

void link_sdlthreads(void);

#endif
