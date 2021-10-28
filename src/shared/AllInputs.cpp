/*
    Copyright 2015-2020 Clément Gallet <clement.gallet@ens-lyon.org>

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

#include "AllInputs.h"
// #include <X11/keysym.h>
#include <iostream>

void AllInputs::emptyInputs() {
    int i,j;
    for (i=0; i<MAXKEYS; i++)
        keyboard[i] = 0;

    pointer_x = 0;
    pointer_y = 0;
    pointer_mode = SingleInput::POINTER_MODE_ABSOLUTE;
    pointer_mask = 0;

    for (i=0; i<MAXJOYS; i++) {
        for (j=0; j<MAXAXES; j++)
            controller_axes[i][j] = 0;
        controller_buttons[i] = 0;
    }

    flags = 0;
    framerate_den = 0;
    framerate_num = 0;
}

bool AllInputs::isDefaultController(int j) const
{
    for (int a=0; a<MAXAXES; a++)
        if (controller_axes[j][a] != 0)
            return false;
    return (controller_buttons[j] == 0);
}

int AllInputs::getInput(const SingleInput &si) const
{
    /* Keyboard inputs */
    if (si.type == SingleInput::IT_KEYBOARD) {
        for (const uint32_t& ks : keyboard) {
            if (si.value == ks) {
                return 1;
            }
        }
        return 0;
    }

    /* Mouse inputs */
    if (si.type == SingleInput::IT_POINTER_X) {
        return pointer_x;
    }
    if (si.type == SingleInput::IT_POINTER_Y) {
        return pointer_y;
    }
    if (si.type == SingleInput::IT_POINTER_MODE) {
        return pointer_mode;
    }
    if (si.type >= SingleInput::IT_POINTER_B1 && si.type <= SingleInput::IT_POINTER_B5) {
        return (pointer_mask >> (si.type - SingleInput::IT_POINTER_B1)) & 0x1;
    }

    /* Flag inputs */
    if (si.type == SingleInput::IT_FLAG) {
        return (flags >> si.value) & 0x1;
    }

    /* Framerate inputs */
    if (si.type == SingleInput::IT_FRAMERATE_NUM) {
        return framerate_num;
    }
    if (si.type == SingleInput::IT_FRAMERATE_DEN) {
        return framerate_den;
    }

    /* Controller inputs */
    if (si.inputTypeIsController()) {
        int controller_i = si.inputTypeToControllerNumber();
        bool is_controller_axis = si.inputTypeToAxisFlag();
        int controller_type = si.inputTypeToInputNumber();

        /* We don't support analog inputs in input editor */
        if (is_controller_axis) {
            return controller_axes[controller_i][controller_type];
        }
        else {
            return (controller_buttons[controller_i] >> controller_type) & 0x1;
        }
    }

    return 0;
}

