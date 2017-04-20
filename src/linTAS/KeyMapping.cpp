/*
    Copyright 2015-2016 Clément Gallet <clement.gallet@ens-lyon.org>

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

#include "KeyMapping.h"
#include <X11/XKBlib.h>

KeyMapping::KeyMapping()
{
    /* Fill hotkey list */
    hotkey_list.push_back({{IT_KEYBOARD, XK_Pause}, HOTKEY_PLAYPAUSE, "Play/Pause"});
    hotkey_list.push_back({{IT_KEYBOARD, XK_v}, HOTKEY_FRAMEADVANCE, "Frame Advance"});
    hotkey_list.push_back({{IT_KEYBOARD, XK_Tab}, HOTKEY_FASTFORWARD, "Fast-forward"});
    hotkey_list.push_back({{IT_NONE, 0}, HOTKEY_READWRITE, "Toggle ReadWrite/ReadOnly"});
    hotkey_list.push_back({{IT_NONE, 0}, HOTKEY_SAVESTATE, "Save State"});
    hotkey_list.push_back({{IT_NONE, 0}, HOTKEY_LOADSTATE, "Load State"});

    /* Set default hotkeys */
    default_hotkeys();

    /* Gather the list of valid X11 KeyCode values */
    int min_keycodes_return, max_keycodes_return;
    Display *display = XOpenDisplay(NULL);
    if (display == NULL)
        return;
    XDisplayKeycodes(display, &min_keycodes_return, &max_keycodes_return);

    /* Build the list of KeySym values to be mapped based on valid KeyCodes.
     * This list is dependent on the keyboard layout.
     */
    for (int k=min_keycodes_return; k<=max_keycodes_return; k++) {
        KeySym ks = XkbKeycodeToKeysym(display, k, 0, 0);
        if (ks == NoSymbol) continue;

        SingleInput si;
        si.type = IT_KEYBOARD;
        si.value = static_cast<int>(ks);
        si.description = XKeysymToString(ks);
        input_list.push_back(si);
    }

    /* Add controller mapping */
    input_list.push_back({IT_CONTROLLER1_BUTTON_A, 1, "Controller 1 - A"});
    input_list.push_back({IT_CONTROLLER1_BUTTON_B, 1, "Controller 1 - B"});
    input_list.push_back({IT_CONTROLLER1_BUTTON_X, 1, "Controller 1 - X"});
    input_list.push_back({IT_CONTROLLER1_BUTTON_Y, 1, "Controller 1 - Y"});
    input_list.push_back({IT_CONTROLLER1_BUTTON_BACK, 1, "Controller 1 - Back"});
    input_list.push_back({IT_CONTROLLER1_BUTTON_GUIDE, 1, "Controller 1 - Guide"});
    input_list.push_back({IT_CONTROLLER1_BUTTON_START, 1, "Controller 1 - Start"});
    input_list.push_back({IT_CONTROLLER1_BUTTON_LEFTSTICK, 1, "Controller 1 - Left Stick"});
    input_list.push_back({IT_CONTROLLER1_BUTTON_RIGHTSTICK, 1, "Controller 1 - Right Stick"});
    input_list.push_back({IT_CONTROLLER1_BUTTON_LEFTSHOULDER, 1, "Controller 1 - Left Shoulder"});
    input_list.push_back({IT_CONTROLLER1_BUTTON_RIGHTSHOULDER, 1, "Controller 1 - Right Shoulder"});
    input_list.push_back({IT_CONTROLLER1_BUTTON_DPAD_UP, 1, "Controller 1 - Dpad Up"});
    input_list.push_back({IT_CONTROLLER1_BUTTON_DPAD_DOWN, 1, "Controller 1 - Dpad Down"});
    input_list.push_back({IT_CONTROLLER1_BUTTON_DPAD_LEFT, 1, "Controller 1 - Dpad Left"});
    input_list.push_back({IT_CONTROLLER1_BUTTON_DPAD_RIGHT, 1, "Controller 1 - Dpad Right"});

    input_list.push_back({IT_CONTROLLER2_BUTTON_A, 1, "Controller 2 - A"});
    input_list.push_back({IT_CONTROLLER2_BUTTON_B, 1, "Controller 2 - B"});
    input_list.push_back({IT_CONTROLLER2_BUTTON_X, 1, "Controller 2 - X"});
    input_list.push_back({IT_CONTROLLER2_BUTTON_Y, 1, "Controller 2 - Y"});
    input_list.push_back({IT_CONTROLLER2_BUTTON_BACK, 1, "Controller 2 - Back"});
    input_list.push_back({IT_CONTROLLER2_BUTTON_GUIDE, 1, "Controller 2 - Guide"});
    input_list.push_back({IT_CONTROLLER2_BUTTON_START, 1, "Controller 2 - Start"});
    input_list.push_back({IT_CONTROLLER2_BUTTON_LEFTSTICK, 1, "Controller 2 - Left Stick"});
    input_list.push_back({IT_CONTROLLER2_BUTTON_RIGHTSTICK, 1, "Controller 2 - Right Stick"});
    input_list.push_back({IT_CONTROLLER2_BUTTON_LEFTSHOULDER, 1, "Controller 2 - Left Shoulder"});
    input_list.push_back({IT_CONTROLLER2_BUTTON_RIGHTSHOULDER, 1, "Controller 2 - Right Shoulder"});
    input_list.push_back({IT_CONTROLLER2_BUTTON_DPAD_UP, 1, "Controller 2 - Dpad Up"});
    input_list.push_back({IT_CONTROLLER2_BUTTON_DPAD_DOWN, 1, "Controller 2 - Dpad Down"});
    input_list.push_back({IT_CONTROLLER2_BUTTON_DPAD_LEFT, 1, "Controller 2 - Dpad Left"});
    input_list.push_back({IT_CONTROLLER2_BUTTON_DPAD_RIGHT, 1, "Controller 2 - Dpad Right"});

    input_list.push_back({IT_CONTROLLER3_BUTTON_A, 1, "Controller 3 - A"});
    input_list.push_back({IT_CONTROLLER3_BUTTON_B, 1, "Controller 3 - B"});
    input_list.push_back({IT_CONTROLLER3_BUTTON_X, 1, "Controller 3 - X"});
    input_list.push_back({IT_CONTROLLER3_BUTTON_Y, 1, "Controller 3 - Y"});
    input_list.push_back({IT_CONTROLLER3_BUTTON_BACK, 1, "Controller 3 - Back"});
    input_list.push_back({IT_CONTROLLER3_BUTTON_GUIDE, 1, "Controller 3 - Guide"});
    input_list.push_back({IT_CONTROLLER3_BUTTON_START, 1, "Controller 3 - Start"});
    input_list.push_back({IT_CONTROLLER3_BUTTON_LEFTSTICK, 1, "Controller 3 - Left Stick"});
    input_list.push_back({IT_CONTROLLER3_BUTTON_RIGHTSTICK, 1, "Controller 3 - Right Stick"});
    input_list.push_back({IT_CONTROLLER3_BUTTON_LEFTSHOULDER, 1, "Controller 3 - Left Shoulder"});
    input_list.push_back({IT_CONTROLLER3_BUTTON_RIGHTSHOULDER, 1, "Controller 3 - Right Shoulder"});
    input_list.push_back({IT_CONTROLLER3_BUTTON_DPAD_UP, 1, "Controller 3 - Dpad Up"});
    input_list.push_back({IT_CONTROLLER3_BUTTON_DPAD_DOWN, 1, "Controller 3 - Dpad Down"});
    input_list.push_back({IT_CONTROLLER3_BUTTON_DPAD_LEFT, 1, "Controller 3 - Dpad Left"});
    input_list.push_back({IT_CONTROLLER3_BUTTON_DPAD_RIGHT, 1, "Controller 3 - Dpad Right"});

    input_list.push_back({IT_CONTROLLER4_BUTTON_A, 1, "Controller 4 - A"});
    input_list.push_back({IT_CONTROLLER4_BUTTON_B, 1, "Controller 4 - B"});
    input_list.push_back({IT_CONTROLLER4_BUTTON_X, 1, "Controller 4 - X"});
    input_list.push_back({IT_CONTROLLER4_BUTTON_Y, 1, "Controller 4 - Y"});
    input_list.push_back({IT_CONTROLLER4_BUTTON_BACK, 1, "Controller 4 - Back"});
    input_list.push_back({IT_CONTROLLER4_BUTTON_GUIDE, 1, "Controller 4 - Guide"});
    input_list.push_back({IT_CONTROLLER4_BUTTON_START, 1, "Controller 4 - Start"});
    input_list.push_back({IT_CONTROLLER4_BUTTON_LEFTSTICK, 1, "Controller 4 - Left Stick"});
    input_list.push_back({IT_CONTROLLER4_BUTTON_RIGHTSTICK, 1, "Controller 4 - Right Stick"});
    input_list.push_back({IT_CONTROLLER4_BUTTON_LEFTSHOULDER, 1, "Controller 4 - Left Shoulder"});
    input_list.push_back({IT_CONTROLLER4_BUTTON_RIGHTSHOULDER, 1, "Controller 4 - Right Shoulder"});
    input_list.push_back({IT_CONTROLLER4_BUTTON_DPAD_UP, 1, "Controller 4 - Dpad Up"});
    input_list.push_back({IT_CONTROLLER4_BUTTON_DPAD_DOWN, 1, "Controller 4 - Dpad Down"});
    input_list.push_back({IT_CONTROLLER4_BUTTON_DPAD_LEFT, 1, "Controller 4 - Dpad Left"});
    input_list.push_back({IT_CONTROLLER4_BUTTON_DPAD_RIGHT, 1, "Controller 4 - Dpad Right"});

    /* Set default inputs */
    default_inputs();

    /* Filling some custom mapping to controller buttons.
     * This is for testing, it will be removed when the controller mapping
     * interface will be implemented.
     */
    input_mapping[XK_w].type = IT_CONTROLLER1_BUTTON_A;
    input_mapping[XK_w].value = 1;
    input_mapping[XK_x].type = IT_CONTROLLER1_BUTTON_B;
    input_mapping[XK_x].value = 1;
    input_mapping[XK_c].type = IT_CONTROLLER1_BUTTON_X;
    input_mapping[XK_c].value = 1;
    input_mapping[XK_d].type = IT_CONTROLLER1_BUTTON_LEFTSHOULDER;
    input_mapping[XK_d].value = 1;
    input_mapping[XK_f].type = IT_CONTROLLER1_BUTTON_RIGHTSHOULDER;
    input_mapping[XK_f].value = 1;
    input_mapping[XK_i].type = IT_CONTROLLER1_BUTTON_DPAD_UP;
    input_mapping[XK_i].value = 1;
    input_mapping[XK_k].type = IT_CONTROLLER1_BUTTON_DPAD_DOWN;
    input_mapping[XK_k].value = 1;
    input_mapping[XK_j].type = IT_CONTROLLER1_BUTTON_DPAD_LEFT;
    input_mapping[XK_j].value = 1;
    input_mapping[XK_l].type = IT_CONTROLLER1_BUTTON_DPAD_RIGHT;
    input_mapping[XK_l].value = 1;
}

