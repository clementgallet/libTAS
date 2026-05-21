/*
    Copyright 2015-2026 Clément Gallet <clement.gallet@ens-lyon.org>

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

#ifndef LIBTAS_SDLWINDOWS_H_INCL
#define LIBTAS_SDLWINDOWS_H_INCL

#include "hook.h"
#include "../external/SDL1.h"
#include "../external/SDL2.h"
#include "../external/SDL3.h"

namespace libtas {

namespace sdl {
    /* SDL2 game window (we suppose there is only one) */
    extern SDL_Window* gameSDLWindow;    
}

/**
 * \brief Swap the OpenGL buffers for a window, if double-buffering is
 *        supported.
 */
void sdl2::SDL_GL_SwapWindow(SDL_Window* window);

/**
 * Update a window with OpenGL rendering.
 *
 * This is used with double-buffered OpenGL contexts, which are the default.
 *
 * On macOS, make sure you bind 0 to the draw framebuffer before swapping the
 * window, otherwise nothing will happen. If you aren't using
 * glBindFramebuffer(), this is the default and you won't have to do anything
 * extra.
 *
 * \param window the window to change.
 * \returns true on success or false on failure; call SDL_GetError() for more
 *          information.
 *
 * \threadsafety This function should only be called on the main thread.
 *
 * \since This function is available since SDL 3.2.0.
 */
bool sdl3::SDL_GL_SwapWindow(SDL_Window *window);

/**
 *  \brief Create a window with the specified position, dimensions, and flags.
 *
 *  \param title The title of the window, in UTF-8 encoding.
 *  \param x     The x position of the window, ::SDL_WINDOWPOS_CENTERED, or
 *               ::SDL_WINDOWPOS_UNDEFINED.
 *  \param y     The y position of the window, ::SDL_WINDOWPOS_CENTERED, or
 *               ::SDL_WINDOWPOS_UNDEFINED.
 *  \param w     The width of the window, in screen coordinates.
 *  \param h     The height of the window, in screen coordinates.
 *  \param flags The flags for the window, a mask of any of the following:
 *               ::SDL_WINDOW_FULLSCREEN,    ::SDL_WINDOW_OPENGL,
 *               ::SDL_WINDOW_HIDDEN,        ::SDL_WINDOW_BORDERLESS,
 *               ::SDL_WINDOW_RESIZABLE,     ::SDL_WINDOW_MAXIMIZED,
 *               ::SDL_WINDOW_MINIMIZED,     ::SDL_WINDOW_INPUT_GRABBED,
 *               ::SDL_WINDOW_ALLOW_HIGHDPI.
 *
 *  \return The id of the window created, or zero if window creation failed.
 *
 *  If the window is created with the SDL_WINDOW_ALLOW_HIGHDPI flag, its size
 *  in pixels may differ from its size in screen coordinates on platforms with
 *  high-DPI support (e.g. iOS and Mac OS X). Use SDL_GetWindowSize() to query
 *  the client area's size in screen coordinates, and SDL_GL_GetDrawableSize()
 *  or SDL_GetRendererOutputSize() to query the drawable size in pixels.
 *
 *  \sa SDL_DestroyWindow()
 */
SDL_Window* sdl2::SDL_CreateWindow(const char* title, int x, int y, int w, int h, Uint32 flags);

