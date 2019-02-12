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
#include "DeterministicTimer.h"
#include "WindowTitle.h"
#include "encoding/AVEncoder.h"
#include "EventQueue.h"
#include "openglwrappers.h" // checkMesa()
// #include <string>
// #include <X11/Xlib.h> // Display

namespace libtas {

DECLARE_ORIG_POINTER(SDL_GL_SwapWindow);
DECLARE_ORIG_POINTER(SDL_CreateWindow);
DECLARE_ORIG_POINTER(SDL_GetWindowID);
DECLARE_ORIG_POINTER(SDL_GetWindowFlags);
DECLARE_ORIG_POINTER(SDL_SetWindowTitle);
DEFINE_ORIG_POINTER(SDL_WM_SetCaption);
DECLARE_ORIG_POINTER(SDL_GL_CreateContext);
DECLARE_ORIG_POINTER(SDL_GL_DeleteContext);
DECLARE_ORIG_POINTER(SDL_GL_SetSwapInterval);
DECLARE_ORIG_POINTER(SDL_DestroyWindow);
DECLARE_ORIG_POINTER(SDL_SetWindowSize);
DECLARE_ORIG_POINTER(SDL_CreateWindowAndRenderer);
DEFINE_ORIG_POINTER(SDL_SetVideoMode);
DEFINE_ORIG_POINTER(SDL_GL_SwapBuffers);
DEFINE_ORIG_POINTER(SDL_Flip);
DECLARE_ORIG_POINTER(SDL_SetColorKey);
DEFINE_ORIG_POINTER(SDL_UpdateRects);
DEFINE_ORIG_POINTER(SDL_UpdateRect);
DECLARE_ORIG_POINTER(SDL_GL_SetAttribute);

/* SDL 1.2 */
/* Override */ void SDL_GL_SwapBuffers(void)
{
    LINK_NAMESPACE_SDL1(SDL_GL_SwapBuffers);

    if (GlobalState::isNative())
        return orig::SDL_GL_SwapBuffers();

    debuglog(LCF_SDL | LCF_OGL | LCF_WINDOW, __func__, " call.");

    /* Start the frame boundary and pass the function to draw */
#ifdef LIBTAS_ENABLE_HUD
    static RenderHUD_GL renderHUD_GL;
    frameBoundary(true, [] () {orig::SDL_GL_SwapBuffers();}, renderHUD_GL, false);
#else
    frameBoundary(true, [] () {orig::SDL_GL_SwapBuffers();}, false);
#endif
}

/* Override */ void SDL_GL_SwapWindow(SDL_Window* window)
{
    LINK_NAMESPACE_SDL2(SDL_GL_SwapWindow);

    if (GlobalState::isNative())
        return orig::SDL_GL_SwapWindow(window);

    debuglog(LCF_SDL | LCF_OGL | LCF_WINDOW, __func__, " call.");

    /* Start the frame boundary and pass the function to draw */
#ifdef LIBTAS_ENABLE_HUD
    static RenderHUD_GL renderHUD_GL;
    frameBoundary(true, [&] () {orig::SDL_GL_SwapWindow(window);}, renderHUD_GL, false);
#else
    frameBoundary(true, [&] () {orig::SDL_GL_SwapWindow(window);}, false);
#endif
}

void* SDL_GL_CreateContext(SDL_Window *window)
{
    DEBUGLOGCALL(LCF_SDL | LCF_OGL | LCF_WINDOW);
    LINK_NAMESPACE_SDL2(SDL_GL_CreateContext);

    void* context;
    NATIVECALL(context = orig::SDL_GL_CreateContext(window));

    /* We override this function to disable vsync,
     * except when using non deterministic timer.
     */
    if (!(shared_config.debug_state & SharedConfig::DEBUG_UNCONTROLLED_TIME)) {
        LINK_NAMESPACE_SDL2(SDL_GL_SetSwapInterval);
        orig::SDL_GL_SetSwapInterval(0);
        debuglog(LCF_WINDOW, "Disable vsync !!");
    }

    /* If the context creation failed, we stop here */
    if (!context) {
        return context;
    }

    /* Now that the context is created, we can init the screen capture */
    ScreenCapture::init();

#ifdef LIBTAS_ENABLE_HUD
    /* Create texture and fbo in the OSD */
    RenderHUD_GL::init();
#endif

    /* Alerting the user if software rendering is not active */
    checkMesa();

    return context;
}

void SDL_GL_DeleteContext(SDL_GLContext context)
{
    DEBUGLOGCALL(LCF_SDL | LCF_OGL | LCF_WINDOW);
    LINK_NAMESPACE_SDL2(SDL_GL_DeleteContext);

    #ifdef LIBTAS_ENABLE_HUD
        /* Delete texture and fbo in the OSD */
        RenderHUD_GL::fini();
    #endif

    orig::SDL_GL_DeleteContext(context);
}

static int swapInterval = 0;

/* Override */ int SDL_GL_SetSwapInterval(int interval)
{
    debuglog(LCF_SDL | LCF_OGL | LCF_WINDOW, __func__, " call - setting to ", interval);
    LINK_NAMESPACE_SDL2(SDL_GL_SetSwapInterval);

    /* We save the interval if the game wants it later */
    swapInterval = interval;

    /* When using non deterministic timer, we let the game set vsync */
    if (shared_config.debug_state & SharedConfig::DEBUG_UNCONTROLLED_TIME) {
        debuglog(LCF_WINDOW, "Set swap interval !!");
        int ret = orig::SDL_GL_SetSwapInterval(interval);
        debuglog(LCF_WINDOW, "   return ", ret);
        return ret;
        // return orig::SDL_GL_SetSwapInterval(interval);
    }

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

    if (shared_config.screen_width && w > shared_config.screen_width)
        w = shared_config.screen_width;

    if (shared_config.screen_height && h > shared_config.screen_height)
        h = shared_config.screen_height;

    gameSDLWindow = orig::SDL_CreateWindow(title, x, y, w, h, flags); // Save the game window

    if (flags & SDL_WINDOW_OPENGL) {
        game_info.video |= GameInfo::OPENGL;
        game_info.tosend = true;
    }
    else {
        game_info.video &= ~GameInfo::OPENGL;
        game_info.tosend = true;
    }

    LINK_NAMESPACE_SDL2(SDL_SetWindowTitle);
    WindowTitle::setUpdateFunc([] (const char* t) {orig::SDL_SetWindowTitle(gameSDLWindow, t);});

    /* Push the few events that generated by a window creation */
    struct timespec time = detTimer.getTicks();
    int timestamp = time.tv_sec * 1000 + time.tv_nsec / 1000000;

    /* Get window id */
    LINK_NAMESPACE_SDL2(SDL_GetWindowID);
    Uint32 windowID = orig::SDL_GetWindowID(gameSDLWindow);

    SDL_Event event;
    event.type = SDL_WINDOWEVENT;
    event.window.windowID = windowID;
    event.window.timestamp = timestamp;
    event.window.event = SDL_WINDOWEVENT_SHOWN;
    sdlEventQueue.insert(&event);

    event.window.event = SDL_WINDOWEVENT_MOVED;
    sdlEventQueue.insert(&event);

    event.window.event = SDL_WINDOWEVENT_SHOWN;
    sdlEventQueue.insert(&event);

    event.window.event = SDL_WINDOWEVENT_ENTER;
    sdlEventQueue.insert(&event);

    event.window.event = SDL_WINDOWEVENT_FOCUS_GAINED;
    sdlEventQueue.insert(&event);

    return gameSDLWindow;
}

/* Override */ void SDL_DestroyWindow(SDL_Window* window){
    DEBUGLOGCALL(LCF_SDL | LCF_WINDOW);
    LINK_NAMESPACE_SDL2(SDL_DestroyWindow);

    orig::SDL_DestroyWindow(window);

    if (gameSDLWindow == window)
        gameSDLWindow = nullptr;

    /* Destroy the AVEncoder object */
    if (avencoder)
        avencoder.reset(nullptr);

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

    game_info.video |= GameInfo::SDL2_RENDERER;

    int ret = orig::SDL_CreateWindowAndRenderer(width, height, window_flags, window, renderer);
    gameSDLWindow = *window;

    /* If we are going to save the screen when savestating, we need to init
     * our pixel access routine */
    ScreenCapture::init();

    return ret;
}

/* Override */ void SDL_SetWindowPosition(SDL_Window*, int, int)
{
    DEBUGLOGCALL(LCF_SDL | LCF_WINDOW);
    /* Preventing the game to change the window position */
}

/* Override */ void SDL_GetWindowPosition(SDL_Window *, int *x, int *y)
{
    DEBUGLOGCALL(LCF_SDL | LCF_WINDOW);
    /* Always simulate the game window being on top-left corner, so that games
     * using global mouse coords do not desync on different window positions.
     */
    *x = 0;
    *y = 0;
}

/* Override */ void SDL_SetWindowSize(SDL_Window* window, int w, int h)
{
    DEBUGLOGCALL(LCF_SDL | LCF_WINDOW);
    debuglog(LCF_SDL | LCF_WINDOW, "    New size: ", w, " x ", h);
    LINK_NAMESPACE_SDL2(SDL_SetWindowSize);

    NATIVECALL(orig::SDL_SetWindowSize(window, w, h));

    int old_width, old_height;
    ScreenCapture::getDimensions(old_width, old_height);
    if ((old_width != w) || (old_height != h)) {
        ScreenCapture::resize(w, h);

        /* We need to close the dumping if needed, and open a new one */
        if (shared_config.av_dumping) {
            debuglog(LCF_SDL | LCF_WINDOW | LCF_DUMP, "    Dumping is restarted");
            avencoder.reset(new AVEncoder());
        }
    }
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
    ScreenCapture::init();

#ifdef LIBTAS_ENABLE_HUD
    /* Create texture and fbo in the OSD */
    RenderHUD_GL::init();
#endif

    return surf;
}

/* Override */ int SDL_SetColorKey(SDL_Surface *surface, int flag, Uint32 key)
{
    debuglog(LCF_SDL | LCF_WINDOW, __func__, " call with flag ", flag, " and key ", key);
    LINK_NAMESPACE_SDLX(SDL_SetColorKey);
    return orig::SDL_SetColorKey(surface, flag, key);
}

/* Override */ int SDL_Flip(SDL1::SDL_Surface *screen)
{
    LINK_NAMESPACE_SDL1(SDL_Flip);

    if (GlobalState::isNative())
        return orig::SDL_Flip(screen);

    debuglog(LCF_SDL | LCF_WINDOW, __func__, " call.");

    /* Start the frame boundary and pass the function to draw */
#ifdef LIBTAS_ENABLE_HUD
    static RenderHUD_SDL1 renderHUD;
    frameBoundary(true, [&] () {orig::SDL_Flip(screen);}, renderHUD, false);
#else
    frameBoundary(true, [&] () {orig::SDL_Flip(screen);}, false);
#endif

    return 0;
}

OVERRIDE void SDL_UpdateRects(SDL1::SDL_Surface *screen, int numrects, SDL1::SDL_Rect *rects)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE_SDL1(SDL_UpdateRects);
        return orig::SDL_UpdateRects(screen, numrects, rects);
    }

    LINK_NAMESPACE_SDL1(SDL_UpdateRect);
    debuglog(LCF_SDL | LCF_WINDOW, __func__, " call with ", numrects, " rects");

    /* Start the frame boundary and pass the function to draw */
