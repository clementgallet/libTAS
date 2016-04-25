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

#include "keymapping.h"
#include <map>

/* Map keyboard Keycodes to a single input of a keyboard or controller */
std::map<KeySym,struct SingleInput> input_mapping;

void default_hotkeys(KeySym *hotkeys){

    hotkeys[HOTKEY_PLAYPAUSE] = XK_Pause;
    hotkeys[HOTKEY_FRAMEADVANCE] = XK_v;
    hotkeys[HOTKEY_FASTFORWARD] = XK_Tab;
    hotkeys[HOTKEY_READWRITE] = XK_p;
    hotkeys[HOTKEY_SAVESTATE] = XK_s;
    hotkeys[HOTKEY_LOADSTATE] = XK_m;

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

/* 
 * We are building the whole AllInputs structure,
 * that will be passed to the game and saved.
 * We will be doing the following steps:
 * - Convert keyboard keycodes (physical keys) to keysyms (key meaning)
 * - Check if the keysym is mapped to a hotkey. If so, we skip it
 * - Check if the key is mapped to another input and fill the AllInputs struct accordingly
 */

void buildAllInputs(struct AllInputs* ai, Display *display, char keyboard_state[], KeySym hotkeys[]){
    int i,j,k;
    int keysym_i = 0;

    ai->emptyInputs();

    for (i=0; i<32; i++) {
        if (keyboard_state[i] == 0)
            continue;
        for (j=0; j<8; j++) {
            if ((keyboard_state[i] >> j) & 0x1) {

                /* We got a pressed keycode */
                KeyCode kc = (i << 3) | j;
                /* Translating to keysym */
                KeySym ks = XkbKeycodeToKeysym(display, kc, 0, 0);

                for (k=0; k<HOTKEY_LEN; k++) {
                    if (hotkeys[k] == ks) {
                        /* The pressed key was in fact a hotkey. */
                        break;
                    }
                }

                if (k < HOTKEY_LEN) {
                    /* Dealing with a hotkey, skipping */
                    continue;
                }

                /* Checking the mapped input for that key */
                struct SingleInput si = {IT_ID,0};
                if (input_mapping.find(ks) != input_mapping.end()) 
                    si = input_mapping[ks];

                if (si.type == IT_NONE) {
                    /* Key is mapped to nothing */
                    continue;
                }

                if (si.type == IT_ID) {
                    /* Key is mapped to itself */
                    si.type = IT_KEYBOARD;
                    si.value = ks;
                }

                if (si.type == IT_KEYBOARD) {

                    /* Checking the current number of keys */
                    if (keysym_i >= AllInputs::MAXKEYS) {
                        fprintf(stderr, "Reached maximum number of inputs (%d).", AllInputs::MAXKEYS);
                        return;
                    }

                    /* Saving the key */
                    ai->keyboard[keysym_i++] = si.value;
                }

                if (si.type >= IT_CONTROLLER1_AXIS_LEFTX) {
                    /* Key is mapped to a game controller */

                    /* Getting Controller id 
                     * Arithmetic on enums is bad, no?
                     */
                    int controller_i = si.type / IT_CONTROLLER1_AXIS_LEFTX - 1;
                    int controller_type = si.type % IT_CONTROLLER1_AXIS_LEFTX;
                    int button_start = IT_CONTROLLER1_BUTTON_A % IT_CONTROLLER1_AXIS_LEFTX;
                    if (controller_type < button_start) {
                        ai->controller_axes[controller_i][controller_type] = static_cast<short>(si.value);
                    }
                    else {
                        int button_type = controller_type - button_start;
                        ai->controller_buttons[controller_i] |= (si.value & 0x1) << button_type;
                    }
                }
            }
        }
    }
}