/**
 * Create a window with the specified dimensions and flags.
 *
 * `flags` may be any of the following OR'd together:
 *
 * - `SDL_WINDOW_FULLSCREEN`: fullscreen window at desktop resolution
 * - `SDL_WINDOW_OPENGL`: window usable with an OpenGL context
 * - `SDL_WINDOW_OCCLUDED`: window partially or completely obscured by another
 *   window
 * - `SDL_WINDOW_HIDDEN`: window is not visible
 * - `SDL_WINDOW_BORDERLESS`: no window decoration
 * - `SDL_WINDOW_RESIZABLE`: window can be resized
 * - `SDL_WINDOW_MINIMIZED`: window is minimized
 * - `SDL_WINDOW_MAXIMIZED`: window is maximized
 * - `SDL_WINDOW_MOUSE_GRABBED`: window has grabbed mouse focus
 * - `SDL_WINDOW_INPUT_FOCUS`: window has input focus
 * - `SDL_WINDOW_MOUSE_FOCUS`: window has mouse focus
 * - `SDL_WINDOW_EXTERNAL`: window not created by SDL
 * - `SDL_WINDOW_MODAL`: window is modal
 * - `SDL_WINDOW_HIGH_PIXEL_DENSITY`: window uses high pixel density back
 *   buffer if possible
 * - `SDL_WINDOW_MOUSE_CAPTURE`: window has mouse captured (unrelated to
 *   MOUSE_GRABBED)
 * - `SDL_WINDOW_ALWAYS_ON_TOP`: window should always be above others
 * - `SDL_WINDOW_UTILITY`: window should be treated as a utility window, not
 *   showing in the task bar and window list
 * - `SDL_WINDOW_TOOLTIP`: window should be treated as a tooltip and does not
 *   get mouse or keyboard focus, requires a parent window
 * - `SDL_WINDOW_POPUP_MENU`: window should be treated as a popup menu,
 *   requires a parent window
 * - `SDL_WINDOW_KEYBOARD_GRABBED`: window has grabbed keyboard input
 * - `SDL_WINDOW_VULKAN`: window usable with a Vulkan instance
 * - `SDL_WINDOW_METAL`: window usable with a Metal instance
 * - `SDL_WINDOW_TRANSPARENT`: window with transparent buffer
 * - `SDL_WINDOW_NOT_FOCUSABLE`: window should not be focusable
 *
 * The SDL_Window is implicitly shown if SDL_WINDOW_HIDDEN is not set.
 *
 * On Apple's macOS, you **must** set the NSHighResolutionCapable Info.plist
 * property to YES, otherwise you will not receive a High-DPI OpenGL canvas.
 *
 * The window pixel size may differ from its window coordinate size if the
 * window is on a high pixel density display. Use SDL_GetWindowSize() to query
 * the client area's size in window coordinates, and
 * SDL_GetWindowSizeInPixels() or SDL_GetRenderOutputSize() to query the
 * drawable size in pixels. Note that the drawable size can vary after the
 * window is created and should be queried again if you get an
 * SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED event.
 *
 * If the window is created with any of the SDL_WINDOW_OPENGL or
 * SDL_WINDOW_VULKAN flags, then the corresponding LoadLibrary function
 * (SDL_GL_LoadLibrary or SDL_Vulkan_LoadLibrary) is called and the
 * corresponding UnloadLibrary function is called by SDL_DestroyWindow().
 *
 * If SDL_WINDOW_VULKAN is specified and there isn't a working Vulkan driver,
 * SDL_CreateWindow() will fail, because SDL_Vulkan_LoadLibrary() will fail.
 *
 * If SDL_WINDOW_METAL is specified on an OS that does not support Metal,
 * SDL_CreateWindow() will fail.
 *
 * If you intend to use this window with an SDL_Renderer, you should use
 * SDL_CreateWindowAndRenderer() instead of this function, to avoid window
 * flicker.
 *
 * On non-Apple devices, SDL requires you to either not link to the Vulkan
 * loader or link to a dynamic library version. This limitation may be removed
 * in a future version of SDL.
 *
 * \param title the title of the window, in UTF-8 encoding.
 * \param w the width of the window.
 * \param h the height of the window.
 * \param flags 0, or one or more SDL_WindowFlags OR'd together.
 * \returns the window that was created or NULL on failure; call
 *          SDL_GetError() for more information.
 *
 * \threadsafety This function should only be called on the main thread.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_CreateWindowAndRenderer
 * \sa SDL_CreatePopupWindow
 * \sa SDL_CreateWindowWithProperties
 * \sa SDL_DestroyWindow
 */
SDL_Window * sdl3::SDL_CreateWindow(const char *title, int w, int h, SDL_WindowFlags flags);

/**
 *  \brief Get the numeric ID of a window, for logging purposes.
 */
OVERRIDE Uint32 SDL_GetWindowID(SDL_Window* window);

/**
 *  \brief Get a window from a stored ID, or NULL if it doesn't exist.
 */
OVERRIDE SDL_Window* SDL_GetWindowFromID(Uint32 id);

