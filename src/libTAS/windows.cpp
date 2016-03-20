#include "windows.h"
#include "hook.h"
#include "logging.h"
#include "socket.h"
#include "../shared/messages.h"
#include "frame.h"
#ifdef LIBTAS_DUMP
#include "dumpvideo.h"
#endif

/* 
 * Store the game window pointer
 * (SDL_Window* in fact, but we don't need the SDL_Window struct)
 * We assume the game never open multiple windows at a time
 */
void* gameWindow = NULL;

/* Has the game window pointer be sent to the program? */
int gw_sent = 0;

/* Does SDL use OpenGL for display? */
int video_opengl = 0;

/* Original function pointers */
void(* SDL_GL_SwapWindow_real)(void* window);
void*(* SDL_CreateWindow_real)(const char*, int, int, int, int, Uint32);
Uint32 (*SDL_GetWindowID_real)(void*);
Uint32 (*SDL_GetWindowFlags_real)(void*);
SDL_bool (*SDL_GetWindowWMInfo_real)(void* window, SDL_SysWMinfo* info);
int (*SDL_GL_SetSwapInterval_real)(int interval);
void (*SDL_DestroyWindow_real)(void*);
SDL_Surface *(*SDL_SetVideoMode_real)(int width, int height, int bpp, Uint32 flags);
void (*SDL_GL_SwapBuffers_real)(void);

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

/* Override */ Uint32 SDL_GetWindowID(void* window){
    debuglog(LCF_SDL, __func__, " call.");
    return SDL_GetWindowID_real(window);
}

/* Override */ Uint32 SDL_GetWindowFlags(void* window){
    debuglog(LCF_SDL, __func__, " call.");
    return SDL_GetWindowFlags_real(window);
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

void link_sdlwindows(void)
{
    if (SDLver == 1) {
        LINK_SUFFIX(SDL_GL_SwapBuffers, "libSDL-1.2");
        LINK_SUFFIX(SDL_SetVideoMode, "libSDL-1.2");
    }
    if (SDLver == 2) {
        LINK_SUFFIX(SDL_GL_SwapWindow, "libSDL2-2");
        LINK_SUFFIX(SDL_CreateWindow, "libSDL2-2");
        LINK_SUFFIX(SDL_DestroyWindow, "libSDL2-2");
        LINK_SUFFIX(SDL_GetWindowID, "libSDL2-2");
        LINK_SUFFIX(SDL_GetWindowFlags, "libSDL2-2");
        LINK_SUFFIX(SDL_GL_SetSwapInterval, "libSDL2-2");
        LINK_SUFFIX(SDL_GetWindowWMInfo, "libSDL2-2");
    }
}

