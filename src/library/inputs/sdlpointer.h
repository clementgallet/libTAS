/*
    Copyright 2015-2020 Clément Gallet <clement.gallet@ens-lyon.org>

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

#include "../../external/SDL1.h"
#include <SDL2/SDL.h>
#include "inputs.h"
#include "../global.h"
// #include "../sdl/sdlwindows.h" // SDL_Window

namespace libtas {

//typedef SDL_Cursor int;   /* Implementation dependent */

/**
 *  \brief Get the window which currently has mouse focus.
 */
OVERRIDE SDL_Window *SDL_GetMouseFocus(void);

/**
 *  \brief Retrieve the current state of the mouse.
 *
 *  The current button state is returned as a button bitmask, which can
 *  be tested using the SDL_BUTTON(X) macros, and x and y are set to the
 *  mouse cursor position relative to the focus window for the currently
 *  selected mouse.  You can pass NULL for either x or y.
 */
OVERRIDE Uint32 SDL_GetMouseState(int *x, int *y);

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
OVERRIDE Uint32 SDL_GetGlobalMouseState(int *x, int *y);

/**
 *  \brief Retrieve the relative state of the mouse.
 *
 *  The current button state is returned as a button bitmask, which can
 *  be tested using the SDL_BUTTON(X) macros, and x and y are set to the
 *  mouse deltas since the last call to SDL_GetRelativeMouseState().
 */
OVERRIDE Uint32 SDL_GetRelativeMouseState(int *x, int *y);

/**
 *  \brief Moves the mouse to the given position within the window.
 *
 *  \param window The window to move the mouse into, or NULL for the current mouse focus
 *  \param x The x coordinate within the window
 *  \param y The y coordinate within the window
 *
 *  \note This function generates a mouse motion event
 */
OVERRIDE void SDL_WarpMouseInWindow(SDL_Window * window, int x, int y);

/**
 *  \brief Moves the mouse to the given position in global screen space.
 *
 *  \param x The x coordinate
 *  \param y The y coordinate
 *  \return 0 on success, -1 on error (usually: unsupported by a platform).
 *
 *  \note This function generates a mouse motion event
 */
OVERRIDE int SDL_WarpMouseGlobal(int x, int y);

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
OVERRIDE int SDL_CaptureMouse(SDL_bool enabled);

/**
 *  \brief Query whether relative mouse mode is enabled.
 *
 *  \sa SDL_SetRelativeMouseMode()
 */
OVERRIDE SDL_bool SDL_GetRelativeMouseMode(void);

#if 0
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
extern DECLSPEC SDL_Cursor *SDLCALL SDL_CreateCursor(const Uint8 * data,
                                                     const Uint8 * mask,
                                                     int w, int h, int hot_x,
                                                     int hot_y);

/**
 *  \brief Create a color cursor.
 *
 *  \sa SDL_FreeCursor()
 */
extern DECLSPEC SDL_Cursor *SDLCALL SDL_CreateColorCursor(SDL_Surface *surface,
                                                          int hot_x,
                                                          int hot_y);

/**
 *  \brief Create a system cursor.
 *
 *  \sa SDL_FreeCursor()
 */
extern DECLSPEC SDL_Cursor *SDLCALL SDL_CreateSystemCursor(SDL_SystemCursor id);

/**
 *  \brief Set the active cursor.
 */
extern DECLSPEC void SDLCALL SDL_SetCursor(SDL_Cursor * cursor);

/**
 *  \brief Return the active cursor.
 */
extern DECLSPEC SDL_Cursor *SDLCALL SDL_GetCursor(void);

/**
 *  \brief Return the default cursor.
 */
extern DECLSPEC SDL_Cursor *SDLCALL SDL_GetDefaultCursor(void);

/**
 *  \brief Frees a cursor created with SDL_CreateCursor().
 *
 *  \sa SDL_CreateCursor()
 */
extern DECLSPEC void SDLCALL SDL_FreeCursor(SDL_Cursor * cursor);

#endif
/**
 *  \brief Toggle whether or not the cursor is shown.
 *
 *  \param toggle 1 to show the cursor, 0 to hide it, -1 to query the current
 *                state.
 *
 *  \return 1 if the cursor is shown, or 0 if the cursor is hidden.
 */
OVERRIDE int SDL_ShowCursor(int toggle);

/**
 *  Used as a mask when testing buttons in buttonstate.
 *   - Button 1:  Left mouse button
 *   - Button 2:  Middle mouse button
 *   - Button 3:  Right mouse button
 */

}

#endif