/**
 *  \brief Get the window flags.
 */
Uint32 sdl2::SDL_GetWindowFlags(SDL_Window* window);

/**
 * Get the window flags.
 *
 * \param window the window to query.
 * \returns a mask of the SDL_WindowFlags associated with `window`.
 *
 * \threadsafety This function should only be called on the main thread.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_CreateWindow
 * \sa SDL_HideWindow
 * \sa SDL_MaximizeWindow
 * \sa SDL_MinimizeWindow
 * \sa SDL_SetWindowFullscreen
 * \sa SDL_SetWindowMouseGrab
 * \sa SDL_ShowWindow
 */
sdl3::SDL_WindowFlags sdl3::SDL_GetWindowFlags(SDL_Window *window);

/**
 *  \brief Set the title of a window, in UTF-8 format.
 *
 *  \sa SDL_GetWindowTitle()
 */
OVERRIDE void SDL_SetWindowTitle(SDL_Window * window, const char *title);

/** SDL 1.2
 * Sets the title and icon text of the display window (UTF-8 encoded)
 */
OVERRIDE void SDL_WM_SetCaption(const char *title, const char *icon);

/**
 *  \brief Set the border state of a window.
 *
 *  This will add or remove the window's SDL_WINDOW_BORDERLESS flag and
 *  add or remove the border from the actual window. This is a no-op if the
 *  window's border already matches the requested state.
 *
 *  \param window The window of which to change the border state.
 *  \param bordered SDL_FALSE to remove border, SDL_TRUE to add border.
 *
 *  \note You can't change the border state of a fullscreen window.
 *
 *  \sa SDL_GetWindowFlags()
 */
OVERRIDE void SDL_SetWindowBordered(SDL_Window * window, SDL_bool bordered);

/**
 * Set the user-resizable state of a window.
 *
 * This will add or remove the window's `SDL_WINDOW_RESIZABLE` flag and
 * allow/disallow user resizing of the window. This is a no-op if the window's
 * resizable state already matches the requested state.
 *
 * You can't change the resizable state of a fullscreen window.
 *
 * \param window the window of which to change the resizable state.
 * \param resizable SDL_TRUE to allow resizing, SDL_FALSE to disallow.
 *
 * \since This function is available since SDL 2.0.5.
 *
 * \sa SDL_GetWindowFlags
 */
OVERRIDE void SDL_SetWindowResizable(SDL_Window * window, SDL_bool resizable);

/**
 *  \brief Set a window's fullscreen state.
 *
 *  \return 0 on success, or -1 if setting the display mode failed.
 *
 *  \sa SDL_SetWindowDisplayMode()
 *  \sa SDL_GetWindowDisplayMode()
 */
int sdl2::SDL_SetWindowFullscreen(SDL_Window * window, Uint32 flags);

/**
 * Request that the window's fullscreen state be changed.
 *
 * By default a window in fullscreen state uses borderless fullscreen desktop
 * mode, but a specific exclusive display mode can be set using
 * SDL_SetWindowFullscreenMode().
 *
 * On some windowing systems this request is asynchronous and the new
 * fullscreen state may not have have been applied immediately upon the return
 * of this function. If an immediate change is required, call SDL_SyncWindow()
 * to block until the changes have taken effect.
 *
 * When the window state changes, an SDL_EVENT_WINDOW_ENTER_FULLSCREEN or
 * SDL_EVENT_WINDOW_LEAVE_FULLSCREEN event will be emitted. Note that, as this
 * is just a request, it can be denied by the windowing system.
 *
 * \param window the window to change.
 * \param fullscreen true for fullscreen mode, false for windowed mode.
 * \returns true on success or false on failure; call SDL_GetError() for more
 *          information.
 *
 * \threadsafety This function should only be called on the main thread.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_GetWindowFullscreenMode
 * \sa SDL_SetWindowFullscreenMode
 * \sa SDL_SyncWindow
 * \sa SDL_WINDOW_FULLSCREEN
 */
bool sdl3::SDL_SetWindowFullscreen(SDL_Window *window, bool fullscreen);

