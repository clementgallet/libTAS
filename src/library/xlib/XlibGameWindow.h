/*
    Copyright 2015-2024 Clément Gallet <clement.gallet@ens-lyon.org>

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

#ifndef LIBTAS_XLIBGAMEWINDOW_H_INCLUDED
#define LIBTAS_XLIBGAMEWINDOW_H_INCLUDED

#include <X11/X.h>
#include <X11/Xlib.h>
#include <xcb/xcb.h>

namespace libtas {

/* Handle xlib windows and determine which window to listen to. */
namespace XlibGameWindow {

/* Push a new window identifier */
void push(Window w);

/* Promote an identifier to become the game window */
void promote(Window w);

/* Returns the current game window */
Window get();

/* Delete a window id from the list and if game window, promote the next one
 * in the list */ 
void pop(Window w);

/* Send the window id to libtas program to listen for events */
void sendId(Window w);

/* Returns if the window is a root window */
bool isRootWindow(Display *display, Window w);
bool isRootWindow(xcb_connection_t *c, Window w);

/* Returns if the window is a top-level window */
bool isTopLevel(Display *display, Window w);

/* Get and set coords of a window */
void setCoords(Window w, int x, int y);
void getCoords(Window w, int* x, int* y);

};

}

#endif
