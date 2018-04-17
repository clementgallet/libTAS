/*
    Copyright 2015-2018 Clément Gallet <clement.gallet@ens-lyon.org>

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

#include "sdlwindows.h"
#include "hook.h"
#include "sdlversion.h"
#include "logging.h"
#include "../shared/sockethelpers.h"
#include "../shared/messages.h"
#include "../shared/SharedConfig.h"
#include "frame.h"
#include "renderhud/RenderHUD_GL.h"
#include "renderhud/RenderHUD_SDL1.h"
#include "renderhud/RenderHUD_SDL2.h"
#include "timewrappers.h"
#include "ScreenCapture.h"
#include <SDL2/SDL_syswm.h>
#include "WindowTitle.h"

namespace libtas {

/*
 * Store the game window pointer
 * We assume the game never open multiple windows at a time
 */
SDL_Window* gameWindow = nullptr;

DEFINE_ORIG_POINTER(SDL_GL_SwapWindow);
DEFINE_ORIG_POINTER(SDL_CreateWindow);
DEFINE_ORIG_POINTER(SDL_GetWindowID);
DEFINE_ORIG_POINTER(SDL_GetWindowFlags);
DEFINE_ORIG_POINTER(SDL_SetWindowTitle);
DEFINE_ORIG_POINTER(SDL_WM_SetCaption);
DEFINE_ORIG_POINTER(SDL_GL_CreateContext);
DEFINE_ORIG_POINTER(SDL_GL_SetSwapInterval);
DEFINE_ORIG_POINTER(SDL_DestroyWindow);
DEFINE_ORIG_POINTER(SDL_GetWindowPosition);
DEFINE_ORIG_POINTER(SDL_SetWindowSize);
DEFINE_ORIG_POINTER(SDL_CreateRenderer);
DEFINE_ORIG_POINTER(SDL_CreateWindowAndRenderer);
DEFINE_ORIG_POINTER(SDL_RenderPresent);
DEFINE_ORIG_POINTER(SDL_SetVideoMode);
DEFINE_ORIG_POINTER(SDL_GL_SwapBuffers);
DEFINE_ORIG_POINTER(SDL_Flip);
DEFINE_ORIG_POINTER(SDL_GL_SetAttribute);

/* SDL 1.2 */
/* Override */ void SDL_GL_SwapBuffers(void)
{
    LINK_NAMESPACE_SDL1(SDL_GL_SwapBuffers);

    if (GlobalState::isNative())
        return orig::SDL_GL_SwapBuffers();

    debuglog(LCF_SDL | LCF_FRAME | LCF_OGL | LCF_WINDOW, __func__, " call.");

    /* Start the frame boundary and pass the function to draw */
#ifdef LIBTAS_ENABLE_HUD
    static RenderHUD_GL renderHUD;
    frameBoundary(true, [] () {orig::SDL_GL_SwapBuffers();}, renderHUD);
#else
    frameBoundary(true, [] () {orig::SDL_GL_SwapBuffers();});
#endif
}

/* Override */ void SDL_GL_SwapWindow(SDL_Window* window)
{
    LINK_NAMESPACE_SDL2(SDL_GL_SwapWindow);

    if (GlobalState::isNative())
        return orig::SDL_GL_SwapWindow(window);

    debuglog(LCF_SDL | LCF_FRAME | LCF_OGL | LCF_WINDOW, __func__, " call.");

    /* Start the frame boundary and pass the function to draw */
#ifdef LIBTAS_ENABLE_HUD
    static RenderHUD_GL renderHUD;
    frameBoundary(true, [&] () {orig::SDL_GL_SwapWindow(window);}, renderHUD);
#else
    frameBoundary(true, [&] () {orig::SDL_GL_SwapWindow(window);});
#endif
}

void* SDL_GL_CreateContext(SDL_Window *window)
{
    DEBUGLOGCALL(LCF_SDL | LCF_OGL | LCF_WINDOW);
    LINK_NAMESPACE_SDL2(SDL_GL_CreateContext);

    void* context = orig::SDL_GL_CreateContext(window);

    /* We override this function to disable vsync,
     * except when using non deterministic timer.
     */
    if (shared_config.framerate > 0) {
        LINK_NAMESPACE_SDL2(SDL_GL_SetSwapInterval);
        orig::SDL_GL_SetSwapInterval(0);
    }

    /* Now that the context is created, we can init the screen capture */
    ScreenCapture::init(window);

    return context;
}

