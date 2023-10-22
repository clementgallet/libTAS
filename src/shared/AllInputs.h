/*
    Copyright 2015-2023 Clément Gallet <clement.gallet@ens-lyon.org>

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

#include "SingleInput.h"

#include <array>
#include <set>
#include <cstdint>

/* Input structure that is filled by libTAS and send to the game every frame
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

        std::array<uint32_t,MAXKEYS> keyboard;

        /* Pointer coordinates relative to the origin of the game window */
        int pointer_x;
        int pointer_y;

        /* Absolute or relative mode */
        unsigned int pointer_mode;

        /* Pointer buttons */
        unsigned int pointer_mask;

        /* controller_axes[i][j] stores the state of axis j of controller i */
        std::array<std::array<short, MAXAXES>,MAXJOYS> controller_axes;

        /* controller_buttons[i] stores the bitmap state of buttons of
         * controller i. Bit j set means that button j is pressed.
         * We use a 2-byte integer here, meaning we can have at most
         * 16 buttons in a controller.
         */
        std::array<unsigned short,MAXJOYS> controller_buttons;

        /* Flags */
        uint32_t flags;

        /* Framerate values */
        uint32_t framerate_den, framerate_num;

        /* Realtime values */
        uint32_t realtime_sec, realtime_nsec;

        /* Operator needed for comparing movies */
        inline bool operator==(const AllInputs& other) const
        {
            return ((keyboard == other.keyboard) &&
                (pointer_x == other.pointer_x) &&
                (pointer_y == other.pointer_y) &&
                (pointer_mask == other.pointer_mask) &&
                (controller_axes == other.controller_axes) &&
                (controller_buttons == other.controller_buttons) &&
                (flags == other.flags) &&
                (framerate_den == other.framerate_den) &&
                (framerate_num == other.framerate_num) &&
                (realtime_sec == other.realtime_sec) &&
                (realtime_nsec == other.realtime_nsec));
        }

        /* Empty the state, set axes to neutral position. */
        void emptyInputs();

        /* Check if a controller has default values */
        bool isDefaultController(int j) const;

        /* Get the value of a single input, or 0/1 if the input is set/not set */
        int getInput(const SingleInput &si) const;

        /* Set a single input in the inputs to a certain value */
        void setInput(const SingleInput &si, int value);

        /* Toggle a single input from the inputs and return the new value */
        int toggleInput(const SingleInput &si);

        /* Extract all single inputs and insert them in the set */
        void extractInputs(std::set<SingleInput> &set) const;

};

#endif
