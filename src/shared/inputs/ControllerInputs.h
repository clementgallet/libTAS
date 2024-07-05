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

#ifndef LIBTAS_CONTROLLERINPUTS_H_INCLUDED
#define LIBTAS_CONTROLLERINPUTS_H_INCLUDED

#include "SingleInput.h"

#include <array>
#include <set>
#include <cstdint>

class ControllerInputs {
    public:
        static const int MAXAXES = 6;

        /* axes[j] stores the state of axis j of controller */
        std::array<short, MAXAXES> axes;

        /* buttons stores the bitmap state of buttons of
         * controller. Bit j set means that button j is pressed.
         * We use a 2-byte integer here, meaning we can have at most
         * 16 buttons in a controller.
         */
        unsigned short buttons;

        /* Operator needed for comparing movies */
        inline bool operator==(const ControllerInputs& other) const
        {
            return ((axes == other.axes) &&
                (buttons == other.buttons));
        }

        /* OR all elements of the struct, so that unique inputs can be queried */
        ControllerInputs& operator|=(const ControllerInputs& ci);
        
        ControllerInputs& operator=(const ControllerInputs& ci);

        /* Empty the state, set axes to neutral position. */
        void clear();

        /* Check if a controller has default values */
        bool isDefaultController() const;

        /* Get the value of a single input, or 0/1 if the input is set/not set */
        int getInput(const SingleInput &si) const;

        /* Set a single input in the inputs to a certain value */
        void setInput(const SingleInput &si, int value);

        /* Extract all single inputs and insert them in the set */
        void extractInputs(std::set<SingleInput> &set, int controller_id) const;

};

#endif