static int swapInterval = 0;

/* Override */ int SDL_GL_SetSwapInterval(int interval)
{
    debuglog(LCF_SDL | LCF_OGL | LCF_WINDOW, __func__, " call - setting to ", interval);
    LINK_NAMESPACE_SDL2(SDL_GL_SetSwapInterval);

    /* We save the interval if the game wants it later */
    swapInterval = interval;

    /* When using non deterministic timer, we let the game set vsync */
    if (shared_config.framerate == 0)
        return orig::SDL_GL_SetSwapInterval(interval);

    return 0; // Success
}

/* Override */ int SDL_GL_GetSwapInterval(void)
{
    DEBUGLOGCALL(LCF_SDL | LCF_OGL | LCF_WINDOW);
    return swapInterval;
}

/* Override */ SDL_Window* SDL_CreateWindow(const char* title, int x, int y, int w, int h, Uint32 flags){
    debuglog(LCF_SDL | LCF_WINDOW, __func__, " call - title: ", title, ", pos: (", x, ",", y, "), size: (", w, ",", h, "), flags: 0x", std::hex, flags, std::dec);
    LINK_NAMESPACE_SDL2(SDL_CreateWindow);

    WindowTitle::setOriginalTitle(title);

    /* Disable fullscreen */
    flags &= 0xFFFFFFFF ^ SDL_WINDOW_FULLSCREEN_DESKTOP;

    /* Disable hidden windows */
    flags &= 0xFFFFFFFF ^ SDL_WINDOW_HIDDEN;

    /* Disable high DPI mode */
    flags &= 0xFFFFFFFF ^ SDL_WINDOW_ALLOW_HIGHDPI;

    gameWindow = orig::SDL_CreateWindow(title, x, y, w, h, flags); // Save the game window

    if (flags & SDL_WINDOW_OPENGL) {
        game_info.video |= GameInfo::OPENGL;
        game_info.tosend = true;
    }
    else {
        game_info.video &= ~GameInfo::OPENGL;
        game_info.tosend = true;
    }

    LINK_NAMESPACE_SDL2(SDL_SetWindowTitle);
    WindowTitle::setUpdateFunc([] (const char* t) {orig::SDL_SetWindowTitle(gameWindow, t);});

    return gameWindow;
}

/* Override */ void SDL_DestroyWindow(SDL_Window* window){
    DEBUGLOGCALL(LCF_SDL | LCF_WINDOW);
    LINK_NAMESPACE_SDL2(SDL_DestroyWindow);

    orig::SDL_DestroyWindow(window);

    if (gameWindow == window)
        gameWindow = NULL;

#ifdef LIBTAS_ENABLE_AVDUMPING
    /* Destroy the AVEncoder object */
    if (avencoder)
        avencoder.reset(nullptr);
#endif

    ScreenCapture::fini();
}

/* Override */ Uint32 SDL_GetWindowID(SDL_Window* window){
    DEBUGLOGCALL(LCF_SDL | LCF_WINDOW);
    LINK_NAMESPACE_SDL2(SDL_GetWindowID);
    return orig::SDL_GetWindowID(window);
}

/* Override */ Uint32 SDL_GetWindowFlags(SDL_Window* window){
    DEBUGLOGCALL(LCF_SDL | LCF_WINDOW);
    LINK_NAMESPACE_SDL2(SDL_GetWindowFlags);
    Uint32 flags = orig::SDL_GetWindowFlags(window);
    flags |= SDL_WINDOW_INPUT_FOCUS | SDL_WINDOW_MOUSE_FOCUS;
    debuglog(LCF_SDL | LCF_WINDOW, "  flags: ", flags);
    return flags;
}

/* Override */ void SDL_SetWindowTitle(SDL_Window * window, const char *title)
{
    debuglog(LCF_SDL | LCF_WINDOW, __func__, " call with title ", title?title:"[null]");
    LINK_NAMESPACE_SDL2(SDL_SetWindowTitle);

    WindowTitle::setOriginalTitle(title);
    WindowTitle::setUpdateFunc([window] (const char* t) {orig::SDL_SetWindowTitle(window, t);});

    orig::SDL_SetWindowTitle(window, title);
}

