#ifndef THREADS_H_INCL
#define THREADS_H_INCL

#include "hook.h"

typedef int (*SDL_ThreadFunction) (void *data);
typedef void SDL_Thread; // For now

void setMainThread(void);
int isMainThread(void);

extern "C" SDL_Thread* SDL_CreateThread(SDL_ThreadFunction fn, const char *name, void *data);
extern "C" void SDL_WaitThread(SDL_Thread * thread, int *status);
//void SDL_DetachThread(SDL_Thread * thread);

extern "C" int pthread_create (pthread_t * thread, const pthread_attr_t * attr, void * (* start_routine) (void *), void * arg) throw();
extern "C" void pthread_exit (void *retval);
extern "C" int pthread_join (pthread_t thread, void **thread_return);
extern "C" int pthread_detach (pthread_t thread) throw();
extern "C" int pthread_tryjoin_np(pthread_t thread, void **retval) throw();
extern "C" int pthread_timedjoin_np(pthread_t thread, void **retval, const struct timespec *abstime);

#endif
