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

#include "AllInputsFlat.h"
#include "ControllerInputs.h"
#include "MouseInputs.h"
#include "MiscInputs.h"
#include "../shared/messages.h"
#include "../shared/sockethelpers.h"

#include <iostream>
#include <cstring>

void AllInputsFlat::clear() {
    memset(keyboard.data(), 0, sizeof(uint32_t)*keyboard.size());
    pointer.clear();
    for (int j=0; j<AllInputs::MAXJOYS; j++) {
        controllers[j].clear();
    }
    misc.clear();
}

void AllInputsFlat::recv()
{
    clear();
    
    receiveData(keyboard.data(), keyboard.size() * sizeof(uint32_t));
    
    int message = receiveMessage();
    while (message != MSGN_END_INPUTS) {
        switch (message) {
            case MSGN_POINTER_INPUTS:
                receiveData(&pointer, sizeof(MouseInputs));
                break;
            
            case MSGN_CONTROLLER_INPUTS: {
                int joy;
                receiveData(&joy, sizeof(int));
                receiveData(&controllers[joy], sizeof(ControllerInputs));
                break;
            }

            case MSGN_MISC_INPUTS:
                receiveData(&misc, sizeof(MiscInputs));
                break;
        }
        message = receiveMessage();
    }
}
