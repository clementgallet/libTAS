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
#include "windows.h"
#include "dlhook.h"
#include "events.h"
#include "threads.h"
#include "opengl.h"
#include "socket.h"
#include "logging.h"
#include "NonDeterministicTimer.h"
#include "DeterministicTimer.h"
#include "sdl_ttf.h"
#include "../shared/messages.h"
#include "../shared/tasflags.h"
#include "../shared/AllInputs.h"
#include "hook.h"
#include "inputs/inputs.h"
#include "fileio.h"
#ifdef LIBTAS_ENABLE_AVDUMPING
#include "avdumping.h"
#endif

/* Did we call our constructor? */
bool libTAS_init = false;

#ifdef LIBTAS_ENABLE_HUD
/* Font used for displaying HUD on top of the game window */
TTF_Font *font = NULL;
#endif

/* Function pointers to real functions */
void (*SDL_Init_real)(unsigned int flags) = nullptr;
int (*SDL_InitSubSystem_real)(Uint32 flags) = nullptr;
void (*SDL_Quit_real)(void) = nullptr;

void __attribute__((constructor (101))) init(void)
{
    bool didConnect = initSocket();
    /* Sometimes the game starts a process that is not a thread, so that this constructor is called again
     * In this case, we must detect it and do not run this again
     */
    if (! didConnect)
        return;

#ifdef LIBTAS_ENABLE_HUD
    /* Initialize SDL TTF */
    if(TTF_Init() == -1) {
        debuglog(LCF_ERROR | LCF_SDL, "Couldn't init SDL TTF.");
        exit(1);
    }

    font = TTF_OpenFont("/home/clement/libTAS/src/external/FreeSans.ttf", 30);
    if (font == NULL) {
        debuglog(LCF_ERROR | LCF_SDL, "Couldn't load font");
        exit(1);
    }
#endif
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
    while (message != MSGN_END_INIT) {
        std::vector<char> buf;
        std::string libstring;
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
                av_filename = (char*)malloc(dump_len+1);
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
                libraries.push_back(libstring);
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
    link_stdiofileio();
    link_posixfileio();
    link_time();
    link_pthread();

    /* Initialize timers */
    nonDetTimer.initialize();
    detTimer.initialize();

    libTAS_init = true;

}

void __attribute__((destructor)) term(void)
{
    dlhook_end();

#ifdef LIBTAS_ENABLE_HUD
    TTF_CloseFont(font);
    TTF_Quit();
#endif
    closeSocket();

    debuglog(LCF_SOCKET, "Exiting.");
}

/* Override */ void SDL_Init(unsigned int flags){
    debuglog(LCF_SDL, __func__, " call.");

    /* Get which sdl version we are using.
     * Stores it in an extern variable.
     */
    get_sdlversion();

    LINK_SUFFIX_SDLX(SDL_Init);
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
        SDL_Init_real(flags);
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
    LINK_SUFFIX_SDLX(SDL_Init);
    LINK_SUFFIX_SDLX(SDL_InitSubSystem);
    LINK_SUFFIX_SDLX(SDL_Quit);

    link_sdlwindows();
    link_sdlevents();
    link_sdlthreads();
    link_sdlfileio();
#ifdef LIBTAS_ENABLE_HUD
    link_opengl(); // TODO: Put this when creating the opengl context
#endif

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

    return SDL_InitSubSystem_real(flags);
}

/* Override */ void SDL_Quit(){
    debuglog(LCF_SDL, __func__, " call.");

#ifdef LIBTAS_ENABLE_AVDUMPING
    /* SDL 1.2 does not have a destroy window function,
     * because there is only one window,
     * so we close the av dumping here instead
     */
    if (tasflags.av_dumping && (SDLver == 1))
        closeAVDumping();
#endif

    sendMessage(MSGB_QUIT);
    SDL_Quit_real();
}