/* Override */ void SDL_WM_SetCaption(const char *title, const char *icon)
{
    debuglog(LCF_SDL | LCF_WINDOW, __func__, " call with title ", title?title:"[null]");
    LINK_NAMESPACE_SDL1(SDL_WM_SetCaption);
    WindowTitle::setOriginalTitle(title);
    WindowTitle::setUpdateFunc([icon] (const char* t) {orig::SDL_WM_SetCaption(t, icon);});
    orig::SDL_WM_SetCaption(title, icon);
}

/* Override */ int SDL_SetWindowFullscreen(SDL_Window * window, Uint32 flags)
{
    debuglog(LCF_SDL | LCF_WINDOW, __func__, " call with flags ", flags);
    return 0; // success
}

/* Override */ void SDL_SetWindowBordered(SDL_Window * window, SDL_bool bordered)
{
    debuglog(LCF_SDL | LCF_WINDOW, __func__, " call with border ", bordered);
    /* Don't do anything */
}

/* Override */ SDL_Renderer *SDL_CreateRenderer(SDL_Window * window, int index, Uint32 flags)
{
    DEBUGLOGCALL(LCF_SDL | LCF_WINDOW);
    LINK_NAMESPACE_SDL2(SDL_CreateRenderer);

    if (flags & SDL_RENDERER_SOFTWARE)
        debuglog(LCF_SDL | LCF_WINDOW, "  flag SDL_RENDERER_SOFTWARE");
    if (flags & SDL_RENDERER_ACCELERATED)
        debuglog(LCF_SDL | LCF_WINDOW, "  flag SDL_RENDERER_ACCELERATED");
    if (flags & SDL_RENDERER_PRESENTVSYNC)
        debuglog(LCF_SDL | LCF_WINDOW, "   flag SDL_RENDERER_PRESENTVSYNC");
    if (flags & SDL_RENDERER_TARGETTEXTURE)
        debuglog(LCF_SDL | LCF_WINDOW, "   flag SDL_RENDERER_TARGETTEXTURE");

    SDL_Renderer* renderer = orig::SDL_CreateRenderer(window, index, flags);

    ScreenCapture::init(window);

    return renderer;
}

/* Override */ int SDL_CreateWindowAndRenderer(int width, int height,
        Uint32 window_flags, SDL_Window **window, SDL_Renderer **renderer)
{
    DEBUGLOGCALL(LCF_SDL | LCF_WINDOW);
    debuglog(LCF_SDL | LCF_WINDOW, "  size ", width, " x ", height);
    LINK_NAMESPACE_SDL2(SDL_CreateWindowAndRenderer);

    /* Disable fullscreen */
    window_flags &= 0xFFFFFFFF ^ SDL_WINDOW_FULLSCREEN_DESKTOP;

    /* Disable hidden windows */
    window_flags &= 0xFFFFFFFF ^ SDL_WINDOW_HIDDEN;

    /* Disable high DPI mode */
    window_flags &= 0xFFFFFFFF ^ SDL_WINDOW_ALLOW_HIGHDPI;

    int ret = orig::SDL_CreateWindowAndRenderer(width, height, window_flags, window, renderer);
    gameWindow = *window;

    game_info.video &= ~GameInfo::OPENGL;

    /* If we are going to save the screen when savestating, we need to init
     * our pixel access routine */
    ScreenCapture::init(gameWindow);

    return ret;
}

/* Override */ void SDL_RenderPresent(SDL_Renderer * renderer)
{
    LINK_NAMESPACE_SDL2(SDL_RenderPresent);

    if (GlobalState::isNative())
        return orig::SDL_RenderPresent(renderer);

    DEBUGLOGCALL(LCF_SDL | LCF_WINDOW);

    /* Start the frame boundary and pass the function to draw */
#ifdef LIBTAS_ENABLE_HUD
    static RenderHUD_SDL2 renderHUD;
    renderHUD.setRenderer(renderer);
    frameBoundary(true, [&] () {orig::SDL_RenderPresent(renderer);}, renderHUD);
#else
    frameBoundary(true, [&] () {orig::SDL_RenderPresent(renderer);});
#endif
}

/* Override */ void SDL_GetWindowPosition(SDL_Window * window, int *x, int *y)
{
    DEBUGLOGCALL(LCF_SDL | LCF_WINDOW);
    /* Always simulate the game window being on top-left corner, so that games
     * using global mouse coords do not desync on different window positions.
     */
    x = 0;
    y = 0;
}

