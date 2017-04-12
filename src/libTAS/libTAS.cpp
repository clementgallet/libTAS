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

#include "libTAS.h"
#include <vector>
#include <string>
#include "time.h"
#include "timer.h"
#include "sleep.h"
#include "sdlwindows.h"
#include "dlhook.h"
#include "sdlevents.h"
#include "threads.h"
#include "socket.h"
#include "logging.h"
#include "NonDeterministicTimer.h"
#include "DeterministicTimer.h"
#include "../shared/messages.h"
#include "../shared/tasflags.h"
#include "../shared/AllInputs.h"
#include "hook.h"
#include "inputs/inputs.h"
#include "ThreadManager.h"
//#ifdef LIBTAS_ENABLE_AVDUMPING
//#include "avdumping.h"
//#endif

/* Did we call our constructor? */
bool libTAS_init = false;

/* Function pointers to real functions */
namespace orig {
    static void (*SDL_Init)(unsigned int flags) = nullptr;
    static int (*SDL_InitSubSystem)(Uint32 flags) = nullptr;
    static int (*SDL1_VideoInit)(const char* driver_name, Uint32 flags) = nullptr;
    static int (*SDL2_VideoInit)(const char* driver_name) = nullptr;
    static void (*SDL_VideoQuit)(void) = nullptr;
    static void (*SDL_Quit)(void) = nullptr;
}

void __attribute__((constructor)) init(void)
{
    bool didConnect = initSocket();
    /* Sometimes the game starts a process that is not a thread, so that this constructor is called again
     * In this case, we must detect it and do not run this again
     */
    if (! didConnect)
        return;

    /* Send information to the program */

    /* Send game process pid */
    debuglog(LCF_SOCKET, "Send pid to program");
    sendMessage(MSGB_PID);
    pid_t mypid = getpid();
    sendData(&mypid, sizeof(pid_t));

    /* End message */
    sendMessage(MSGB_END_INIT);

    /* Receive information from the program */
    int message;
    receiveData(&message, sizeof(int));
    libraries = new safe::vector<safe::string>;
    while (message != MSGN_END_INIT) {
        std::vector<char> buf;
        safe::string libstring;
        switch (message) {
            case MSGN_TASFLAGS:
                debuglog(LCF_SOCKET, "Receiving tas flags");
                receiveData(&tasflags, sizeof(struct TasFlags));
                break;
            case MSGN_DUMP_FILE:
                debuglog(LCF_SOCKET, "Receiving dump filename");
                size_t dump_len;
                receiveData(&dump_len, sizeof(size_t));
                /* TODO: Put all this in TasFlags class methods */
                av_filename = static_cast<char*>(malloc(dump_len+1));
                receiveData(av_filename, dump_len);
                av_filename[dump_len] = '\0';
                debuglog(LCF_SOCKET, "File ", av_filename);
                break;
            case MSGN_LIB_FILE:
                debuglog(LCF_SOCKET, "Receiving lib filename");
                size_t lib_len;
                receiveData(&lib_len, sizeof(size_t));
                buf.resize(lib_len, 0x00);
                receiveData(&(buf[0]), lib_len);
                libstring.assign(&(buf[0]), buf.size());
                libraries->push_back(libstring);
                debuglog(LCF_SOCKET, "Lib ", libstring.c_str());
                break;
            default:
                debuglog(LCF_ERROR | LCF_SOCKET, "Unknown socket message ", message);
                exit(1);
        }
        receiveData(&message, sizeof(int));
    }

    ai.emptyInputs();
    old_ai.emptyInputs();
    game_ai.emptyInputs();

    /* We initialize our dl functions hooking, and link some functions */
    link_time();
    link_sleep();
    link_sdltimer();
    link_pthread();

    /* Initialize timers */
    nonDetTimer.initialize();
    detTimer.initialize();

    libTAS_init = true;

}

void __attribute__((destructor)) term(void)
{
    dlhook_end();

    closeSocket();

    debuglog(LCF_SOCKET, "Exiting.");
}