/**
 *  \brief Create an OpenGL context for use with an OpenGL window, and make it
 *         current.
 *
 *  \sa SDL_GL_DeleteContext()
 */
OVERRIDE void* SDL_GL_CreateContext(SDL_Window *window);

/**
 *  \brief Delete an OpenGL context.
 *
 *  \sa SDL_GL_CreateContext()
 */
OVERRIDE void SDL_GL_DeleteContext(SDL_GLContext context);

/**
 * Delete an OpenGL context.
 *
 * \param context the OpenGL context to be deleted.
 * \returns true on success or false on failure; call SDL_GetError() for more
 *          information.
 *
 * \threadsafety This function should only be called on the main thread.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_GL_CreateContext
 */
OVERRIDE bool SDL_GL_DestroyContext(SDL_GLContext context);

/**
 *  \brief Set the swap interval for the current OpenGL context.
 *
 *  \param interval 0 for immediate updates, 1 for updates synchronized with the
 *                  vertical retrace. If the system supports it, you may
 *                  specify -1 to allow late swaps to happen immediately
 *                  instead of waiting for the next retrace.
 *
 *  \return 0 on success, or -1 if setting the swap interval is not supported.
 *
 *  \sa SDL_GL_GetSwapInterval()
 */
OVERRIDE int SDL_GL_SetSwapInterval(int interval);

/**
 *  \brief Get the swap interval for the current OpenGL context.
 *
 *  \return 0 if there is no vertical retrace synchronization, 1 if the buffer
 *          swap is synchronized with the vertical retrace, and -1 if late
 *          swaps happen immediately instead of waiting for the next retrace.
 *          If the system can't determine the swap interval, or there isn't a
 *          valid current context, this will return 0 as a safe default.
 *
 *  \sa SDL_GL_SetSwapInterval()
 */
OVERRIDE int SDL_GL_GetSwapInterval(void);

/**
 *  \brief Destroy a window.
 */
OVERRIDE void SDL_DestroyWindow(SDL_Window* window);

/**
 *  \brief Create a window and default renderer
 *
 *  \param width    The width of the window
 *  \param height   The height of the window
 *  \param window_flags The flags used to create the window
 *  \param window   A pointer filled with the window, or NULL on error
 *  \param renderer A pointer filled with the renderer, or NULL on error
 *
 *  \return 0 on success, or -1 on error
 */
// OVERRIDE int SDL_CreateWindowAndRenderer(
//                  int width, int height, Uint32 window_flags,
//                  SDL_Window **window, SDL_Renderer **renderer);

/**
 * Create a window and default renderer.
 *
 * \param title the title of the window, in UTF-8 encoding.
 * \param width the width of the window.
 * \param height the height of the window.
 * \param window_flags the flags used to create the window (see
 *                     SDL_CreateWindow()).
 * \param window a pointer filled with the window, or NULL on error.
 * \param renderer a pointer filled with the renderer, or NULL on error.
 * \returns true on success or false on failure; call SDL_GetError() for more
 *          information.
 *
 * \threadsafety This function should only be called on the main thread.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_CreateRenderer
 * \sa SDL_CreateWindow
 */
bool sdl3::SDL_CreateWindowAndRenderer(const char *title, int width, int height, SDL_WindowFlags window_flags, SDL_Window **window, SDL_Renderer **renderer);

/**
 *  \brief Set the position of a window.
 *
 *  \param window   The window to reposition.
 *  \param x        The x coordinate of the window in screen coordinates, or
 *                  ::SDL_WINDOWPOS_CENTERED or ::SDL_WINDOWPOS_UNDEFINED.
 *  \param y        The y coordinate of the window in screen coordinates, or
 *                  ::SDL_WINDOWPOS_CENTERED or ::SDL_WINDOWPOS_UNDEFINED.
 *
 *  \note The window coordinate origin is the upper left of the display.
 *
 *  \sa SDL_GetWindowPosition()
 */
void sdl2::SDL_SetWindowPosition(SDL_Window * window, int x, int y);

