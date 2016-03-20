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
#include "../shared/inputs.h"
#include "hook.h"
#include "inputs.h"

#ifdef LIBTAS_HUD
/* Font used for displaying HUD on top of the game window */
TTF_Font *font = NULL;
#endif

char* dumpfile = NULL;

/* Function pointers to real functions */
void (*SDL_Init_real)(unsigned int flags);
int (*SDL_InitSubSystem_real)(Uint32 flags);
void (*SDL_Quit_real)(void);

void __attribute__((constructor)) init(void)
{
    /* Multiple threads may launch the init function, but we only want the main thread to do this */
    //if (! isMainThread())
    //    return;

    initSocket();

#ifdef LIBTAS_HUD
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
                size_t str_len;
                receiveData(&str_len, sizeof(size_t));
                dumpfile = new char[str_len+1];
                receiveData(dumpfile, str_len * sizeof(char));
                dumpfile[str_len] = '\0';
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

    link_time();

    /* Initialize timers */
    nonDetTimer.initialize();
    detTimer.initialize();


}

void __attribute__((destructor)) term(void)
{
    if (getppid() != getpgrp())
        return;
#ifdef LIBTAS_HUD
    TTF_CloseFont(font);
    TTF_Quit();
#endif
    closeSocket();

    debuglog(LCF_SOCKET, "Exiting.");
}

/* Override */ void SDL_Init(unsigned int flags){
    debuglog(LCF_SDL, __func__, " call.");
    /* The thread calling this is probably the main thread */
    setMainThread();

    /* Get which sdl version we are using.
     * Stores it in an extern variable.
     */
    get_sdlversion();

    /* Link function pointers to SDL functions */
    if (SDLver == 1) {
        LINK_SUFFIX(SDL_Init, "libSDL-1.2");
        LINK_SUFFIX(SDL_InitSubSystem, "libSDL-1.2");
        LINK_SUFFIX(SDL_Quit, "libSDL-1.2");
    }
    if (SDLver == 2) {
        LINK_SUFFIX(SDL_Init, "libSDL2-2");
        LINK_SUFFIX(SDL_InitSubSystem, "libSDL2-2");
        LINK_SUFFIX(SDL_Quit, "libSDL2-2");
    }

    link_sdlwindows();
    link_sdlevents();
	link_sdlthreads();
#ifdef LIBTAS_HUD
    link_opengl(); // TODO: Put this when creating the opengl context
#endif

    SDL_Init_real(flags);
    if (flags & SDL_INIT_TIMER)
        debuglog(LCF_SDL, "    SDL_TIMER enabled.");
    if (flags & SDL_INIT_AUDIO)
        debuglog(LCF_SDL, "    SDL_AUDIO enabled.");
    if (flags & SDL_INIT_VIDEO)
        debuglog(LCF_SDL, "    SDL_VIDEO enabled.");
    if (flags & SDL_INIT_CDROM)
        debuglog(LCF_SDL, "    SDL_CDROM enabled.");
    if (flags & SDL_INIT_JOYSTICK)
        debuglog(LCF_SDL, "    SDL_JOYSTICK enabled.");
    if (flags & SDL_INIT_HAPTIC)
        debuglog(LCF_SDL, "    SDL_HAPTIC enabled.");
    if (flags & SDL_INIT_GAMECONTROLLER)
        debuglog(LCF_SDL, "    SDL_GAMECONTROLLER enabled.");
    if (flags & SDL_INIT_EVENTS)
        debuglog(LCF_SDL, "    SDL_EVENTS enabled.");
}

/* Override */ int SDL_InitSubSystem(Uint32 flags){
    debuglog(LCF_SDL, __func__, " call.");
    if (flags & SDL_INIT_TIMER)
        debuglog(LCF_SDL, "    SDL_TIMER enabled.");
    if (flags & SDL_INIT_AUDIO)
        debuglog(LCF_SDL, "    SDL_AUDIO enabled.");
    if (flags & SDL_INIT_VIDEO)
        debuglog(LCF_SDL, "    SDL_VIDEO enabled.");
    if (flags & SDL_INIT_CDROM)
        debuglog(LCF_SDL, "    SDL_CDROM enabled.");
    if (flags & SDL_INIT_JOYSTICK)
        debuglog(LCF_SDL, "    SDL_JOYSTICK enabled.");
    if (flags & SDL_INIT_HAPTIC)
        debuglog(LCF_SDL, "    SDL_HAPTIC enabled.");
    if (flags & SDL_INIT_GAMECONTROLLER)
        debuglog(LCF_SDL, "    SDL_GAMECONTROLLER enabled.");
    if (flags & SDL_INIT_EVENTS)
        debuglog(LCF_SDL, "    SDL_EVENTS enabled.");
    return SDL_InitSubSystem_real(flags);
}

/* Override */ void SDL_Quit(){
    debuglog(LCF_SDL, __func__, " call.");
    sendMessage(MSGB_QUIT);
#ifdef LIBTAS_DUMP
    if (tasflags.av_dumping)
        closeVideoDump();
#endif
    SDL_Quit_real();
}


