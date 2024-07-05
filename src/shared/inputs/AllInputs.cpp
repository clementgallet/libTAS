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

#include "AllInputs.h"
#include "ControllerInputs.h"
#include "MouseInputs.h"
#include "MiscInputs.h"
#include "../shared/messages.h"
#include "../shared/sockethelpers.h"

#include <iostream>

AllInputs::AllInputs(const AllInputs& ai)
{
    *this = ai;
}

bool AllInputs::operator==(const AllInputs& other) const
{
    if (keyboard != other.keyboard)
        return false;
    
    if (pointer && other.pointer) {
        if (!(*pointer == *other.pointer))
            return false;
    }
        
    for (int j = 0; j < MAXJOYS; j++) {
        if (controllers[j] && other.controllers[j]) {
            if (!(*controllers[j] == *other.controllers[j]))
                return false;
        }
    }

    if (misc && other.misc) {
        return (*misc == *other.misc);
    }
    
    return true;
}

AllInputs& AllInputs::operator|=(const AllInputs& ai)
{
    /* keyboard is a bit complex */
    for (int k1 = 0; k1 < MAXKEYS; k1++) {
        if (ai.keyboard[k1] != 0) {
            bool toadd = true;
            int k2;
            for (k2 = 0; k2 < MAXKEYS; k2++) {
                if (keyboard[k2] == 0) {
                    toadd = true;
                    break;
                }
                if (ai.keyboard[k1] == keyboard[k2]) {
                    toadd = false;
                    break;
                }
            }
            if (toadd && (k2 < MAXKEYS))
                keyboard[k2] = ai.keyboard[k1];
        }
    }
    
    if (pointer && ai.pointer) {
        *pointer |= *ai.pointer;
    }
    
    for (int j=0; j<MAXJOYS; j++) {
        if (controllers[j] && ai.controllers[j]) {
            *controllers[j] |= *ai.controllers[j];
        }
    }

    if (misc && ai.misc) {
        *misc |= *ai.misc;
    }

    return *this;
}

AllInputs& AllInputs::operator=(const AllInputs& other)
{
    keyboard = other.keyboard;
    
    if (!other.pointer) {
        if (pointer)
            pointer->clear();
    }
    else {
        if (!pointer)
            pointer.reset(new MouseInputs{});
        
        *pointer = *other.pointer;
    }

    for (int j = 0; j < MAXJOYS; j++) {
        if (!other.controllers[j]) {
            if (controllers[j])
                controllers[j]->clear();
        }
        else {
            if (!controllers[j])
                controllers[j].reset(new ControllerInputs{});
            
            *controllers[j] = *other.controllers[j];
        }
    }

    if (!other.misc) {
        if (misc)
            misc->clear();
    }
    else {
        if (!misc)
            misc.reset(new MiscInputs{});
        
        *misc = *other.misc;
    }
    
    return *this;
}

void AllInputs::clear() {
    int i,j;
    for (i=0; i<MAXKEYS; i++)
        keyboard[i] = 0;

    if (pointer)
        pointer->clear();

    for (j=0; j<MAXJOYS; j++) {
        if (controllers[j]) {
            controllers[j]->clear();
        }
    }

    if (misc)
        misc->clear();
}

void AllInputs::buildAndClear()
{
    if (!pointer)
        pointer.reset(new MouseInputs{});

    for (int j=0; j<MAXJOYS; j++) {
        if (!controllers[j]) {
            controllers[j].reset(new ControllerInputs{});            
        }
    }

    if (!misc)
        misc.reset(new MiscInputs{});
    
    clear();
}

bool AllInputs::isDefaultController(int j) const
{
    if (controllers[j]) {
        return controllers[j]->isDefaultController();
    }
    return true;
}

