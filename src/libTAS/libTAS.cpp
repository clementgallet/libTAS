#include "libTAS.h"
#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/syscall.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <iomanip>
#include <vector>
#include <string>

#include "../external/SDL_ttf.h"
#include "keyboard_helper.h"
#include "hook.h"
#include "../shared/messages.h"
#include "../shared/inputs.h"
#include "../shared/tasflags.h"
#ifdef LIBTAS_DUMP
#include "dumpvideo.h"
#endif
#include "time.h"
#include "inputs.h"
#include "opengl.h"
#include "frame.h"
#include "socket.h"
#include "threads.h"
#include "logging.h"
#include "events.h"
#include "NonDeterministicTimer.h"
#include "DeterministicTimer.h"
#include "dlhook.h"

/* Handle to the SDL dynamic library that is shipped with the game */
void* SDL_handle;

/* Does SDL use OpenGL for display? */
int video_opengl = 0;

/* Message that identify what is sent in the socket */
int message;

#ifdef LIBTAS_HUD
/* Font used for displaying HUD on top of the game window */
TTF_Font *font = NULL;
#endif

/* 
 * Store the game window pointer
 * (SDL_Window* in fact, but we don't need the SDL_Window struct)
 * We assume the game never open multiple windows at a time
 */
void* gameWindow = NULL;

/* Has the game window pointer be sent to the program? */
int gw_sent = 0;

char* dumpfile = NULL;
char* sdlfile = NULL;


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
            case MSGN_SDL_FILE:
                debuglog(LCF_SOCKET, "Receiving sdl filename");
                size_t sdl_len;
                receiveData(&sdl_len, sizeof(size_t));
                sdlfile = new char[sdl_len+1];
                receiveData(sdlfile, sdl_len * sizeof(char));
                sdlfile[sdl_len] = '\0';
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
        
    // SMB uses its own version of SDL.
    dlenter();
    SDL_handle = dlopen(sdlfile, RTLD_LAZY);
    dlleave();
    if (!SDL_handle)
    {
        debuglog(LCF_ERROR | LCF_HOOK, "Could not load SDL.");
        exit(-1);
    }

    if (!hook_functions(SDL_handle))
        exit(-1);

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
    dlclose(SDL_handle);
    closeSocket();

    debuglog(LCF_SOCKET, "Exiting.");
}

/* SDL 1.2 */
/* Override */ void SDL_GL_SwapBuffers(void)
{
    debuglog(LCF_SDL | LCF_FRAME | LCF_OGL, __func__, " call.");
    SDL_GL_SwapBuffers_real();

    /* TODO: Fill here same as SDL_GL_SwapWindow */

    /* SDL 1.2 does only have one window,
     * thus it does not provide any access to window identifiers.
     * We need to pass a window id to linTAS so that it can capture inputs.
     * In our case, let's just pass a dummy value indicating that
     * we could not get access to it.
     * It will have to guess it, probably by getting the active window
     */
    if (!gw_sent) {
        Window w = 0;
        sendMessage(MSGB_WINDOW_ID);
        sendData(&w, sizeof(Window));
        gw_sent = 1;
        debuglog(LCF_SDL, "Send dummy X11 window id.");
    }
    frameBoundary();
}

