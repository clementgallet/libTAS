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

#include "ControllerInputs.h"

#include <iostream>

ControllerInputs& ControllerInputs::operator|=(const ControllerInputs& ci)
{
    for (int i=0; i<MAXAXES; i++)
        axes[i] |= ci.axes[i];
    buttons |= ci.buttons;
    return *this;
}

ControllerInputs& ControllerInputs::operator=(const ControllerInputs& ci)
{
    for (int axis=0; axis<MAXAXES; axis++) {
        axes[axis] = ci.axes[axis];
    }
    buttons = ci.buttons;
    return *this;
}

void ControllerInputs::emptyInputs() {
    for (int i=0; i<MAXAXES; i++)
        axes[i] = 0;
    buttons = 0;
}

bool ControllerInputs::isDefaultController() const
{
    for (int a=0; a<MAXAXES; a++)
        if (axes[a] != 0)
            return false;
    return (buttons == 0);
}

int ControllerInputs::getInput(const SingleInput &si) const
{
    bool is_axis = si.inputTypeToAxisFlag();
    int type = si.inputTypeToInputNumber();

    if (is_axis)
        return axes[type];
    return (buttons >> type) & 0x1;
}

void ControllerInputs::setInput(const SingleInput &si, int value)
{
    bool is_axis = si.inputTypeToAxisFlag();
    int type = si.inputTypeToInputNumber();

    if (is_axis) {
        if (value > INT16_MAX)
            axes[type] = INT16_MAX;
        else if (value < INT16_MIN)
            axes[type] = INT16_MIN;
        else
            axes[type] = static_cast<short>(value);
    }
    else {
        if (value)
            buttons |= (0x1 << type);
        else
            buttons &= ~(0x1 << type);
    }
}

void ControllerInputs::extractInputs(std::set<SingleInput> &input_set, int controller_id) const
{
    SingleInput si;

    for (int a = 0; a < MAXAXES; a++) {
        if (axes[a]) {
            si = {(((controller_id+1) << SingleInput::IT_CONTROLLER_ID_SHIFT) | SingleInput::IT_CONTROLLER_AXIS_MASK) + a, 1, ""};
            input_set.insert(si);
        }
    }

    if (buttons) {
        for (int b=0; b<16; b++) {
            if (buttons & (1 << b)) {
                si = {((controller_id+1) << SingleInput::IT_CONTROLLER_ID_SHIFT) + b, 1, ""};
                input_set.insert(si);
            }
        }
    }
}