int AllInputs::getInput(const SingleInput &si) const
{
    switch (si.type) {
        /* Keyboard inputs */
        case SingleInput::IT_KEYBOARD:
            for (const uint32_t& ks : keyboard) {
                if (si.value == ks) {
                    return 1;
                }
            }
            return 0;

        /* Mouse inputs */
        case SingleInput::IT_POINTER_X:
        case SingleInput::IT_POINTER_Y:
        case SingleInput::IT_POINTER_WHEEL:
        case SingleInput::IT_POINTER_MODE:
        case SingleInput::IT_POINTER_B1:
        case SingleInput::IT_POINTER_B2:
        case SingleInput::IT_POINTER_B3:
        case SingleInput::IT_POINTER_B4:
        case SingleInput::IT_POINTER_B5:
            if (pointer)
                return pointer->getInput(si);
            else
                break;

        case SingleInput::IT_FLAG:
        case SingleInput::IT_FRAMERATE_NUM:
        case SingleInput::IT_FRAMERATE_DEN:
        case SingleInput::IT_REALTIME_SEC:
        case SingleInput::IT_REALTIME_NSEC:
            if (misc)
                return misc->getInput(si);
            else
                break;

        default:
            /* Controller inputs */
            if (si.inputTypeIsController()) {
                int j = si.inputTypeToControllerNumber();
                if (controllers[j]) {
                    return controllers[j]->getInput(si);
                }
            }
    }
    return 0;
}

void AllInputs::setInput(const SingleInput &si, int value)
{
    switch (si.type) {
        case SingleInput::IT_KEYBOARD:
        {
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
        break;

        /* Mouse inputs */
        case SingleInput::IT_POINTER_X:
        case SingleInput::IT_POINTER_Y:
        case SingleInput::IT_POINTER_WHEEL:
        case SingleInput::IT_POINTER_MODE:
        case SingleInput::IT_POINTER_B1:
        case SingleInput::IT_POINTER_B2:
        case SingleInput::IT_POINTER_B3:
        case SingleInput::IT_POINTER_B4:
        case SingleInput::IT_POINTER_B5:
            if (!pointer)
                pointer.reset(new MouseInputs{});
            return pointer->setInput(si, value);
            
        case SingleInput::IT_FLAG:
        case SingleInput::IT_FRAMERATE_NUM:
        case SingleInput::IT_FRAMERATE_DEN:
        case SingleInput::IT_REALTIME_SEC:
        case SingleInput::IT_REALTIME_NSEC:
            if (!misc)
                misc.reset(new MiscInputs{});
            return misc->setInput(si, value);

        default:
            /* Controller inputs */
            if (si.inputTypeIsController()) {
                int j = si.inputTypeToControllerNumber();
                if (!controllers[j])
                    controllers[j].reset(new ControllerInputs());
                return controllers[j]->setInput(si, value);
            }
            break;
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

    if (pointer)
        pointer->extractInputs(input_set);

    if (misc)
        misc->extractInputs(input_set);

    for (int c = 0; c < AllInputs::MAXJOYS; c++) {
        if (controllers[c]) {
            controllers[c]->extractInputs(input_set, c);
        }
    }
}

void AllInputs::send(bool preview)
{
    if (preview)
        sendMessage(MSGN_PREVIEW_INPUTS);
    else
        sendMessage(MSGN_ALL_INPUTS);
    
    sendData(keyboard.data(), keyboard.size() * sizeof(uint32_t));

    if (pointer) {
        sendMessage(MSGN_POINTER_INPUTS);
        sendData(pointer.get(), sizeof(MouseInputs));        
    }
    
    for (int j = 0; j < MAXJOYS; j++) {
        if (controllers[j]) {
            sendMessage(MSGN_CONTROLLER_INPUTS);
            sendData(&j, sizeof(int));
            sendData(controllers[j].get(), sizeof(ControllerInputs));
        }
    }

    if (misc) {
        sendMessage(MSGN_MISC_INPUTS);
        sendData(misc.get(), sizeof(MiscInputs));        
    }
    
    sendMessage(MSGN_END_INPUTS);
}