void KeyMapping::default_hotkeys()
{
    hotkey_mapping.clear();
    for (auto iter : hotkey_list) {
        if (iter.default_input.type == IT_KEYBOARD) {
            hotkey_mapping[iter.default_input.value] = iter;
        }
    }
}

void KeyMapping::default_inputs()
{
    input_mapping.clear();
    for (auto iter : input_list) {
        if (iter.type == IT_KEYBOARD) {
            input_mapping[iter.value] = iter;
        }
    }
}

void KeyMapping::default_hotkey(int hotkey_index)
{
    /* Hotkey selected */
    HotKey hk = hotkey_list[hotkey_index];

    /* Remove previous mapping from this key */
    for (auto iter : hotkey_mapping) {
        if (iter.second == hk) {
            hotkey_mapping.erase(iter.first);
            break;
        }
    }

    if (hk.default_input.type == IT_KEYBOARD)
        hotkey_mapping[hk.default_input.value] = hk;

}

void KeyMapping::default_input(int input_index)
{
    /* Input selected */
    SingleInput si = input_list[input_index];

    /* Remove previous mapping from this key */
    for (auto iter : input_mapping) {
        if (iter.second == si) {
            input_mapping.erase(iter.first);
            break;
        }
    }

    if (si.type == IT_KEYBOARD)
        input_mapping[si.value] = si;
}

