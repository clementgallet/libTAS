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

#ifndef LIBTAS_SDLPOINTER_H_INCL
#define LIBTAS_SDLPOINTER_H_INCL

#include "inputs.h"

#include "hook.h"
#include "../external/SDL1.h"
#include "../external/SDL2.h"
#include "../external/SDL3.h"

namespace libtas {

/**
 * Return whether a mouse is currently connected.
 *
 * \returns true if a mouse is connected, false otherwise.
 *
 * \threadsafety This function should only be called on the main thread.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_GetMice
 */
OVERRIDE bool SDL_HasMouse(void);

/**
 * Get a list of currently connected mice.
 *
 * Note that this will include any device or virtual driver that includes
 * mouse functionality, including some game controllers, KVM switches, etc.
 * You should wait for input from a device before you consider it actively in
 * use.
 *
 * \param count a pointer filled in with the number of mice returned, may be
 *              NULL.
 * \returns a 0 terminated array of mouse instance IDs or NULL on failure;
 *          call SDL_GetError() for more information. This should be freed
 *          with SDL_free() when it is no longer needed.
 *
 * \threadsafety This function should only be called on the main thread.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_GetMouseNameForID
 * \sa SDL_HasMouse
 */
OVERRIDE sdl3::SDL_MouseID * SDL_GetMice(int *count);

/**
 *  \brief Get the window which currently has mouse focus.
 */
OVERRIDE SDL_Window *SDL_GetMouseFocus(void);

/* Both SDL2 and SDL3 functions have the same name but a different signature */

/**
 *  \brief Retrieve the current state of the mouse.
 *
 *  The current button state is returned as a button bitmask, which can
 *  be tested using the SDL_BUTTON(X) macros, and x and y are set to the
 *  mouse cursor position relative to the focus window for the currently
 *  selected mouse.  You can pass NULL for either x or y.
 */
Uint32 sdl2::SDL_GetMouseState(int *x, int *y);

/**
 * Query SDL's cache for the synchronous mouse button state and the
 * window-relative SDL-cursor position.
 *
 * This function returns the cached synchronous state as SDL understands it
 * from the last pump of the event queue.
 *
 * To query the platform for immediate asynchronous state, use
 * SDL_GetGlobalMouseState.
 *
 * Passing non-NULL pointers to `x` or `y` will write the destination with
 * respective x or y coordinates relative to the focused window.
 *
 * In Relative Mode, the SDL-cursor's position usually contradicts the
 * platform-cursor's position as manually calculated from
 * SDL_GetGlobalMouseState() and SDL_GetWindowPosition.
 *
 * \param x a pointer to receive the SDL-cursor's x-position from the focused
 *          window's top left corner, can be NULL if unused.
 * \param y a pointer to receive the SDL-cursor's y-position from the focused
 *          window's top left corner, can be NULL if unused.
 * \returns a 32-bit bitmask of the button state that can be bitwise-compared
 *          against the SDL_BUTTON_MASK(X) macro.
 *
 * \threadsafety This function should only be called on the main thread.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_GetGlobalMouseState
 * \sa SDL_GetRelativeMouseState
 */

sdl3::SDL_MouseButtonFlags sdl3::SDL_GetMouseState(float *x, float *y);

/**
 *  \brief Get the current state of the mouse, in relation to the desktop
 *
 *  This works just like SDL_GetMouseState(), but the coordinates will be
 *  reported relative to the top-left of the desktop. This can be useful if
 *  you need to track the mouse outside of a specific window and
 *  SDL_CaptureMouse() doesn't fit your needs. For example, it could be
 *  useful if you need to track the mouse while dragging a window, where
 *  coordinates relative to a window might not be in sync at all times.
 *
 *  \note SDL_GetMouseState() returns the mouse position as SDL understands
 *        it from the last pump of the event queue. This function, however,
 *        queries the OS for the current mouse position, and as such, might
 *        be a slightly less efficient function. Unless you know what you're
 *        doing and have a good reason to use this function, you probably want
 *        SDL_GetMouseState() instead.
 *
 *  \param x Returns the current X coord, relative to the desktop. Can be NULL.
 *  \param y Returns the current Y coord, relative to the desktop. Can be NULL.
 *  \return The current button state as a bitmask, which can be tested using the SDL_BUTTON(X) macros.
 *
 *  \sa SDL_GetMouseState
 */
Uint32 sdl2::SDL_GetGlobalMouseState(int *x, int *y);

/**
 * Query the platform for the asynchronous mouse button state and the
 * desktop-relative platform-cursor position.
 *
 * This function immediately queries the platform for the most recent
 * asynchronous state, more costly than retrieving SDL's cached state in
 * SDL_GetMouseState().
 *
 * Passing non-NULL pointers to `x` or `y` will write the destination with
 * respective x or y coordinates relative to the desktop.
 *
 * In Relative Mode, the platform-cursor's position usually contradicts the
 * SDL-cursor's position as manually calculated from SDL_GetMouseState() and
 * SDL_GetWindowPosition.
 *
 * This function can be useful if you need to track the mouse outside of a
 * specific window and SDL_CaptureMouse() doesn't fit your needs. For example,
 * it could be useful if you need to track the mouse while dragging a window,
 * where coordinates relative to a window might not be in sync at all times.
 *
 * \param x a pointer to receive the platform-cursor's x-position from the
 *          desktop's top left corner, can be NULL if unused.
 * \param y a pointer to receive the platform-cursor's y-position from the
 *          desktop's top left corner, can be NULL if unused.
 * \returns a 32-bit bitmask of the button state that can be bitwise-compared
 *          against the SDL_BUTTON_MASK(X) macro.
 *
 * \threadsafety This function should only be called on the main thread.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_CaptureMouse
 * \sa SDL_GetMouseState
 * \sa SDL_GetGlobalMouseState
 */
sdl3::SDL_MouseButtonFlags sdl3::SDL_GetGlobalMouseState(float *x, float *y);

/**
 *  \brief Retrieve the relative state of the mouse.
 *
 *  The current button state is returned as a button bitmask, which can
 *  be tested using the SDL_BUTTON(X) macros, and x and y are set to the
 *  mouse deltas since the last call to SDL_GetRelativeMouseState().
 */
Uint32 sdl2::SDL_GetRelativeMouseState(int *x, int *y);

/**
 * Query SDL's cache for the synchronous mouse button state and accumulated
 * mouse delta since last call.
 *
 * This function returns the cached synchronous state as SDL understands it
 * from the last pump of the event queue.
 *
 * To query the platform for immediate asynchronous state, use
 * SDL_GetGlobalMouseState.
 *
 * Passing non-NULL pointers to `x` or `y` will write the destination with
 * respective x or y deltas accumulated since the last call to this function
 * (or since event initialization).
 *
 * This function is useful for reducing overhead by processing relative mouse
 * inputs in one go per-frame instead of individually per-event, at the
 * expense of losing the order between events within the frame (e.g. quickly
 * pressing and releasing a button within the same frame).
 *
 * \param x a pointer to receive the x mouse delta accumulated since last
 *          call, can be NULL if unused.
 * \param y a pointer to receive the y mouse delta accumulated since last
 *          call, can be NULL if unused.
 * \returns a 32-bit bitmask of the button state that can be bitwise-compared
 *          against the SDL_BUTTON_MASK(X) macro.
 *
 * \threadsafety This function should only be called on the main thread.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_GetMouseState
 * \sa SDL_GetGlobalMouseState
 */
sdl3::SDL_MouseButtonFlags sdl3::SDL_GetRelativeMouseState(float *x, float *y);

/**
 *  \brief Moves the mouse to the given position within the window.
 *
 *  \param window The window to move the mouse into, or NULL for the current mouse focus
 *  \param x The x coordinate within the window
 *  \param y The y coordinate within the window
 *
 *  \note This function generates a mouse motion event
 */
void sdl2::SDL_WarpMouseInWindow(SDL_Window * window, int x, int y);

/**
 * Move the mouse cursor to the given position within the window.
 *
 * This function generates a mouse motion event if relative mode is not
 * enabled. If relative mode is enabled, you can force mouse events for the
 * warp by setting the SDL_HINT_MOUSE_RELATIVE_WARP_MOTION hint.
 *
 * Note that this function will appear to succeed, but not actually move the
 * mouse when used over Microsoft Remote Desktop.
 *
 * \param window the window to move the mouse into, or NULL for the current
 *               mouse focus.
 * \param x the x coordinate within the window.
 * \param y the y coordinate within the window.
 *
 * \threadsafety This function should only be called on the main thread.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_WarpMouseGlobal
 */
void sdl3::SDL_WarpMouseInWindow(SDL_Window *window, float x, float y);

/**
 *  \brief Moves the mouse to the given position in global screen space.
 *
 *  \param x The x coordinate
 *  \param y The y coordinate
 *  \return 0 on success, -1 on error (usually: unsupported by a platform).
 *
 *  \note This function generates a mouse motion event
 */
int sdl2::SDL_WarpMouseGlobal(int x, int y);

/**
 * Move the mouse to the given position in global screen space.
 *
 * This function generates a mouse motion event.
 *
 * A failure of this function usually means that it is unsupported by a
 * platform.
 *
 * Note that this function will appear to succeed, but not actually move the
 * mouse when used over Microsoft Remote Desktop.
 *
 * \param x the x coordinate.
 * \param y the y coordinate.
 * \returns true on success or false on failure; call SDL_GetError() for more
 *          information.
 *
 * \threadsafety This function should only be called on the main thread.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_WarpMouseInWindow
 */
bool sdl3::SDL_WarpMouseGlobal(float x, float y);

/**
 * Set the position of the mouse cursor (generates a mouse motion event)
 */
OVERRIDE void SDL_WarpMouse(Uint16 x, Uint16 y);

/**
 *  \brief Set relative mouse mode.
 *
 *  \param enabled Whether or not to enable relative mode
 *
 *  \return 0 on success, or -1 if relative mode is not supported.
 *
 *  While the mouse is in relative mode, the cursor is hidden, and the
 *  driver will try to report continuous motion in the current window.
 *  Only relative motion events will be delivered, the mouse position
 *  will not change.
 *
 *  \note This function will flush any pending mouse motion.
 *
 *  \sa SDL_GetRelativeMouseMode()
 */
OVERRIDE int SDL_SetRelativeMouseMode(SDL_bool enabled);

/**
 * Set relative mouse mode for a window.
 *
 * While the window has focus and relative mouse mode is enabled, the cursor
 * is hidden, the mouse position is constrained to the window, and SDL will
 * report continuous relative mouse motion even if the mouse is at the edge of
 * the window.
 *
 * If you'd like to keep the mouse position fixed while in relative mode you
 * can use SDL_SetWindowMouseRect(). If you'd like the cursor to be at a
 * specific location when relative mode ends, you should use
 * SDL_WarpMouseInWindow() before disabling relative mode.
 *
 * This function will flush any pending mouse motion for this window.
 *
 * \param window the window to change.
 * \param enabled true to enable relative mode, false to disable.
 * \returns true on success or false on failure; call SDL_GetError() for more
 *          information.
 *
 * \threadsafety This function should only be called on the main thread.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_GetWindowRelativeMouseMode
 */
OVERRIDE bool SDL_SetWindowRelativeMouseMode(SDL_Window *window, bool enabled);

/**
 *  \brief Capture the mouse, to track input outside an SDL window.
 *
 *  \param enabled Whether or not to enable capturing
 *
 *  Capturing enables your app to obtain mouse events globally, instead of
 *  just within your window. Not all video targets support this function.
 *  When capturing is enabled, the current window will get all mouse events,
 *  but unlike relative mode, no change is made to the cursor and it is
 *  not restrained to your window.
 *
 *  This function may also deny mouse input to other windows--both those in
 *  your application and others on the system--so you should use this
 *  function sparingly, and in small bursts. For example, you might want to
 *  track the mouse while the user is dragging something, until the user
 *  releases a mouse button. It is not recommended that you capture the mouse
 *  for long periods of time, such as the entire time your app is running.
 *
 *  While captured, mouse events still report coordinates relative to the
 *  current (foreground) window, but those coordinates may be outside the
 *  bounds of the window (including negative values). Capturing is only
 *  allowed for the foreground window. If the window loses focus while
 *  capturing, the capture will be disabled automatically.
 *
 *  While capturing is enabled, the current window will have the
 *  SDL_WINDOW_MOUSE_CAPTURE flag set.
 *
 *  \return 0 on success, or -1 if not supported.
 */
int sdl2::SDL_CaptureMouse(SDL_bool enabled);

/**
 * Capture the mouse and to track input outside an SDL window.
 *
 * Capturing enables your app to obtain mouse events globally, instead of just
 * within your window. Not all video targets support this function. When
 * capturing is enabled, the current window will get all mouse events, but
 * unlike relative mode, no change is made to the cursor and it is not
 * restrained to your window.
 *
 * This function may also deny mouse input to other windows--both those in
 * your application and others on the system--so you should use this function
 * sparingly, and in small bursts. For example, you might want to track the
 * mouse while the user is dragging something, until the user releases a mouse
 * button. It is not recommended that you capture the mouse for long periods
 * of time, such as the entire time your app is running. For that, you should
 * probably use SDL_SetWindowRelativeMouseMode() or SDL_SetWindowMouseGrab(),
 * depending on your goals.
 *
 * While captured, mouse events still report coordinates relative to the
 * current (foreground) window, but those coordinates may be outside the
 * bounds of the window (including negative values). Capturing is only allowed
 * for the foreground window. If the window loses focus while capturing, the
 * capture will be disabled automatically.
 *
 * While capturing is enabled, the current window will have the
 * `SDL_WINDOW_MOUSE_CAPTURE` flag set.
 *
 * Please note that SDL will attempt to "auto capture" the mouse while the
 * user is pressing a button; this is to try and make mouse behavior more
 * consistent between platforms, and deal with the common case of a user
 * dragging the mouse outside of the window. This means that if you are
 * calling SDL_CaptureMouse() only to deal with this situation, you do not
 * have to (although it is safe to do so). If this causes problems for your
 * app, you can disable auto capture by setting the
 * `SDL_HINT_MOUSE_AUTO_CAPTURE` hint to zero.
 *
 * \param enabled true to enable capturing, false to disable.
 * \returns true on success or false on failure; call SDL_GetError() for more
 *          information.
 *
 * \threadsafety This function should only be called on the main thread.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_GetGlobalMouseState
 */
bool sdl3::SDL_CaptureMouse(bool enabled);

/**
 *  \brief Query whether relative mouse mode is enabled.
 *
 *  \sa SDL_SetRelativeMouseMode()
 */
OVERRIDE SDL_bool SDL_GetRelativeMouseMode(void);

/**
 * Query whether relative mouse mode is enabled for a window.
 *
 * \param window the window to query.
 * \returns true if relative mode is enabled for a window or false otherwise.
 *
 * \threadsafety This function should only be called on the main thread.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_SetWindowRelativeMouseMode
 */
OVERRIDE bool SDL_GetWindowRelativeMouseMode(SDL_Window *window);

/**
 *  \brief Create a cursor, using the specified bitmap data and
 *         mask (in MSB format).
 *
 *  The cursor width must be a multiple of 8 bits.
 *
 *  The cursor is created in black and white according to the following:
 *  <table>
 *  <tr><td> data </td><td> mask </td><td> resulting pixel on screen </td></tr>
 *  <tr><td>  0   </td><td>  1   </td><td> White </td></tr>
 *  <tr><td>  1   </td><td>  1   </td><td> Black </td></tr>
 *  <tr><td>  0   </td><td>  0   </td><td> Transparent </td></tr>
 *  <tr><td>  1   </td><td>  0   </td><td> Inverted color if possible, black
 *                                         if not. </td></tr>
 *  </table>
 *
 *  \sa SDL_FreeCursor()
 */
OVERRIDE SDL_Cursor *SDL_CreateCursor(const Uint8 * data,
                                                     const Uint8 * mask,
                                                     int w, int h, int hot_x,
                                                     int hot_y);

/**
 *  \brief Create a color cursor.
 *
 *  \sa SDL_FreeCursor()
 */
OVERRIDE SDL_Cursor *SDL_CreateColorCursor(sdl2::SDL_Surface *surface,
                                                          int hot_x,
                                                          int hot_y);

/**
 *  \brief Create a system cursor.
 *
 *  \sa SDL_FreeCursor()
 */
OVERRIDE SDL_Cursor *SDL_CreateSystemCursor(sdl2::SDL_SystemCursor id);

/**
 *  \brief Set the active cursor.
 */
OVERRIDE bool SDL_SetCursor(SDL_Cursor * cursor);

/**
 *  \brief Return the active cursor.
 */
OVERRIDE SDL_Cursor *SDL_GetCursor(void);

/**
 *  \brief Return the default cursor.
 */
OVERRIDE SDL_Cursor *SDL_GetDefaultCursor(void);

/**
 *  \brief Frees a cursor created with SDL_CreateCursor().
 *
 *  \sa SDL_CreateCursor()
 */
OVERRIDE void SDL_FreeCursor(SDL_Cursor * cursor);

/**
 * Free a previously-created cursor.
 *
 * Use this function to free cursor resources created with SDL_CreateCursor(),
 * SDL_CreateColorCursor() or SDL_CreateSystemCursor().
 *
 * \param cursor the cursor to free.
 *
 * \threadsafety This function should only be called on the main thread.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_CreateColorCursor
 * \sa SDL_CreateCursor
 * \sa SDL_CreateSystemCursor
 */
OVERRIDE void SDL_DestroyCursor(SDL_Cursor *cursor);

/**
 *  \brief Toggle whether or not the cursor is shown.
 *
 *  \param toggle 1 to show the cursor, 0 to hide it, -1 to query the current
 *                state.
 *
 *  \return 1 if the cursor is shown, or 0 if the cursor is hidden.
 */
int sdl2::SDL_ShowCursor(int toggle);

/**
 * Show the cursor.
 *
 * \returns true on success or false on failure; call SDL_GetError() for more
 *          information.
 *
 * \threadsafety This function should only be called on the main thread.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_CursorVisible
 * \sa SDL_HideCursor
 */
bool sdl3::SDL_ShowCursor(void);

/**
 * Hide the cursor.
 *
 * \returns true on success or false on failure; call SDL_GetError() for more
 *          information.
 *
 * \threadsafety This function should only be called on the main thread.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_CursorVisible
 * \sa SDL_ShowCursor
 */
OVERRIDE bool SDL_HideCursor(void);

/**
 * Return whether the cursor is currently being shown.
 *
 * \returns `true` if the cursor is being shown, or `false` if the cursor is
 *          hidden.
 *
 * \threadsafety This function should only be called on the main thread.
 *
 * \since This function is available since SDL 3.2.0.
 *
 * \sa SDL_HideCursor
 * \sa SDL_ShowCursor
 */
OVERRIDE bool SDL_CursorVisible(void);

/** 
 * Set a window's input grab mode.
 *  
 * When input is grabbed, the mouse is confined to the window. This function
 * will also grab the keyboard if `SDL_HINT_GRAB_KEYBOARD` is set. To grab the
 * keyboard without also grabbing the mouse, use SDL_SetWindowKeyboardGrab().
 *
 * If the caller enables a grab while another window is currently grabbed, the
 * other window loses its grab in favor of the caller's window.
 *
 * \param window the window for which the input grab mode should be set
 * \param grabbed SDL_TRUE to grab input or SDL_FALSE to release input
 *
 * \since This function is available since SDL 2.0.0.
 *
 * \sa SDL_GetGrabbedWindow
 * \sa SDL_GetWindowGrab
 */
OVERRIDE void SDL_SetWindowGrab(SDL_Window * window, SDL_bool grabbed);

/**
 * Set a window's mouse grab mode.
 *
 * Mouse grab confines the mouse cursor to the window.
 *
 * \param window The window for which the mouse grab mode should be set.
 * \param grabbed This is SDL_TRUE to grab mouse, and SDL_FALSE to release.
 *
 * \since This function is available since SDL 2.0.16.
 *
 * \sa SDL_GetWindowMouseGrab
 * \sa SDL_SetWindowKeyboardGrab
 * \sa SDL_SetWindowGrab
 */
OVERRIDE void SDL_SetWindowMouseGrab(SDL_Window * window, SDL_bool grabbed);

/**
 * Get a window's input grab mode.
 *
 * \param window the window to query
 * \returns SDL_TRUE if input is grabbed, SDL_FALSE otherwise.
 *
 * \since This function is available since SDL 2.0.0.
 *
 * \sa SDL_SetWindowGrab
 */
OVERRIDE SDL_bool SDL_GetWindowGrab(SDL_Window * window);

/**
 * Get a window's mouse grab mode.
 *
 * \param window the window to query
 * \returns SDL_TRUE if mouse is grabbed, and SDL_FALSE otherwise.
 *
 * \since This function is available since SDL 2.0.16.
 *
 * \sa SDL_SetWindowKeyboardGrab
 * \sa SDL_GetWindowGrab
 */
OVERRIDE SDL_bool SDL_GetWindowMouseGrab(SDL_Window * window);

/**
 * Get the window that currently has an input grab enabled.
 *
 * \returns the window if input is grabbed or NULL otherwise.
 *
 * \since This function is available since SDL 2.0.4.
 *
 * \sa SDL_GetWindowGrab
 * \sa SDL_SetWindowGrab
 */
OVERRIDE SDL_Window* SDL_GetGrabbedWindow(void);

}

#endif
