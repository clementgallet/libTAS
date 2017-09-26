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
#include <array>

/* Input structure that is filled by linTAS and send to libTAS every frame
 * Structure is inspired by SDL.
 */
class AllInputs {
    public:

        static const int MAXKEYS = 16;
        static const int MAXJOYS = 4;
        static const int MAXAXES = 6;

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
        std::array<KeySym,MAXKEYS> keyboard;

        /* Pointer coordinates relative to the origin of the game window */
        int pointer_x;
        int pointer_y;

        /* Pointer buttons */
        unsigned int pointer_mask;

        enum {
            AXIS_LEFTX,
            AXIS_LEFTY,
            AXIS_RIGHTX,
            AXIS_RIGHTY,
            AXIS_TRIGGERLEFT,
            AXIS_TRIGGERRIGHT
        };

        /* controller_axes[i][j] stores the state of axis j of controller i */
        std::array<std::array<short, MAXAXES>,MAXJOYS> controller_axes;

        enum {
            BUTTON_A,
            BUTTON_B,
            BUTTON_X,
            BUTTON_Y,
            BUTTON_BACK,
            BUTTON_GUIDE,
            BUTTON_START,
            BUTTON_LEFTSTICK,
            BUTTON_RIGHTSTICK,
            BUTTON_LEFTSHOULDER,
            BUTTON_RIGHTSHOULDER,
            BUTTON_DPAD_UP,
            BUTTON_DPAD_DOWN,
            BUTTON_DPAD_LEFT,
            BUTTON_DPAD_RIGHT
        };

        /* controller_buttons[i] stores the bitmap state of buttons of
         * controller i. Bit j set means that button j is pressed.
         * We use a 2-byte integer here, meaning we can have at most
         * 16 buttons in a controller.
         */
        std::array<unsigned short,MAXJOYS> controller_buttons;

        /* Operator needed for comparing movies */
        inline bool operator==(const AllInputs& other) const
        {
            return ((keyboard == other.keyboard) &&
                (pointer_x == other.pointer_x) &&
                (pointer_y == other.pointer_y) &&
                (pointer_mask == other.pointer_mask) &&
                (controller_axes == other.controller_axes) &&
                (controller_buttons == other.controller_buttons));
        }

        /* Empty the state, set axes to neutral position. */
        void emptyInputs();
};

#endif
