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
#include <stdarg.h>

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

/* Handle to the SDL dynamic library that is shipped with the game */
void* SDL_handle;

/* Does SDL use OpenGL for display? */
int video_opengl = 0;

/* Message that identify what is sent in the socket */
int message;

/* Frame counter */
/* TODO: Where should I put this? */
unsigned long frame_counter = 0;

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
        switch (message) {
            case MSGN_TASFLAGS:
                debuglog(LCF_SOCKET, "Receiving tas flags");
                receiveData(&tasflags, sizeof(struct TasFlags));
                break;
            case MSGN_DUMP_FILE:
                debuglog(LCF_SOCKET, "Receiving dump filename");
                size_t str_len;
                receiveData(&str_len, sizeof(size_t));
                dumpfile = malloc(str_len * sizeof(char) + 1);
                receiveData(dumpfile, str_len * sizeof(char));
                dumpfile[str_len] = '\0';
                break;
            case MSGN_SDL_FILE:
                debuglog(LCF_SOCKET, "Receiving sdl filename");
                size_t sdl_len;
                receiveData(&sdl_len, sizeof(size_t));
                sdlfile = malloc(sdl_len * sizeof(char) + 1);
                receiveData(sdlfile, sdl_len * sizeof(char));
                sdlfile[sdl_len] = '\0';
                break;
            default:
                debuglog(LCF_ERROR | LCF_SOCKET, "Unknown socket message %d.", message);
                exit(1);
        }
        receiveData(&message, sizeof(int));
    }
        
    // SMB uses its own version of SDL.
    SDL_handle = dlopen(sdlfile, RTLD_LAZY);
    if (!SDL_handle)
    {
        debuglog(LCF_ERROR | LCF_HOOK, "Could not load SDL.");
        exit(-1);
    }

    if (!hook_functions(SDL_handle))
        exit(-1);

    emptyInputs(&ai);
    emptyInputs(&old_ai);

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
    debuglog(LCF_SDL | LCF_FRAME | LCF_OGL, "%s call.", __func__);
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
        Window w = 42; // TODO: No magic number
        sendMessage(MSGB_WINDOW_ID);
        sendData(&w, sizeof(Window));
        gw_sent = 1;
        debuglog(LCF_SDL, "Send dummy X11 window id.");
    }
    enterFrameBoundary();
}

/* Override */ void SDL_GL_SwapWindow(void* window)
{
    debuglog(LCF_SDL | LCF_FRAME | LCF_OGL, "%s call.", __func__);

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
            debuglog(LCF_SDL, "Send X11 window id: %d", xgw);
        }
    }
    else {
        debuglog(LCF_SDL | LCF_ERROR, "Window pointer is empty but the game wants to draw something.");
        return;
    }

    enterFrameBoundary();

}

/* Override */ int SDL_GL_SetSwapInterval(int interval)
{
    debuglog(LCF_SDL | LCF_OGL, "%s call - setting to %d.", __func__, interval);
    //return SDL_GL_SetSwapInterval_real(interval);

    /* If the game wants the current state of vsync, answer yes */
    if (interval == -1)
        return 1;
    /* Disable vsync */
    return SDL_GL_SetSwapInterval_real(0);
}
    

/* Override */ void* SDL_CreateWindow(const char* title, int x, int y, int w, int h, Uint32 flags){
    debuglog(LCF_SDL, "%s call - title: %s, pos: (%d,%d), size: (%d,%d), flags: 0x%x.", __func__, title, x, y, w, h, flags);
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
    debuglog(LCF_SDL, "%s call.", __func__);
    SDL_DestroyWindow_real(window);
    if (gameWindow == window)
        gameWindow = NULL;
}

/* Override */ Uint32 SDL_GetWindowFlags(void* window){
    debuglog(LCF_SDL, "%s call.", __func__);
    return SDL_GetWindowFlags_real(window);
}

/* Override */ void SDL_Init(unsigned int flags){
    debuglog(LCF_SDL, "%s call.", __func__);
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
    late_hook();
}

/* Override */ int SDL_InitSubSystem(Uint32 flags){
    debuglog(LCF_SDL, "%s call.", __func__);
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
    debuglog(LCF_SDL, "%s call.", __func__);
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
    debuglog(LCF_SDL, "%s call with size (%d,%d), bpp %d and flags 0x%x.", __func__, width, height, bpp, flags);

    /* Disable fullscreen */
    flags &= (0xFFFFFFFF ^ /*SDL_FULLSCREEN*/ 0x80000000);

    if (flags & /*SDL_OPENGL*/ 0x00000002)
        video_opengl = 1;
    return SDL_SetVideoMode_real(width, height, bpp, flags);
}