#ifdef LIBTAS_ENABLE_HUD
    static RenderHUD_SDL1 renderHUD;
    frameBoundary(true, [&] () {orig::SDL_UpdateRect(screen, 0, 0, 0, 0);}, renderHUD, true);
#else
    frameBoundary(true, [&] () {orig::SDL_UpdateRect(screen, 0, 0, 0, 0);}, true);
#endif
}

/* Override */ void SDL_UpdateRect(SDL1::SDL_Surface *screen, Sint32 x, Sint32 y, Uint32 w, Uint32 h)
{
    LINK_NAMESPACE_SDL1(SDL_UpdateRect);

    if (GlobalState::isNative())
        return orig::SDL_UpdateRect(screen, x, y, w, h);

    debuglogstdio(LCF_SDL | LCF_WINDOW, "%s call with pos (%d,%d) and size (%u,%u)", __func__, x, y, w, h);

    /* Start the frame boundary and pass the function to draw */
#ifdef LIBTAS_ENABLE_HUD
    static RenderHUD_SDL1 renderHUD;
    frameBoundary(true, [&] () {orig::SDL_UpdateRect(screen, 0, 0, 0, 0);}, renderHUD, true);
#else
    frameBoundary(true, [&] () {orig::SDL_UpdateRect(screen, 0, 0, 0, 0);}, true);
#endif
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
