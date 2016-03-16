#ifndef THREADS_H_INCL
#define THREADS_H_INCL

#include "hook.h"
#include "global.h"

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

#endif