/**
 * Request that the window's position be set.
 *
 * If the window is in an exclusive fullscreen or maximized state, this
 * request has no effect.
 *
 * This can be used to reposition fullscreen-desktop windows onto a different
 * display, however, as exclusive fullscreen windows are locked to a specific
 * display, they can only be repositioned programmatically via
 * SDL_SetWindowFullscreenMode().
 *
 * On some windowing systems this request is asynchronous and the new
 * coordinates may not have have been applied immediately upon the return of
 * this function. If an immediate change is required, call SDL_SyncWindow() to
 * block until the changes have taken effect.
 *
 * When the window position changes, an SDL_EVENT_WINDOW_MOVED event will be
 * emitted with the window's new coordinates. Note that the new coordinates
 * may not match the exact coordinates requested, as some windowing systems
 * can restrict the position of the window in certain scenarios (e.g.
 * constraining the position so the window is always within desktop bounds).
 * Additionally, as this is just a request, it can be denied by the windowing
 * system.
 *
 * \param window the window to reposition.
 * \param x the x coordinate of the window, or `SDL_WINDOWPOS_CENTERED` or
 *          `SDL_WINDOWPOS_UNDEFINED`.
 * \param y the y coordinate of the window, or `SDL_WINDOWPOS_CENTERED` or
 *          `SDL_WINDOWPOS_UNDEFINED`.
 * \returns true on success or false on failure; call SDL_GetError() for more
 *          information.
 *
 * \threadsafety This function should only be called on the main thread.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_GetWindowPosition
 * \sa SDL_SyncWindow
 */
bool sdl3::SDL_SetWindowPosition(SDL_Window *window, int x, int y);

/**
 * Get the position of a window.
 *
 * This is the current position of the window as last reported by the
 * windowing system.
 *
 * If you do not need the value for one of the positions a NULL may be passed
 * in the `x` or `y` parameter.
 *
 * \param window the window to query.
 * \param x a pointer filled in with the x position of the window, may be
 *          NULL.
 * \param y a pointer filled in with the y position of the window, may be
 *          NULL.
 * \returns true on success or false on failure; call SDL_GetError() for more
 *          information.
 *
 * \threadsafety This function should only be called on the main thread.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_SetWindowPosition
 */
OVERRIDE bool SDL_GetWindowPosition(SDL_Window *window, int *x, int *y);

/**
 *  \brief Set the size of a window's client area.
 *
 *  \param window   The window to resize.
 *  \param w        The width of the window, in screen coordinates. Must be >0.
 *  \param h        The height of the window, in screen coordinates. Must be >0.
 *
 *  \note Fullscreen windows automatically match the size of the display mode,
 *        and you should use SDL_SetWindowDisplayMode() to change their size.
 *
 *  The window size in screen coordinates may differ from the size in pixels, if
 *  the window was created with SDL_WINDOW_ALLOW_HIGHDPI on a platform with
 *  high-dpi support (e.g. iOS or OS X). Use SDL_GL_GetDrawableSize() or
 *  SDL_GetRendererOutputSize() to get the real client area size in pixels.
 *
 *  \sa SDL_GetWindowSize()
 *  \sa SDL_SetWindowDisplayMode()
 */
void sdl2::SDL_SetWindowSize(SDL_Window* window, int w, int h);

/**
 * Request that the size of a window's client area be set.
 *
 * If the window is in a fullscreen or maximized state, this request has no
 * effect.
 *
 * To change the exclusive fullscreen mode of a window, use
 * SDL_SetWindowFullscreenMode().
 *
 * On some windowing systems, this request is asynchronous and the new window
 * size may not have have been applied immediately upon the return of this
 * function. If an immediate change is required, call SDL_SyncWindow() to
 * block until the changes have taken effect.
 *
 * When the window size changes, an SDL_EVENT_WINDOW_RESIZED event will be
 * emitted with the new window dimensions. Note that the new dimensions may
 * not match the exact size requested, as some windowing systems can restrict
 * the window size in certain scenarios (e.g. constraining the size of the
 * content area to remain within the usable desktop bounds). Additionally, as
 * this is just a request, it can be denied by the windowing system.
 *
 * \param window the window to change.
 * \param w the width of the window, must be > 0.
 * \param h the height of the window, must be > 0.
 * \returns true on success or false on failure; call SDL_GetError() for more
 *          information.
 *
 * \threadsafety This function should only be called on the main thread.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_GetWindowSize
 * \sa SDL_SetWindowFullscreenMode
 * \sa SDL_SyncWindow
 */
