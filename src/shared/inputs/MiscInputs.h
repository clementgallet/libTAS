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

#ifndef LIBTAS_MISCINPUTS_H_INCLUDED
#define LIBTAS_MISCINPUTS_H_INCLUDED

#include "SingleInput.h"

#include <array>
#include <set>
#include <cstdint>

class MiscInputs {
    public:
        /* Flags */
        uint32_t flags;

        /* Framerate values */
        uint32_t framerate_den, framerate_num;

        /* Realtime values */
        uint32_t realtime_sec, realtime_nsec;

        /* Operator needed for comparing movies */
        inline bool operator==(const MiscInputs& other) const
        {
            return ((flags == other.flags) &&
                (framerate_den == other.framerate_den) &&
                (framerate_num == other.framerate_num) &&
                (realtime_sec == other.realtime_sec) &&
                (realtime_nsec == other.realtime_nsec));
        }

        /* OR all elements of the struct, so that unique inputs can be queried */
        MiscInputs& operator|=(const MiscInputs& mi);
        
        MiscInputs& operator=(const MiscInputs& mi);

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
