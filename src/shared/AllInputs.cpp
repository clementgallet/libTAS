/*
    Copyright 2015-2018 Clément Gallet <clement.gallet@ens-lyon.org>

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
#include <X11/keysym.h>

void AllInputs::emptyInputs() {
    int i,j;
    for (i=0; i<MAXKEYS; i++)
        keyboard[i] = 0;

    pointer_x = 0;
    pointer_y = 0;
    pointer_mask = 0;

    for (i=0; i<MAXJOYS; i++) {
        for (j=0; j<MAXAXES; j++)
            controller_axes[i][j] = 0;
        controller_buttons[i] = 0;
    }
}

bool AllInputs::checkInput(const SingleInput &si) const
{
    if (si.type == SingleInput::IT_KEYBOARD) {
        for (const KeySym& ks : keyboard) {
            if (si.value == ks) {
                return true;
                break;
            }
        }
    }

    if (si.type & SingleInput::IT_CONTROLLER_ID_MASK) {
        int controller_i = SingleInput::inputTypeToControllerNumber(si.type);
        bool controller_axis = SingleInput::inputTypeToAxisFlag(si.type);
        int controller_type = SingleInput::inputTypeToInputNumber(si.type);

        /* We don't support analog inputs in input editor */
        if (!controller_axis) {
            return controller_buttons[controller_i] & ((si.value & 0x1) << controller_type);
        }
    }

    return false;
}

void AllInputs::setInput(const SingleInput &si)
{
    if (si.type == SingleInput::IT_KEYBOARD) {
        bool is_set = false;
        int k;
        for (k=0; k < AllInputs::MAXKEYS; k++) {
            if (!keyboard[k]) {
                break;
            }
            if (si.value == keyboard[k]) {
                is_set = true;
            }
        }

        /* If not set, add it */
        if (!is_set) {
            if (k < AllInputs::MAXKEYS) {
                keyboard[k] = si.value;
            }
        }
    }

    if (si.type & SingleInput::IT_CONTROLLER_ID_MASK) {
        int controller_i = SingleInput::inputTypeToControllerNumber(si.type);
        bool controller_axis = SingleInput::inputTypeToAxisFlag(si.type);
        int controller_type = SingleInput::inputTypeToInputNumber(si.type);

        /* We don't support analog inputs in input editor */
        if (!controller_axis) {
            controller_buttons[controller_i] |= ((si.value & 0x1) << controller_type);
        }
    }
}

void AllInputs::clearInput(const SingleInput &si)
{
    if (si.type == SingleInput::IT_KEYBOARD) {
        /* Check if key is set and remove it */
        bool is_set = false;
        int index_set = 0;
        int k;
        for (k=0; k < AllInputs::MAXKEYS; k++) {
            if (!keyboard[k]) {
                if (is_set) {
                    /* Switch the last set key and the removed key */
                    keyboard[index_set] = keyboard[k-1];
                    keyboard[k-1] = 0;
                }
                break;
            }
            if (si.value == keyboard[k]) {
                is_set = true;
                index_set = k;
                keyboard[k] = 0;
            }
        }
    }

    if (si.type & SingleInput::IT_CONTROLLER_ID_MASK) {
        int controller_i = SingleInput::inputTypeToControllerNumber(si.type);
        bool controller_axis = SingleInput::inputTypeToAxisFlag(si.type);
        int controller_type = SingleInput::inputTypeToInputNumber(si.type);

        /* We don't support analog inputs in input editor */
        if (!controller_axis) {
            controller_buttons[controller_i] &= ~((si.value & 0x1) << controller_type);
        }
    }
}

bool AllInputs::toggleInput(const SingleInput &si)
{
    if (si.type == SingleInput::IT_KEYBOARD) {
        /* Check if key is set and remove it */
        bool is_set = false;
        int index_set = 0;
        int k;
        for (k=0; k < AllInputs::MAXKEYS; k++) {
            if (!keyboard[k]) {
                if (is_set) {
                    /* Switch the last set key and the removed key */
                    keyboard[index_set] = keyboard[k-1];
                    keyboard[k-1] = 0;
                    return false;
                }
                break;
            }
            if (si.value == keyboard[k]) {
                is_set = true;
                index_set = k;
                keyboard[k] = 0;
            }
        }

        /* If not set, add it */
        if (!is_set) {
            if (k < AllInputs::MAXKEYS) {
                keyboard[k] = si.value;
                return true;
            }
        }
    }

    if (si.type & SingleInput::IT_CONTROLLER_ID_MASK) {
        int controller_i = SingleInput::inputTypeToControllerNumber(si.type);
        bool controller_axis = SingleInput::inputTypeToAxisFlag(si.type);
        int controller_type = SingleInput::inputTypeToInputNumber(si.type);

        /* We don't support analog inputs in input editor */
        if (!controller_axis) {
            bool value = controller_buttons[controller_i] & ((si.value & 0x1) << controller_type);
            controller_buttons[controller_i] ^= ((si.value & 0x1) << controller_type);
            return !value;
        }
    }

    return false;
}