bool sdl3::SDL_SetWindowSize(SDL_Window *window, int w, int h);

/**
 * Get the size of a window's client area.
 *
 * NULL can safely be passed as the `w` or `h` parameter if the width or
 * height value is not desired.
 *
 * The window size in screen coordinates may differ from the size in pixels,
 * if the window was created with `SDL_WINDOW_ALLOW_HIGHDPI` on a platform
 * with high-dpi support (e.g. iOS or macOS). Use SDL_GL_GetDrawableSize(),
 * SDL_Vulkan_GetDrawableSize(), or SDL_GetRendererOutputSize() to get the
 * real client area size in pixels.
 *
 * \param window the window to query the width and height from
 * \param w a pointer filled in with the width of the window, in screen
 *          coordinates, may be NULL
 * \param h a pointer filled in with the height of the window, in screen
 *          coordinates, may be NULL
 *
 * \since This function is available since SDL 2.0.0.
 *
 * \sa SDL_GL_GetDrawableSize
 * \sa SDL_Vulkan_GetDrawableSize
 * \sa SDL_SetWindowSize
 */
void sdl2::SDL_GetWindowSize(SDL_Window * window, int *w, int *h);

/**
 * Get the size of a window's client area.
 *
 * The window pixel size may differ from its window coordinate size if the
 * window is on a high pixel density display. Use SDL_GetWindowSizeInPixels()
 * or SDL_GetRenderOutputSize() to get the real client area size in pixels.
 *
 * \param window the window to query the width and height from.
 * \param w a pointer filled in with the width of the window, may be NULL.
 * \param h a pointer filled in with the height of the window, may be NULL.
 * \returns true on success or false on failure; call SDL_GetError() for more
 *          information.
 *
 * \threadsafety This function should only be called on the main thread.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_GetRenderOutputSize
 * \sa SDL_GetWindowSizeInPixels
 * \sa SDL_SetWindowSize
 */
bool sdl3::SDL_GetWindowSize(SDL_Window *window, int *w, int *h);

/**
 * Set up a video mode with the specified width, height and bits-per-pixel.
 *
 * If 'bpp' is 0, it is treated as the current display bits per pixel.
 *
 * If SDL_ANYFORMAT is set in 'flags', the SDL library will try to set the
 * requested bits-per-pixel, but will return whatever video pixel format is
 * available.  The default is to emulate the requested pixel format if it
 * is not natively available.
 *
 * If SDL_HWSURFACE is set in 'flags', the video surface will be placed in
 * video memory, if possible, and you may have to call SDL_LockSurface()
 * in order to access the raw framebuffer.  Otherwise, the video surface
 * will be created in system memory.
 *
 * If SDL_ASYNCBLIT is set in 'flags', SDL will try to perform rectangle
 * updates asynchronously, but you must always lock before accessing pixels.
 * SDL will wait for updates to complete before returning from the lock.
 *
 * If SDL_HWPALETTE is set in 'flags', the SDL library will guarantee
 * that the colors set by SDL_SetColors() will be the colors you get.
 * Otherwise, in 8-bit mode, SDL_SetColors() may not be able to set all
 * of the colors exactly the way they are requested, and you should look
 * at the video surface structure to determine the actual palette.
 * If SDL cannot guarantee that the colors you request can be set,
 * i.e. if the colormap is shared, then the video surface may be created
 * under emulation in system memory, overriding the SDL_HWSURFACE flag.
 *
 * If SDL_FULLSCREEN is set in 'flags', the SDL library will try to set
 * a fullscreen video mode.  The default is to create a windowed mode
 * if the current graphics system has a window manager.
 * If the SDL library is able to set a fullscreen video mode, this flag
 * will be set in the surface that is returned.
 *
 * If SDL_DOUBLEBUF is set in 'flags', the SDL library will try to set up
 * two surfaces in video memory and swap between them when you call
 * SDL_Flip().  This is usually slower than the normal single-buffering
 * scheme, but prevents "tearing" artifacts caused by modifying video
 * memory while the monitor is refreshing.  It should only be used by
 * applications that redraw the entire screen on every update.
 *
 * If SDL_RESIZABLE is set in 'flags', the SDL library will allow the
 * window manager, if any, to resize the window at runtime.  When this
 * occurs, SDL will send a SDL_VIDEORESIZE event to you application,
 * and you must respond to the event by re-calling SDL_SetVideoMode()
 * with the requested size (or another size that suits the application).
 *
 * If SDL_NOFRAME is set in 'flags', the SDL library will create a window
 * without any title bar or frame decoration.  Fullscreen video modes have
 * this flag set automatically.
 *
 * This function returns the video framebuffer surface, or NULL if it fails.
 *
 * If you rely on functionality provided by certain video flags, check the
 * flags of the returned surface to make sure that functionality is available.
 * SDL will fall back to reduced functionality if the exact flags you wanted
 * are not available.
 */
