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

#ifndef LIBTAS_ALLINPUTS_H_INCLUDED
#define LIBTAS_ALLINPUTS_H_INCLUDED

#include <X11/Xlib.h> // For the KeySym type
#include <X11/keysym.h>

#define ALLINPUTS_MAXKEY 16

/* Input structure that is filled by linTAS and send to libTAS every frame
 * Structure is inspired by SDL.
 */
class AllInputs {
    public:
        /* Keyboard state. Each element is a (X11) KeySym of a pressed key.
         * KeySym is a 4-byte integer struct containing a key symbol or meaning,
         * not the physical key itself, so it can contain misc or non-latin symbols
         * for example. It should work well with computers with
         * different keyboard layouts.
         * 
         * We choose a set of keys of a fixed size, instead of a bitmap of all
         * keys. The drawback is that we are limited by the number of 
         * simultaneous pressed keys, but the size is much smaller:
         * - Set: 16 keys * 4 bytes = 64 bytes
         * - Bitmap: ~2400 different KeySym / 8 = 300 bytes
         * and there would be a highly non-trivial translation between bitmap
         * position and equivalent KeySym.
         */
        KeySym keyboard[ALLINPUTS_MAXKEY];

        /* controller_axes[i][j] stores the state of axis j of controller i.
         * List of axes is coming from SDL GameController.
         * Maybe we should use our own enum and translate to SDL enum, in case
         * we must support other joystick libraries like plain joydev.
         */
        short controller_axes[4][6];

        /* controller_buttons[i] stores the bitmap state of buttons of
         * controller i. Bit j set means that button j is pressed.
         * We use a 2-byte integer here, meaning we can have at most
         * 16 buttons in a controller.
         * Again, the list of buttons is taken from SDL GameController. 
         */
        unsigned short controller_buttons[4];

        /* Empty the state, set axes to neutral position. */
        void emptyInputs();
};

#endif