/* Override */ void SDL_GL_SwapWindow(void* window)
{
    debuglog(LCF_SDL | LCF_FRAME | LCF_OGL, __func__, " call.");

#ifdef LIBTAS_HUD
    SDL_Color color = {255, 0, 0, 0};
    RenderText(font, "Test test", 640, 480, color, 2, 2);
#endif

    SDL_GL_SwapWindow_real(window);

#ifdef LIBTAS_DUMP
    /* Dumping audio and video if needed */
    static int dump_inited = 0;
    if (tasflags.av_dumping) {
        if (! dump_inited) {
            /* Initializing the video dump */
            int av = openVideoDump(gameWindow, video_opengl, dumpfile);
            dump_inited = 1;
            if (av != 0)
                /* Init failed, disable AV dumping */
                tasflags.av_dumping = 0;
        }
    }

    if (tasflags.av_dumping) {
        /* Write the current frame */
        int enc = encodeOneFrame(frame_counter, gameWindow);
        if (enc != 0)
            /* Encode failed, disable AV dumping */
            tasflags.av_dumping = 0;

    }
#endif

    /* 
     * We need to pass the game window identifier to the program
     * so that it can capture inputs
     */
    if (gameWindow != NULL) {
        if (!gw_sent) {

            /* Access the X Window identifier from the SDL_Window struct */
            SDL_SysWMinfo info;
            SDL_GetVersion_real(&info.version);
            if (SDL_GetWindowWMInfo_real(gameWindow, &info) == SDL_FALSE) {
                debuglog(LCF_SDL | LCF_ERROR, "Could not get the X11 window identifier");
                return;
            }
            if (info.subsystem != SDL_SYSWM_X11) {
                debuglog(LCF_SDL | LCF_ERROR, "SDL says we are not running on X11");
                return;
            }
            Window xgw = info.info.x11.window;

            /* Send the X Window identifier to the program */
            sendMessage(MSGB_WINDOW_ID);
            sendData(&xgw, sizeof(Window));
            gw_sent = 1;
            debuglog(LCF_SDL, "Send X11 window id: ", xgw);
        }
    }
    else {
        debuglog(LCF_SDL | LCF_ERROR, "Window pointer is empty but the game wants to draw something.");
        return;
    }

    frameBoundary();

}

/* Override */ int SDL_GL_SetSwapInterval(int interval)
{
    debuglog(LCF_SDL | LCF_OGL, __func__, " call - setting to ", interval);
    //return SDL_GL_SetSwapInterval_real(interval);

    /* If the game wants the current state of vsync, answer yes */
    if (interval == -1)
        return 1;
    /* Disable vsync */
    /* TODO: Put this at another place to be sure it is executed */
    return SDL_GL_SetSwapInterval_real(0);
}
    

/* Override */ void* SDL_CreateWindow(const char* title, int x, int y, int w, int h, Uint32 flags){
    debuglog(LCF_SDL, __func__, " call - title: ", title, ", pos: (", x, ",", y, "), size: (", w, ",", h, "), flags: 0x", std::hex, flags, std::dec);
    if (flags & /* SDL_WINDOW_OPENGL */ 0x00000002)
        video_opengl = 1;

    /* Disable fullscreen */
    flags &= 0xFFFFFFFF ^ /*SDL_WINDOW_FULLSCREEN_DESKTOP*/ 0x00001001;

    /* Disable hidden windows */
    flags &= 0xFFFFFFFF ^ /*SDL_WINDOW_HIDDEN*/ 0x00000008;

    /* Disable high DPI mode */
    flags &= 0xFFFFFFFF ^ /*SDL_WINDOW_ALLOW_HIGHDPI*/ 0x00002000;

    /* Check if the game provided screen coordinates */
    if (w == 0 || h == 0) {
        w = 800;
        h = 600;
    }

    gameWindow = SDL_CreateWindow_real(title, x, y, w, h, flags); // Save the game window
    /* A new window was created. It needs to be passed to the program */
    gw_sent = 0;
    return gameWindow;
}

/* Override */ void SDL_DestroyWindow(void* window){
    debuglog(LCF_SDL, __func__, " call.");
    SDL_DestroyWindow_real(window);
    if (gameWindow == window)
        gameWindow = NULL;
}

/* Override */ Uint32 SDL_GetWindowFlags(void* window){
    debuglog(LCF_SDL, __func__, " call.");
    return SDL_GetWindowFlags_real(window);
}

/* Override */ void SDL_Init(unsigned int flags){
    debuglog(LCF_SDL, __func__, " call.");
    /* The thread calling this is probably the main thread */
    setMainThread();

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
    /* Try to hook more functions after SDL was inited */
    late_glhook();
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

/* SDL 1.2 */
/* Override */ SDL_Surface *SDL_SetVideoMode(int width, int height, int bpp, Uint32 flags)
{
    debuglog(LCF_SDL, __func__, " call with size (", width, ",", height, "), bpp ", bpp, " and flags ", std::hex, flags, std::dec);

    /* Disable fullscreen */
    flags &= (0xFFFFFFFF ^ /*SDL_FULLSCREEN*/ 0x80000000);

    if (flags & /*SDL_OPENGL*/ 0x00000002)
        video_opengl = 1;
    return SDL_SetVideoMode_real(width, height, bpp, flags);
}