OVERRIDE sdl1::SDL_Surface *SDL_SetVideoMode(int width, int height, int bpp, Uint32 flags);

/**
 * Swap the OpenGL buffers, if double-buffering is supported.
 */
OVERRIDE void SDL_GL_SwapBuffers(void);

/** @name SDL_Update Functions
 * These functions should not be called while 'screen' is locked.
 */
/*@{*/
/**
 * Makes sure the given list of rectangles is updated on the given screen.
 */
OVERRIDE void SDL_UpdateRects(sdl1::SDL_Surface *screen, int numrects, sdl1::SDL_Rect *rects);

/**
 * If 'x', 'y', 'w' and 'h' are all 0, SDL_UpdateRect will update the entire
 * screen.
 */
OVERRIDE void SDL_UpdateRect(sdl1::SDL_Surface *screen, Sint32 x, Sint32 y, Uint32 w, Uint32 h);
/*@}*/

/**
 * Sets the color key (transparent pixel) in a blittable surface.
 * If 'flag' is SDL_SRCCOLORKEY (optionally OR'd with SDL_RLEACCEL),
 * 'key' will be the transparent pixel in the source image of a blit.
 * SDL_RLEACCEL requests RLE acceleration for the surface if present,
 * and removes RLE acceleration if absent.
 * If 'flag' is 0, this function clears any current color key.
 * This function returns 0, or -1 if there was an error.
 */
OVERRIDE int SDL_SetColorKey(sdl2::SDL_Surface *surface, int flag, Uint32 key);

/**
 * On hardware that supports double-buffering, this function sets up a flip
 * and returns.  The hardware will wait for vertical retrace, and then swap
 * video buffers before the next video surface blit or lock will return.
 * On hardware that doesn not support double-buffering, this is equivalent
 * to calling SDL_UpdateRect(screen, 0, 0, 0, 0);
 * The SDL_DOUBLEBUF flag must have been passed to SDL_SetVideoMode() when
 * setting the video mode for this function to perform hardware flipping.
 * This function returns 0 if successful, or -1 if there was an error.
 */
OVERRIDE int SDL_Flip(sdl1::SDL_Surface *screen);

/**
 * This function allows you to set and query the input grab state of
 * the application.  It returns the new input grab state.
 *
 * Grabbing means that the mouse is confined to the application window,
 * and nearly all keyboard input is passed directly to the application,
 * and not interpreted by a window manager, if any.
 */
OVERRIDE sdl1::SDL_GrabMode SDL_WM_GrabInput(sdl1::SDL_GrabMode mode);

/**
 *  \brief Set an OpenGL window attribute before window creation.
 */
OVERRIDE int SDL_GL_SetAttribute(sdl2::SDL_GLattr attr, int value);

/**
 *  \brief Copy the window surface to the screen.
 *
 *  \return 0 on success, or -1 on error.
 */
OVERRIDE int SDL_UpdateWindowSurface(SDL_Window * window);

/**
 *  \brief Copy a number of rectangles on the window surface to the screen.
 *
 *  \return 0 on success, or -1 on error.
 */
OVERRIDE int SDL_UpdateWindowSurfaceRects(SDL_Window * window,
                                                         const sdl2::SDL_Rect * rects,
                                                         int numrects);

}

#endif