/* Override */ void SDL_SetWindowSize(SDL_Window* window, int w, int h)
{
    DEBUGLOGCALL(LCF_SDL | LCF_WINDOW);
    debuglog(LCF_SDL | LCF_WINDOW, "    New size: ", w, " x ", h);
    LINK_NAMESPACE_SDL2(SDL_SetWindowSize);

    orig::SDL_SetWindowSize(window, w, h);

    ScreenCapture::reinit(gameWindow);

    /* We need to close the dumping if needed, and open a new one */
#ifdef LIBTAS_ENABLE_AVDUMPING
    if (shared_config.av_dumping) {
        debuglog(LCF_SDL | LCF_WINDOW | LCF_DUMP, "    Dumping is restarted");
        avencoder.reset(new AVEncoder(gameWindow, framecount));
    }
#endif

}

/* SDL 1.2 */
/* Override */ SDL1::SDL_Surface *SDL_SetVideoMode(int width, int height, int bpp, Uint32 flags)
{
    LINK_NAMESPACE_SDL1(SDL_SetVideoMode);

    debuglog(LCF_SDL | LCF_WINDOW, __func__, " call with size (", width, ",", height, "), bpp ", bpp, " and flags ", std::hex, flags, std::dec);

    /* Disable fullscreen */
    flags &= (0xFFFFFFFF ^ /*SDL_FULLSCREEN*/ 0x80000000);

    /* Call real function, but do not return yet */
    SDL1::SDL_Surface *surf = orig::SDL_SetVideoMode(width, height, bpp, flags);

    if (flags & /*SDL_OPENGL*/ 0x00000002) {
        game_info.video |= GameInfo::OPENGL;
        game_info.tosend = true;
    }
    else {
        game_info.video &= ~GameInfo::OPENGL;
    }

    /* If we are going to save the screen when savestating, we need to init
     * our pixel access routine */
    ScreenCapture::init(nullptr);

    return surf;
}

/* Override */ int SDL_Flip(SDL1::SDL_Surface *screen)
{
    LINK_NAMESPACE_SDL1(SDL_Flip);

    if (GlobalState::isNative())
        return orig::SDL_Flip(screen);

    debuglog(LCF_SDL | LCF_FRAME | LCF_WINDOW, __func__, " call.");

    /* Start the frame boundary and pass the function to draw */
#ifdef LIBTAS_ENABLE_HUD
    static RenderHUD_SDL1 renderHUD;
    frameBoundary(true, [&] () {orig::SDL_Flip(screen);}, renderHUD);
#else
    frameBoundary(true, [&] () {orig::SDL_Flip(screen);});
#endif

    return 0;
}

/* Override */ SDL1::SDL_GrabMode SDL_WM_GrabInput(SDL1::SDL_GrabMode mode)
{
    debuglog(LCF_SDL | LCF_KEYBOARD | LCF_MOUSE | LCF_WINDOW, __func__, " call with mode ", mode);
    static SDL1::SDL_GrabMode fakeGrab = SDL1::SDL_GRAB_OFF;
    if (mode != SDL1::SDL_GRAB_QUERY)
        fakeGrab = mode;
    return fakeGrab;
}


/* Override */ int SDL_GL_SetAttribute(SDL_GLattr attr, int value)
{
    debuglog(LCF_SDL | LCF_OGL | LCF_WINDOW, __func__, " call with attr ", attr, " and value ", value);
    LINK_NAMESPACE_SDL2(SDL_GL_SetAttribute);

    switch (attr) {
    case SDL_GL_CONTEXT_MAJOR_VERSION:
        game_info.opengl_major = value;
        game_info.tosend = true;
        break;
    case SDL_GL_CONTEXT_MINOR_VERSION:
        game_info.opengl_minor = value;
        game_info.tosend = true;
        break;
    case SDL_GL_CONTEXT_PROFILE_MASK:
        switch (value) {
        case SDL_GL_CONTEXT_PROFILE_CORE:
            game_info.opengl_profile = GameInfo::CORE;
            break;
        case SDL_GL_CONTEXT_PROFILE_COMPATIBILITY:
            game_info.opengl_profile = GameInfo::COMPATIBILITY;
            break;
        case SDL_GL_CONTEXT_PROFILE_ES:
            game_info.opengl_profile = GameInfo::ES;
            break;
        default:
            break;
        }
        game_info.tosend = true;
        break;
    default:
        break;
    }

    return orig::SDL_GL_SetAttribute(attr, value);
}

}
