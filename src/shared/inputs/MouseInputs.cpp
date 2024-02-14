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

#include "MouseInputs.h"

#include <iostream>

MouseInputs& MouseInputs::operator|=(const MouseInputs& mi)
{
    x |= mi.x;
    y |= mi.y;
    mode |= mi.mode;
    mask |= mi.mask;
    return *this;
}

MouseInputs& MouseInputs::operator=(const MouseInputs& mi)
{
    x = mi.x;
    y = mi.y;
    mode = mi.mode;
    mask = mi.mask;
    return *this;
}

void MouseInputs::clear() {
    x = 0;
    y = 0;
    mode = SingleInput::POINTER_MODE_ABSOLUTE;
    mask = 0;
}

int MouseInputs::getInput(const SingleInput &si) const
{
    switch (si.type) {
        /* Mouse inputs */
        case SingleInput::IT_POINTER_X:
            return x;
        case SingleInput::IT_POINTER_Y:
            return y;
        case SingleInput::IT_POINTER_MODE:
            return mode;
        case SingleInput::IT_POINTER_B1:
        case SingleInput::IT_POINTER_B2:
        case SingleInput::IT_POINTER_B3:
        case SingleInput::IT_POINTER_B4:
        case SingleInput::IT_POINTER_B5:
            return (mask >> (si.type - SingleInput::IT_POINTER_B1)) & 0x1;
    }
    return 0;
}

void MouseInputs::setInput(const SingleInput &si, int value)
{
    switch (si.type) {
        case SingleInput::IT_POINTER_X:
            x = value;
            break;
        case SingleInput::IT_POINTER_Y:
            y = value;
            break;
        case SingleInput::IT_POINTER_MODE:
            mode = value;
            break;
        case SingleInput::IT_POINTER_B1:
        case SingleInput::IT_POINTER_B2:
        case SingleInput::IT_POINTER_B3:
        case SingleInput::IT_POINTER_B4:
        case SingleInput::IT_POINTER_B5:
            if (value)
                mask |= (0x1u << (si.type - SingleInput::IT_POINTER_B1));
            else
                mask &= ~(0x1u << (si.type - SingleInput::IT_POINTER_B1));
            break;
    }
}

void MouseInputs::extractInputs(std::set<SingleInput> &input_set) const
{
    SingleInput si;

    if (x) {
        si = {SingleInput::IT_POINTER_X, 1, ""};
        input_set.insert(si);
    }
    if (y) {
        si = {SingleInput::IT_POINTER_Y, 1, ""};
        input_set.insert(si);
    }
    if (mode) {
        si = {SingleInput::IT_POINTER_MODE, 1, ""};
        input_set.insert(si);
    }
    for (int b=0; b<5; b++) {
        if (mask & (1 << b)) {
            si = {SingleInput::IT_POINTER_B1 + b, 1, ""};
            input_set.insert(si);
        }
    }
}
