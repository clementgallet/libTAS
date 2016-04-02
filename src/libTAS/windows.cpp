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

#include "windows.h"
#include "hook.h"
#include "logging.h"
#include "socket.h"
#include "../shared/messages.h"
#include "../shared/tasflags.h"
#include "frame.h"
#ifdef LIBTAS_ENABLE_AVDUMPING
#include "avdumping.h"
#endif

/* 
 * Store the game window pointer
 * We assume the game never open multiple windows at a time
 */
SDL_Window* gameWindow = nullptr;

/* Has the game window pointer be sent to the program? */
bool gw_sent = 0;

std::string dumpfile;

/* Original function pointers */
void(* SDL_GL_SwapWindow_real)(SDL_Window* window);
SDL_Window*(* SDL_CreateWindow_real)(const char*, int, int, int, int, Uint32);
Uint32 (*SDL_GetWindowID_real)(SDL_Window*);
Uint32 (*SDL_GetWindowFlags_real)(SDL_Window*);
SDL_bool (*SDL_GetWindowWMInfo_real)(SDL_Window* window, SDL_SysWMinfo* info);
int (*SDL_GL_SetSwapInterval_real)(int interval);
void (*SDL_DestroyWindow_real)(SDL_Window*);
SDL1::SDL_Surface *(*SDL_SetVideoMode_real)(int width, int height, int bpp, Uint32 flags);
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

/* Override */ void SDL_GL_SwapWindow(SDL_Window* window)
{
    debuglog(LCF_SDL | LCF_FRAME | LCF_OGL, __func__, " call.");

#ifdef LIBTAS_ENABLE_HUD
    SDL_Color color = {255, 0, 0, 0};
    RenderText(font, "Test test", 640, 480, color, 2, 2);
#endif

    SDL_GL_SwapWindow_real(window);

    /* 
     * We need to pass the game window identifier to the program
     * so that it can capture inputs
     */
    if (gameWindow != nullptr) {
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
            gw_sent = true;
            debuglog(LCF_SDL, "Send X11 window id: ", xgw);
        }
    }
    else {
        debuglog(LCF_SDL | LCF_ERROR, "Window pointer is empty but the game wants to draw something.");
        return;
    }

    frameBoundary();

}

static int swapInterval = 0;

/* Override */ int SDL_GL_SetSwapInterval(int interval)
{
    debuglog(LCF_SDL | LCF_OGL, __func__, " call - setting to ", interval);

    /* We save the interval if the game wants it later */
    swapInterval = interval;
    
    /* Disable vsync */
    /* TODO: Put this at another place to be sure it is executed */
    SDL_GL_SetSwapInterval_real(0);

    return 0; // Success
}
    
/* Override */ int SDL_GL_GetSwapInterval(int interval)
{
    DEBUGLOGCALL(LCF_SDL | LCF_OGL);
    return swapInterval;
}

/* Override */ SDL_Window* SDL_CreateWindow(const char* title, int x, int y, int w, int h, Uint32 flags){
    debuglog(LCF_SDL, __func__, " call - title: ", title, ", pos: (", x, ",", y, "), size: (", w, ",", h, "), flags: 0x", std::hex, flags, std::dec);
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
    gw_sent = false;

#ifdef LIBTAS_ENABLE_AVDUMPING
    /* Initializing the video dump */
    if (tasflags.av_dumping) {
        int video_opengl = 0;
        if (flags & /* SDL_WINDOW_OPENGL */ 0x00000002)
            video_opengl = 1;

        int av = openAVDumping(gameWindow, video_opengl, dumpfile, frame_counter);
        if (av != 0) {
            /* Init failed, disable AV dumping */
            tasflags.av_dumping = 0;
        }
    }
#endif

    return gameWindow;
}

/* Override */ void SDL_DestroyWindow(SDL_Window* window){
    DEBUGLOGCALL(LCF_SDL);
    SDL_DestroyWindow_real(window);
    if (gameWindow == window)
        gameWindow = NULL;
#ifdef LIBTAS_ENABLE_AVDUMPING
    if (tasflags.av_dumping)
        closeAVDumping();
#endif
}

/* Override */ Uint32 SDL_GetWindowID(SDL_Window* window){
    DEBUGLOGCALL(LCF_SDL);
    return SDL_GetWindowID_real(window);
}

/* Override */ Uint32 SDL_GetWindowFlags(SDL_Window* window){
    DEBUGLOGCALL(LCF_SDL);
    return SDL_GetWindowFlags_real(window);
}

int SDL_SetWindowFullscreen(SDL_Window * window, Uint32 flags)
{
    debuglog(LCF_SDL, __func__, " call with flags ", flags);
    return 0; // success
}

void SDL_SetWindowBordered(SDL_Window * window, SDL_bool bordered)
{
    debuglog(LCF_SDL, __func__, " call with border ", bordered);
    /* Don't do anything */
}

/* SDL 1.2 */
/* Override */ SDL1::SDL_Surface *SDL_SetVideoMode(int width, int height, int bpp, Uint32 flags)
{
    debuglog(LCF_SDL, __func__, " call with size (", width, ",", height, "), bpp ", bpp, " and flags ", std::hex, flags, std::dec);

    /* Disable fullscreen */
    flags &= (0xFFFFFFFF ^ /*SDL_FULLSCREEN*/ 0x80000000);

    /* Call real function, but do not return yet */
    SDL1::SDL_Surface *surf = SDL_SetVideoMode_real(width, height, bpp, flags);

#ifdef LIBTAS_ENABLE_AVDUMPING
    /* Initializing the video dump */
    int video_opengl = 0;
    if (flags & /*SDL_OPENGL*/ 0x00000002)
        video_opengl = 1;

    if (tasflags.av_dumping) {
        int av = openAVDumping(gameWindow, video_opengl, dumpfile, frame_counter);
        if (av != 0) {
            /* Init failed, disable AV dumping */
            tasflags.av_dumping = 0;
        }
    }
#endif

    return surf;
}

void link_sdlwindows(void)
{
    if (SDLver == 1) {
        LINK_SUFFIX_SDL1(SDL_GL_SwapBuffers);
        LINK_SUFFIX_SDL1(SDL_SetVideoMode);
    }
    if (SDLver == 2) {
        LINK_SUFFIX_SDL2(SDL_GL_SwapWindow);
        LINK_SUFFIX_SDL2(SDL_CreateWindow);
        LINK_SUFFIX_SDL2(SDL_DestroyWindow);
        LINK_SUFFIX_SDL2(SDL_GetWindowID);
        LINK_SUFFIX_SDL2(SDL_GetWindowFlags);
        LINK_SUFFIX_SDL2(SDL_GL_SetSwapInterval);
        LINK_SUFFIX_SDL2(SDL_GetWindowWMInfo);
    }
}

