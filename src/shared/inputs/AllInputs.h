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
#include "ControllerInputs.h"
#include "MouseInputs.h"
#include "MiscInputs.h"

#include <array>
#include <set>
#include <cstdint>
#include <memory>

/* Input structure that is filled by libTAS and send to the game every frame
 * Structure is inspired by SDL.
 */
class AllInputs {
    public:
        AllInputs() {};
        AllInputs(const AllInputs&);

        static const int MAXKEYS = 16;
        static const int MAXJOYS = 4;

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

        std::unique_ptr<MouseInputs> pointer;

        std::array<std::unique_ptr<ControllerInputs>,MAXJOYS> controllers;

        std::unique_ptr<MiscInputs> misc;

        /* Operator needed for comparing movies */
        bool operator==(const AllInputs& other) const;

        /* OR all elements of the struct, so that unique inputs can be queried */
        AllInputs& operator|=(const AllInputs& ai);

        AllInputs& operator=(const AllInputs& ai);
            
        /* Empty the state, set axes to neutral position. */
        void clear();

        /* Construct all input objects and clear the state */
        void buildAndClear();

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

        /* Send inputs through the socket */
        void send(bool preview);

        /* Receive inputs through the socket excluding the first message MSGN_ALL_INPUTS */
        void recv();

};

#endif
