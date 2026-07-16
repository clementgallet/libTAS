// Compile with `g++ cpp_exceptions.cpp -lSDL2 -o cpp_exceptions`

/* pthread_cancel and c++ exceptions do not mix well together.
 * To reproduce the crash, save a state before frame 10, advance past frame 10 and reload the state.
 *
 * Log:
 * 
 * INFO: Caught exception in thread
 * FATAL: exception not rethrown
 *
 * Stacktrace:
 * 
 * Thread 27 "cpp_exceptions" received signal SIGABRT, Aborted.
 * [Switching to Thread 0x7fff81bff6c0 (LWP 34072)]
 * __pthread_kill_implementation (threadid=<optimized out>, signo=signo@entry=6, no_tid=no_tid@entry=0) at ./nptl/pthread_kill.c:44
 * warning: 44	./nptl/pthread_kill.c: Aucun fichier ou dossier de ce nom
 * (gdb) bt
 * #0  __pthread_kill_implementation (threadid=<optimized out>, signo=signo@entry=6, no_tid=no_tid@entry=0) at ./nptl/pthread_kill.c:44
 * #1  0x00007ffff709f9ff in __pthread_kill_internal (threadid=<optimized out>, signo=6) at ./nptl/pthread_kill.c:89
 * #2  0x00007ffff704acc2 in __GI_raise (sig=sig@entry=6) at ../sysdeps/posix/raise.c:26
 * #3  0x00007ffff70334ac in __GI_abort () at ./stdlib/abort.c:77
 * #4  0x00007ffff7034291 in __libc_message_impl (fmt=fmt@entry=0x7ffff71b502b "%s") at ../sysdeps/posix/libc_fatal.c:134
 * #5  0x00007ffff7093b65 in __GI___libc_fatal (message=message@entry=0x7ffff71b7de8 "FATAL: exception not rethrown\n") at ../sysdeps/posix/libc_fatal.c:143
 * #6  0x00007ffff70a6070 in unwind_cleanup (reason=<optimized out>, exc=<optimized out>) at ./nptl/unwind.c:114
 * #7  0x00005555555552b7 in thread_exception() ()
 * #8  0x0000555555555b9b in void std::__invoke_impl<void, void (*)()>(std::__invoke_other, void (*&&)()) ()
 * #9  0x0000555555555b53 in std::__invoke_result<void (*)()>::type std::__invoke<void (*)()>(void (*&&)()) ()
 * #10 0x0000555555555b00 in void std::thread::_Invoker<std::tuple<void (*)()> >::_M_invoke<0ul>(std::_Index_tuple<0ul>) ()
 * #11 0x0000555555555ad4 in std::thread::_Invoker<std::tuple<void (*)()> >::operator()() ()
 * #12 0x0000555555555ab8 in std::thread::_State_impl<std::thread::_Invoker<std::tuple<void (*)()> > >::_M_run() ()
 * #13 0x00007ffff72e1224 in ?? () from /lib/x86_64-linux-gnu/libstdc++.so.6
 * #14 0x00007ffff78b9e68 in libtas::pthread_start (arg=0x555558ae7e20) at ../../../src/library/general/pthreadwrappers.cpp:81
 * #15 0x00007ffff709db7b in start_thread (arg=<optimized out>) at ./nptl/pthread_create.c:448
 * #16 0x00007ffff711b7b8 in __GI___clone3 () at ../sysdeps/unix/sysv/linux/x86_64/clone3.S:78
 * 
 */

#include "sdl_test_helpers.h"
#include <exception>
#include <unistd.h>
#include <thread>

static void slsl(int i) {
    sleep(i);
}

static void thread_exception() {
    try {
        slsl(1000);
    } catch (...) {
        SDL_Log("Caught exception in thread");
    }
}

int main()
{
    SDL_TEST_INIT();

    bool run = true;

    for (int i = 0; i < 10; i++) {
        SDL_TEST_FRAME(run);
        if (!run) break;
    }

    std::thread th(thread_exception);

    for (int i = 0; i < 100; i++) {
        SDL_TEST_FRAME(run);
        if (!run) break;
    }

    SDL_TEST_QUIT();
}