void KeyMapping::reassign_hotkey(int hotkey_index, KeySym ks)
{
    /* Hotkey selected */
    HotKey hk = hotkey_list[hotkey_index];

    /* Remove previous mapping from this key */
    for (auto iter : hotkey_mapping) {
        if (iter.second == hk) {
            hotkey_mapping.erase(iter.first);
            break;
        }
    }

    if (ks)
        hotkey_mapping[ks] = hk;
}

void KeyMapping::reassign_input(int input_index, KeySym ks)
{
    /* Input selected */
    SingleInput si = input_list[input_index];

    /* Remove previous mapping from this key */
    for (auto iter : input_mapping) {
        if (iter.second == si) {
            input_mapping.erase(iter.first);
            break;
        }
    }

    if (ks)
        input_mapping[ks] = si;
}


void KeyMapping::buildAllInputs(struct AllInputs& ai, Display *display, char keyboard_state[], SharedConfig& sc){
    int i,j,k;
    int keysym_i = 0;

    ai.emptyInputs();

    for (i=0; i<32; i++) {
        if (keyboard_state[i] == 0)
            continue;
        for (j=0; j<8; j++) {
            if ((keyboard_state[i] >> j) & 0x1) {

                /* We got a pressed keycode */
                KeyCode kc = (i << 3) | j;
                /* Translating to keysym */
                KeySym ks = XkbKeycodeToKeysym(display, kc, 0, 0);

                if (hotkey_mapping.find(ks) != hotkey_mapping.end()) {
                    /* Dealing with a hotkey, skipping */
                    continue;
                }

                /* Checking the mapped input for that key */
                struct SingleInput si = {IT_NONE,0};
                if (input_mapping.find(ks) != input_mapping.end())
                    si = input_mapping[ks];

                if (si.type == IT_NONE) {
                    /* Key is mapped to nothing */
                    continue;
                }

                if (si.type == IT_KEYBOARD) {
                    /* Check if we support keyboard */
                    if (!sc.keyboard_support)
                        continue;

                    /* Checking the current number of keys */
                    if (keysym_i >= AllInputs::MAXKEYS) {
                        fprintf(stderr, "Reached maximum number of inputs (%d).", AllInputs::MAXKEYS);
                        return;
                    }

                    /* Saving the key */
                    ai.keyboard[keysym_i++] = si.value;
                }

                if (si.type & IT_CONTROLLER_ID_MASK) {
                    /* Key is mapped to a game controller */

                    /* Getting Controller id
                     * Arithmetic on enums is bad, no?
                     */
                    int controller_i = ((si.type & IT_CONTROLLER_ID_MASK) >> IT_CONTROLLER_ID_SHIFT) - 1;

                    /* Check if we support this joystick */
                    if (controller_i >= sc.numControllers)
                        continue;

                    int controller_axis = si.type & IT_CONTROLLER_AXIS_MASK;
                    int controller_type = si.type & IT_CONTROLLER_TYPE_MASK;
                    if (controller_axis) {
                        ai.controller_axes[controller_i][controller_type] = static_cast<short>(si.value);
                    }
                    else {
                        ai.controller_buttons[controller_i] |= (si.value & 0x1) << controller_type;
                    }
                }
            }
        }
    }
}