/* Override */ int SDL_PeepEvents(SDL_Event* events, int numevents, SDL_eventaction action, ...)
{
    debuglog(LCF_SDL | LCF_EVENTS, "%s call.", __func__);

    /* We need to use a function signature with variable arguments,
     * because SDL 1.2 and SDL 2 provide a different function with the same name.
     * SDL 1.2 is int SDL_PeepEvents(SDL_Event *events, int numevents, SDL_eventaction action, Uint32 mask);
     * SDL 2   is int SDL_PeepEvents(SDL_Event* events, int numevents, SDL_eventaction action, Uint32 minType, Uint32 maxType);
     */

    Uint32 mask;
    Uint32 minType, maxType;

    va_list argp;
    va_start(argp, action);
    if (SDLver == 1) {
        mask = va_arg(argp, Uint32);
    }
    else if (SDLver == 2) {
        minType = va_arg(argp, Uint32);
        maxType = va_arg(argp, Uint32);
    }
    else {
        debuglog(LCF_SDL | LCF_ERROR, "Calling SDL_PeepEvents with no known SDL version!");
    }
    va_end(argp);
    
    switch (action) {
        case SDL_ADDEVENT:
            if (SDLver == 2)
                return SDL_PeepEvents_real(events, numevents, action, minType, maxType);
        case SDL_PEEKEVENT:
            if (SDLver == 2)
                return getSDL2Events(events, numevents, 0, minType, maxType);

        case SDL_GETEVENT:
            if (SDLver == 2)
                return getSDL2Events(events, numevents, 1, minType, maxType);

    }

    return 0;
}

/* Override */ int SDL_PollEvent(SDL_Event *event)
{
    debuglog(LCF_SDL | LCF_EVENTS | LCF_FRAME, "%s call.", __func__);

    return getSDL2Events(event, 1, 1, SDL_FIRSTEVENT, SDL_LASTEVENT);
}

/* 
 * This helper function will return a number of events from the generated event queue.
 * This event queue consists on the real SDL event queue with our own filter
 * (e.g. removing real input events)
 * and generated SDL events containing mostly inputs passed from linTAS.
 *
 * Because SDL has multiple ways of accessing the event queue, we made this function
 * with two parameter indicating the number of events we want and if we need 
 * to update the queue by removing the returned event, or keep it in the queue.
 * 
 * The function returns the number of events returned.
 */
int getSDL2Events(SDL_Event *events, int numevents, int update, Uint32 minType, Uint32 maxType)
{

    SDL_PumpEvents_real();


    /* Total number of events pulled from SDL_PeepEvents call */
    int peepnb = 0;


    if (update)
        peepnb = SDL_PeepEvents_real(events, numevents, SDL_GETEVENT, minType, maxType);
    else
        peepnb = SDL_PeepEvents_real(events, numevents, SDL_PEEKEVENT, minType, maxType);

    /* 
     * Among all the events we pulled, we have to filter some
     * (e.g. input type).
     */

    for (int peepi = 0; peepi < peepnb; peepi++) {
        if (filterEvent(&(events[peepi]))) {
            /* 
             * We have to filter this event.
             * For now, let's just attribute an unused type.
             * We will remove all filtered events later.
             */
            events[peepi].type = SDL_FIRSTEVENT;
        }
    }

    /* Now we remove all filtered events from the array
     *
     * | e0 | e1 | e2 | -- | -- | e3 | e4 |
     *                  pd        ps
     *                   _
     *                  /\________/
     */

    int ps, pd = 0;
    while (ps < peepnb) {
        if (events[pd].type != SDL_FIRSTEVENT) {
            pd++;
            if (pd > ps)
                ps++;
        }
        else if (events[ps].type == SDL_FIRSTEVENT) {
            ps++;
        }
        else {
            /* We are in a position to copy event ps to event pd */
            events[pd] = events[ps];
            events[ps].type = SDL_FIRSTEVENT;
            ps++;
            pd++;
        }
    }

    /* We update the total number of saved events */
    peepnb = pd;

    /* 
     * TODO: Actually because of filtered events, we should make
     * multiple passes to PeepEvents if we did not get the right number,
     * but it gets complicated.
     */

    if (peepnb == numevents) {
        /* We got the right number of events from the real event queue */
        return peepnb;
    }

    /* 
     * We did not get enough events with the event queue only.
     * Now we return our custom events.
     */

    /*
     * Important: You must call the following two functions in that order!
     * This is because the first one updates old_ai by removing elements
     * to match ai, and the second one updates old_ai by adding elements
     * to match ai. So if you call the second one before the first,
     * the keyboard array in old_ai can overflow.
     */

    /* Getting KeyUp events */
    if ((SDL_KEYUP >= minType) && (SDL_KEYUP <= maxType))
        peepnb += generateKeyUpEvent(events + peepnb*sizeof(SDL_Event), gameWindow, numevents - peepnb, update);

    if (peepnb == numevents) return peepnb;

    if ((SDL_KEYDOWN >= minType) && (SDL_KEYDOWN <= maxType))

        /* We must add this failsafe concerning the above comment.
         * If we must update SDL_KEYDOWN but did not update SDL_KEYUP,
         * this is bad. So we must update SDL_KEYUP events
         * and discard the result.
         */

        if (update && ((SDL_KEYUP < minType) || (SDL_KEYUP > maxType)))
            /* Update KEYUP events */
            generateKeyUpEvent(events + peepnb*sizeof(SDL_Event), gameWindow, numevents - peepnb, update);

    peepnb += generateKeyDownEvent(events + peepnb*sizeof(SDL_Event), gameWindow, numevents - peepnb, update);

    if (peepnb == numevents) return peepnb;


    if ((SDL_CONTROLLERAXISMOTION >= minType) && (SDL_CONTROLLERAXISMOTION <= maxType))
        /* TODO: Split the function into functions for each event type,
         * or pass the event filters to the function
         */
        peepnb += generateControllerEvent(events + peepnb*sizeof(SDL_Event), numevents - peepnb, update);

    return peepnb;
}