void AllInputs::setInput(const SingleInput &si, int value)
{
    if (si.type == SingleInput::IT_KEYBOARD) {
        bool is_set = false;
        int index_set = 0;
        int k;
        for (k=0; k < AllInputs::MAXKEYS; k++) {
            if (!keyboard[k]) {
                if (is_set && !value) {
                    /* Switch the last set key and the removed key */
                    keyboard[index_set] = keyboard[k-1];
                    keyboard[k-1] = 0;
                }
                break;
            }
            if (si.value == keyboard[k]) {
                is_set = true;
                if (!value) {
                    index_set = k;
                    keyboard[k] = 0;
                }
            }
        }

        /* If not set, add it */
        if (!is_set && value) {
            if (k < AllInputs::MAXKEYS) {
                keyboard[k] = si.value;
            }
        }
    }

    /* Mouse inputs */
    if (si.type == SingleInput::IT_POINTER_X) {
        pointer_x = value;
    }
    if (si.type == SingleInput::IT_POINTER_Y) {
        pointer_y = value;
    }
    if (si.type == SingleInput::IT_POINTER_MODE) {
        pointer_mode = value;
    }
    if (si.type >= SingleInput::IT_POINTER_B1 && si.type <= SingleInput::IT_POINTER_B5) {
        if (value)
            pointer_mask |= (0x1u << (si.type - SingleInput::IT_POINTER_B1));
        else
            pointer_mask &= ~(0x1u << (si.type - SingleInput::IT_POINTER_B1));
    }

    /* Flag input */
    if (si.type == SingleInput::IT_FLAG) {
        if (value)
            flags |= (0x1 << si.value);
        else
            flags &= ~(0x1 << si.value);
    }

    /* Framerate inputs */
    if (si.type == SingleInput::IT_FRAMERATE_NUM) {
        framerate_num = value;
    }
    if (si.type == SingleInput::IT_FRAMERATE_DEN) {
        framerate_den = value;
    }

    /* Controller inputs */
    if (si.inputTypeIsController()) {
        int controller_i = si.inputTypeToControllerNumber();
        bool is_controller_axis = si.inputTypeToAxisFlag();
        int controller_type = si.inputTypeToInputNumber();

        if (is_controller_axis) {
            if (value > INT16_MAX)
                controller_axes[controller_i][controller_type] = INT16_MAX;
            else if (value < INT16_MIN)
                controller_axes[controller_i][controller_type] = INT16_MIN;
            else
                controller_axes[controller_i][controller_type] = static_cast<short>(value);
        }
        else {
            if (value)
                controller_buttons[controller_i] |= (0x1 << controller_type);
            else
                controller_buttons[controller_i] &= ~(0x1 << controller_type);
        }
    }
}

int AllInputs::toggleInput(const SingleInput &si)
{
    int value = getInput(si);
    setInput(si, !value);
    return !value;
}

void AllInputs::extractInputs(std::set<SingleInput> &input_set) const
{
    SingleInput si;
    for (const uint32_t& ks : keyboard) {
        if (ks) {
            si = {SingleInput::IT_KEYBOARD, static_cast<unsigned int>(ks), std::to_string(ks)};
            input_set.insert(si);
        }
        else {
            break;
        }
    }

    if (pointer_x) {
        si = {SingleInput::IT_POINTER_X, 1, ""};
        input_set.insert(si);
    }
    if (pointer_y) {
        si = {SingleInput::IT_POINTER_Y, 1, ""};
        input_set.insert(si);
    }
    if (pointer_mode) {
        si = {SingleInput::IT_POINTER_MODE, 1, ""};
        input_set.insert(si);
    }
    for (int b=0; b<5; b++) {
        if (pointer_mask & (1 << b)) {
            si = {SingleInput::IT_POINTER_B1 + b, 1, ""};
            input_set.insert(si);
        }
    }

    if (flags) {
        uint32_t temp_flags = flags;
        for (unsigned int i=0; temp_flags!=0; i++, temp_flags >>= 1) {
            if (temp_flags & 0x1) {
                si = {SingleInput::IT_FLAG, i, ""};
                input_set.insert(si);
            }
        }
    }

    if (framerate_num) {
        si = {SingleInput::IT_FRAMERATE_NUM, 1, ""};
        input_set.insert(si);
    }
    if (framerate_den) {
        si = {SingleInput::IT_FRAMERATE_DEN, 1, ""};
        input_set.insert(si);
    }

    for (int c = 0; c < AllInputs::MAXJOYS; c++) {
        for (int a = 0; a < AllInputs::MAXAXES; a++) {
            if (controller_axes[c][a]) {
                si = {(((c+1) << SingleInput::IT_CONTROLLER_ID_SHIFT) | SingleInput::IT_CONTROLLER_AXIS_MASK) + a, 1, ""};
                input_set.insert(si);
            }
        }

        if (!controller_buttons[c]) {
            continue;
        }
        else {
            for (int b=0; b<16; b++) {
                if (controller_buttons[c] & (1 << b)) {
                    si = {((c+1) << SingleInput::IT_CONTROLLER_ID_SHIFT) + b, 1, ""};
                    input_set.insert(si);
                }
            }
        }
    }
}