/* Override */ void SDL_Init(unsigned int flags){
    debuglog(LCF_SDL, __func__, " call.");
    debuglog(LCF_SDL, "Return addr ", __builtin_return_address(0), ".");
    ThreadManager::get().init(pthread_self());
    /* Get which sdl version we are using.
     * Stores it in an extern variable.
     */
    get_sdlversion();

    LINK_NAMESPACE_SDLX(SDL_Init);
    /* In both SDL1 and SDL2, SDL_Init() calls SDL_InitSubSystem(),
     * but in SDL2, SDL_Init() can actually never be called by the game,
     * so we put the rest of relevent code in the SubSystem function.
     *
     * ...well, this is in theory. If on SDL2 we call SDL_Init(), it
     * does not call our SDL_InitSubSystem() function. Maybe it has to do with
     * some compiler optimization, because the real SDL_Init() function looks
     * like this:
     *      int SDL_Init(Uint32 flags) {
     *          return SDL_InitSubSystem(flags);
     *      }
     * So maybe the compiler is inlining stuff. To fix this, we call
     * ourselves our own SDL_InitSubSystem() function.
     */
    if (SDLver == 1)
        orig::SDL_Init(flags);
    if (SDLver == 2)
        SDL_InitSubSystem(flags);
}

/* Override */ int SDL_InitSubSystem(Uint32 flags){
    debuglog(LCF_SDL, __func__, " call.");

    /* Get which sdl version we are using.
     * Stores it in an extern variable.
     */
    get_sdlversion();

    /* Link function pointers to SDL functions */
    LINK_NAMESPACE_SDLX(SDL_Init);
    LINK_NAMESPACE_SDLX(SDL_InitSubSystem);
    if (SDLver == 1)
        link_function((void**)&orig::SDL1_VideoInit, "SDL_VideoInit", "libSDL-1.2");
    if (SDLver == 2)
        link_function((void**)&orig::SDL2_VideoInit, "SDL_VideoInit", "libSDL2-2");
    LINK_NAMESPACE_SDLX(SDL_VideoQuit);
    LINK_NAMESPACE_SDLX(SDL_Quit);

    link_sdlwindows();
    link_sdlevents();
    link_sdlthreads();

    /* The thread calling this is probably the main thread */
    setMainThread();

    if (flags & SDL_INIT_TIMER)
        debuglog(LCF_SDL, "    SDL_TIMER enabled.");
    if (flags & SDL_INIT_AUDIO)
        debuglog(LCF_SDL, "    SDL_AUDIO fake enabled.");
    if (flags & SDL_INIT_VIDEO)
        debuglog(LCF_SDL, "    SDL_VIDEO enabled.");
    if (flags & SDL_INIT_CDROM)
        debuglog(LCF_SDL, "    SDL_CDROM enabled.");
    if (flags & SDL_INIT_JOYSTICK)
        debuglog(LCF_SDL, "    SDL_JOYSTICK fake enabled.");
    if (flags & SDL_INIT_HAPTIC)
        debuglog(LCF_SDL, "    SDL_HAPTIC enabled.");
    if (flags & SDL_INIT_GAMECONTROLLER)
        debuglog(LCF_SDL, "    SDL_GAMECONTROLLER fake enabled.");
    if (flags & SDL_INIT_EVENTS)
        debuglog(LCF_SDL, "    SDL_EVENTS enabled.");

    /* Disabling Joystick subsystem, we don't need any initialization from SDL */
    flags &= 0xFFFFFFFF ^ SDL_INIT_JOYSTICK;

    /* Disabling GameController subsystem, we don't need any initialization from SDL */
    flags &= 0xFFFFFFFF ^ SDL_INIT_GAMECONTROLLER;

    /* Disabling Audio subsystem so that it does not create an extra thread */
    flags &= 0xFFFFFFFF ^ SDL_INIT_AUDIO;

    return orig::SDL_InitSubSystem(flags);
}

/* Override */ int SDL_VideoInit(const char* driver_name, Uint32 flags)
{
    DEBUGLOGCALL(LCF_SDL);
    int rv = 0;
    //threadState.setNative(true);
    if (SDLver == 1) {
        rv = orig::SDL1_VideoInit(driver_name, flags);
    }
    if (SDLver == 2) {
        rv = orig::SDL2_VideoInit(driver_name);
    }
    //threadState.setNative(false);
    return rv;
}

/* Override */ void SDL_VideoQuit(void)
{
    DEBUGLOGCALL(LCF_SDL);
    threadState.setNative(true);
    orig::SDL_VideoQuit();
    threadState.setNative(false);
}

/* Override */ void SDL_Quit(){
    DEBUGLOGCALL(LCF_SDL);
    debuglog(LCF_THREAD, ThreadManager::get().summary());
#ifdef LIBTAS_ENABLE_AVDUMPING
    /* SDL 1.2 does not have a destroy window function,
     * because there is only one window,
     * so we close the av dumping here instead.
     * However, the dumping will be closed at the
     * end of program because, so maybe we don't need this.
     */
    //avencoder.reset(nullptr);
#endif

    sendMessage(MSGB_QUIT);
    orig::SDL_Quit();
}
