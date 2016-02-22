#include "threads.h"
#include "logging.h"

SDL_Thread* SDL_CreateThread(SDL_ThreadFunction fn, const char *name, void *data)
{
    debuglog(LCF_THREAD, "SDL Thread %s was created.\n", name);
    return SDL_CreateThread(fn, name, data);
}

void SDL_WaitThread(SDL_Thread * thread, int *status)
{
    debuglog(LCF_THREAD, "Waiting for another SDL thread.\n");
    SDL_WaitThread(thread, status);

}
/*
void SDL_DetachThread(SDL_Thread * thread)
{
    debuglog(LCF_THREAD, "Some SDL thread is detaching.\n");
    SDL_DetachThread(thread);
}
*/

int pthread_create (pthread_t * thread, void * attr, void * (* start_routine) (void *), void * arg)
{
    //if (1) {// Do not allow multi-threaded game
    //    return 0;
    //}
    char name[16];
    name[0] = '\0';
    int ret = pthread_create_real(thread, attr, start_routine, arg);
    pthread_getname_np_real(*thread, name, 16);
    if (name[0])
        debuglog(LCF_THREAD, "Thread %ld was created (%s).", *thread, name);
    else
        debuglog(LCF_THREAD, "Thread %ld was created.", *thread);
    return ret;
}
void pthread_exit (void *retval)
{
    debuglog(LCF_THREAD, "Thread has exited.");
    pthread_exit_real(retval);
}
int pthread_join (pthread_t thread, void **thread_return)
{
    //if (1) {
    //    return 0;
    //}
    debuglog(LCF_THREAD, "Joining thread %ld", thread);
    return pthread_join_real(thread, thread_return);
}
int pthread_detach (pthread_t thread){
    //if (1) {
    //    return 0;
    //}
    debuglog(LCF_THREAD, "Detaching thread %ld.", thread);
    return pthread_detach_real(thread);
}

int pthread_tryjoin_np(pthread_t thread, void **retval)
{
    debuglog(LCF_THREAD, "Try to join thread %ld.", thread);
    int ret = pthread_tryjoin_np_real(thread, retval);
    if (ret == 0) {
        debuglog(LCF_THREAD, "Joining thread %ld successfully.", thread);
    }
    if (ret == EBUSY) {
        debuglog(LCF_THREAD, "Thread %ld has not yet terminated.", thread);
    }
    return ret;
}

int pthread_timedjoin_np(pthread_t thread, void **retval, const struct timespec *abstime)
{
    debuglog(LCF_THREAD, "Try to join thread %ld in %ld ms.", thread, 1000*abstime->tv_sec + abstime->tv_nsec/1000000);
    int ret = pthread_timedjoin_np_real(thread, retval, abstime);
    if (ret == 0) {
        debuglog(LCF_THREAD, "Joining thread %ld successfully.", thread);
    }
    if (ret == ETIMEDOUT) {
        debuglog(LCF_THREAD, "Call timed out before thread %ld terminated.", thread);
    }
    return ret;

}

