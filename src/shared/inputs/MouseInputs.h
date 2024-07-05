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

#ifndef LIBTAS_MOUSEINPUTS_H_INCLUDED
#define LIBTAS_MOUSEINPUTS_H_INCLUDED

#include "SingleInput.h"

#include <array>
#include <set>
#include <cstdint>

class MouseInputs {
    public:
        /* Pointer coordinates relative to the origin of the game window */
        int x;
        int y;

        /* Pointer vertical wheel */
        int wheel;

        /* Absolute or relative mode */
        unsigned int mode;

        /* Pointer buttons */
        unsigned int mask;

        /* Operator needed for comparing movies */
        inline bool operator==(const MouseInputs& other) const
        {
            return ((x == other.x) && (y == other.y) &&
             (mode == other.mode) && (mask == other.mask) && (wheel == other.wheel));
        }

        /* OR all elements of the struct, so that unique inputs can be queried */
        MouseInputs& operator|=(const MouseInputs& mi);
        
        MouseInputs& operator=(const MouseInputs& mi);

        /* Empty the state, set axes to neutral position. */
        void clear();

        /* Get the value of a single input, or 0/1 if the input is set/not set */
        int getInput(const SingleInput &si) const;

        /* Set a single input in the inputs to a certain value */
        void setInput(const SingleInput &si, int value);

        /* Extract all single inputs and insert them in the set */
        void extractInputs(std::set<SingleInput> &set) const;

};

#endif
