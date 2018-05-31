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

#include "KeyMapping.h"
#include <X11/Xlib.h>
#include <cstring>
#include <iostream>

QDataStream &operator<<(QDataStream &out, const SingleInput &obj) {
    out << obj.type << obj.value;
    return out;
}

QDataStream &operator>>(QDataStream &in, SingleInput &obj) {
    in >> obj.type >> obj.value;
    return in;
}

QDataStream &operator<<(QDataStream &out, const HotKey &obj) {
    out << obj.type;
    return out;
}

QDataStream &operator>>(QDataStream &in, HotKey &obj) {
    in >> obj.type;
    return in;
}

bool is_modifier(xcb_keysym_t ks)
{
    for (ModifierKey modifier : modifier_list) {
        if (modifier.ks == ks)
            return true;
    }
    return false;
}

xcb_keysym_t build_modifiers(unsigned char keyboard_state[], xcb_key_symbols_t *keysyms)
{
    xcb_keysym_t modifiers = 0;

    for (int i=0; i<256; i++) {
        if (keyboard_state[i/8] & (1 << (i % 8))) {
            xcb_keysym_t ks = xcb_key_symbols_get_keysym(keysyms, i, 0);
            for (ModifierKey modifier : modifier_list) {
                if (modifier.ks == ks) {
                    modifiers |= modifier.flag;
                    break;
                }
            }
        }
    }
    return modifiers;
}

void KeyMapping::init(xcb_connection_t* conn)
{
    /* Fill hotkey list */
    hotkey_list.push_back({{SingleInput::IT_KEYBOARD, XK_Pause}, HOTKEY_PLAYPAUSE, "Play/Pause"});
    hotkey_list.push_back({{SingleInput::IT_KEYBOARD, XK_v}, HOTKEY_FRAMEADVANCE, "Frame Advance"});
    hotkey_list.push_back({{SingleInput::IT_KEYBOARD, XK_Tab}, HOTKEY_FASTFORWARD, "Fast-forward"});
    hotkey_list.push_back({{SingleInput::IT_NONE, 0}, HOTKEY_READWRITE, "Toggle Read/Write"});
    hotkey_list.push_back({{SingleInput::IT_KEYBOARD, XK_F1 | XK_Shift_L_Flag}, HOTKEY_SAVESTATE1, "Save State 1"});
    hotkey_list.push_back({{SingleInput::IT_KEYBOARD, XK_F2 | XK_Shift_L_Flag}, HOTKEY_SAVESTATE2, "Save State 2"});
    hotkey_list.push_back({{SingleInput::IT_KEYBOARD, XK_F3 | XK_Shift_L_Flag}, HOTKEY_SAVESTATE3, "Save State 3"});
    hotkey_list.push_back({{SingleInput::IT_KEYBOARD, XK_F4 | XK_Shift_L_Flag}, HOTKEY_SAVESTATE4, "Save State 4"});
    hotkey_list.push_back({{SingleInput::IT_KEYBOARD, XK_F5 | XK_Shift_L_Flag}, HOTKEY_SAVESTATE5, "Save State 5"});
    hotkey_list.push_back({{SingleInput::IT_KEYBOARD, XK_F6 | XK_Shift_L_Flag}, HOTKEY_SAVESTATE6, "Save State 6"});
    hotkey_list.push_back({{SingleInput::IT_KEYBOARD, XK_F7 | XK_Shift_L_Flag}, HOTKEY_SAVESTATE7, "Save State 7"});
    hotkey_list.push_back({{SingleInput::IT_KEYBOARD, XK_F8 | XK_Shift_L_Flag}, HOTKEY_SAVESTATE8, "Save State 8"});
    hotkey_list.push_back({{SingleInput::IT_KEYBOARD, XK_F9 | XK_Shift_L_Flag}, HOTKEY_SAVESTATE9, "Save State 9"});
    hotkey_list.push_back({{SingleInput::IT_KEYBOARD, XK_F1}, HOTKEY_LOADSTATE1, "Load State 1"});
    hotkey_list.push_back({{SingleInput::IT_KEYBOARD, XK_F2}, HOTKEY_LOADSTATE2, "Load State 2"});
    hotkey_list.push_back({{SingleInput::IT_KEYBOARD, XK_F3}, HOTKEY_LOADSTATE3, "Load State 3"});
    hotkey_list.push_back({{SingleInput::IT_KEYBOARD, XK_F4}, HOTKEY_LOADSTATE4, "Load State 4"});
    hotkey_list.push_back({{SingleInput::IT_KEYBOARD, XK_F5}, HOTKEY_LOADSTATE5, "Load State 5"});
    hotkey_list.push_back({{SingleInput::IT_KEYBOARD, XK_F6}, HOTKEY_LOADSTATE6, "Load State 6"});
    hotkey_list.push_back({{SingleInput::IT_KEYBOARD, XK_F7}, HOTKEY_LOADSTATE7, "Load State 7"});
    hotkey_list.push_back({{SingleInput::IT_KEYBOARD, XK_F8}, HOTKEY_LOADSTATE8, "Load State 8"});
    hotkey_list.push_back({{SingleInput::IT_KEYBOARD, XK_F9}, HOTKEY_LOADSTATE9, "Load State 9"});
    hotkey_list.push_back({{SingleInput::IT_NONE, 0}, HOTKEY_TOGGLE_ENCODE, "Toggle encode"});

    /* Set default hotkeys */
    default_hotkeys();

    /* Gather the list of valid X11 KeyCode values */
    xcb_keycode_t min_keycode = xcb_get_setup(conn)->min_keycode;
    xcb_keycode_t max_keycode = xcb_get_setup(conn)->max_keycode;

    /* Build the list of keysym values to be mapped based on valid keycodes.
     * This list is dependent on the keyboard layout.
     */
    xcb_key_symbols_t *keysyms;
    if (!(keysyms = xcb_key_symbols_alloc(conn))) {
        // std::cerr << "Could not allocate key symbols" << std::endl;
        return;
    }

    for (int k=min_keycode; k<=max_keycode; k++) {
        xcb_keysym_t ks = xcb_key_symbols_get_keysym(keysyms, k, 0);
        // KeySym ks = XkbKeycodeToKeysym(display, k, 0, 0);
        if (ks == XCB_NO_SYMBOL) continue;

        SingleInput si;
        si.type = SingleInput::IT_KEYBOARD;
        si.value = static_cast<int>(ks);
        si.description = XKeysymToString(ks); // AFAIK there is no xcb counterpart to this...
        input_list.push_back(si);
    }

    xcb_key_symbols_free(keysyms);

    /* Add mouse mapping */
    input_list.push_back({SingleInput::IT_POINTER_X, 1, "Mouse X coord"});
    input_list.push_back({SingleInput::IT_POINTER_Y, 1, "Mouse Y coord"});
    input_list.push_back({SingleInput::IT_POINTER_B1, 1, "Mouse button 1"});
    input_list.push_back({SingleInput::IT_POINTER_B2, 1, "Mouse button 2"});
    input_list.push_back({SingleInput::IT_POINTER_B3, 1, "Mouse button 3"});
    input_list.push_back({SingleInput::IT_POINTER_B4, 1, "Mouse button 4"});
    input_list.push_back({SingleInput::IT_POINTER_B5, 1, "Mouse button 5"});

    /* Add controller mapping */
    input_list.push_back({SingleInput::IT_CONTROLLER1_BUTTON_A, 1, "Controller 1 - A"});
    input_list.push_back({SingleInput::IT_CONTROLLER1_BUTTON_B, 1, "Controller 1 - B"});
    input_list.push_back({SingleInput::IT_CONTROLLER1_BUTTON_X, 1, "Controller 1 - X"});
    input_list.push_back({SingleInput::IT_CONTROLLER1_BUTTON_Y, 1, "Controller 1 - Y"});
    input_list.push_back({SingleInput::IT_CONTROLLER1_BUTTON_BACK, 1, "Controller 1 - Back"});
    input_list.push_back({SingleInput::IT_CONTROLLER1_BUTTON_GUIDE, 1, "Controller 1 - Guide"});
    input_list.push_back({SingleInput::IT_CONTROLLER1_BUTTON_START, 1, "Controller 1 - Start"});
    input_list.push_back({SingleInput::IT_CONTROLLER1_BUTTON_LEFTSTICK, 1, "Controller 1 - Left Stick"});
    input_list.push_back({SingleInput::IT_CONTROLLER1_BUTTON_RIGHTSTICK, 1, "Controller 1 - Right Stick"});
    input_list.push_back({SingleInput::IT_CONTROLLER1_BUTTON_LEFTSHOULDER, 1, "Controller 1 - Left Shoulder"});
    input_list.push_back({SingleInput::IT_CONTROLLER1_BUTTON_RIGHTSHOULDER, 1, "Controller 1 - Right Shoulder"});
    input_list.push_back({SingleInput::IT_CONTROLLER1_BUTTON_DPAD_UP, 1, "Controller 1 - Dpad Up"});
    input_list.push_back({SingleInput::IT_CONTROLLER1_BUTTON_DPAD_DOWN, 1, "Controller 1 - Dpad Down"});
    input_list.push_back({SingleInput::IT_CONTROLLER1_BUTTON_DPAD_LEFT, 1, "Controller 1 - Dpad Left"});
    input_list.push_back({SingleInput::IT_CONTROLLER1_BUTTON_DPAD_RIGHT, 1, "Controller 1 - Dpad Right"});

    input_list.push_back({SingleInput::IT_CONTROLLER1_AXIS_LEFTX, 1, "Controller 1 - Left Stick X"});
    input_list.push_back({SingleInput::IT_CONTROLLER1_AXIS_LEFTY, 1, "Controller 1 - Left Stick Y"});
    input_list.push_back({SingleInput::IT_CONTROLLER1_AXIS_RIGHTX, 1, "Controller 1 - Right Stick X"});
    input_list.push_back({SingleInput::IT_CONTROLLER1_AXIS_RIGHTY, 1, "Controller 1 - Right Stick Y"});
    input_list.push_back({SingleInput::IT_CONTROLLER1_AXIS_TRIGGERLEFT, 1, "Controller 1 - Left Trigger"});
    input_list.push_back({SingleInput::IT_CONTROLLER1_AXIS_TRIGGERRIGHT, 1, "Controller 1 - Right Trigger"});

    input_list.push_back({SingleInput::IT_CONTROLLER2_BUTTON_A, 1, "Controller 2 - A"});
    input_list.push_back({SingleInput::IT_CONTROLLER2_BUTTON_B, 1, "Controller 2 - B"});
    input_list.push_back({SingleInput::IT_CONTROLLER2_BUTTON_X, 1, "Controller 2 - X"});
    input_list.push_back({SingleInput::IT_CONTROLLER2_BUTTON_Y, 1, "Controller 2 - Y"});
    input_list.push_back({SingleInput::IT_CONTROLLER2_BUTTON_BACK, 1, "Controller 2 - Back"});
    input_list.push_back({SingleInput::IT_CONTROLLER2_BUTTON_GUIDE, 1, "Controller 2 - Guide"});
    input_list.push_back({SingleInput::IT_CONTROLLER2_BUTTON_START, 1, "Controller 2 - Start"});
    input_list.push_back({SingleInput::IT_CONTROLLER2_BUTTON_LEFTSTICK, 1, "Controller 2 - Left Stick"});
    input_list.push_back({SingleInput::IT_CONTROLLER2_BUTTON_RIGHTSTICK, 1, "Controller 2 - Right Stick"});
    input_list.push_back({SingleInput::IT_CONTROLLER2_BUTTON_LEFTSHOULDER, 1, "Controller 2 - Left Shoulder"});
    input_list.push_back({SingleInput::IT_CONTROLLER2_BUTTON_RIGHTSHOULDER, 1, "Controller 2 - Right Shoulder"});
    input_list.push_back({SingleInput::IT_CONTROLLER2_BUTTON_DPAD_UP, 1, "Controller 2 - Dpad Up"});
    input_list.push_back({SingleInput::IT_CONTROLLER2_BUTTON_DPAD_DOWN, 1, "Controller 2 - Dpad Down"});
    input_list.push_back({SingleInput::IT_CONTROLLER2_BUTTON_DPAD_LEFT, 1, "Controller 2 - Dpad Left"});
    input_list.push_back({SingleInput::IT_CONTROLLER2_BUTTON_DPAD_RIGHT, 1, "Controller 2 - Dpad Right"});

    input_list.push_back({SingleInput::IT_CONTROLLER2_AXIS_LEFTX, 1, "Controller 2 - Left Stick X"});
    input_list.push_back({SingleInput::IT_CONTROLLER2_AXIS_LEFTY, 1, "Controller 2 - Left Stick Y"});
    input_list.push_back({SingleInput::IT_CONTROLLER2_AXIS_RIGHTX, 1, "Controller 2 - Right Stick X"});
    input_list.push_back({SingleInput::IT_CONTROLLER2_AXIS_RIGHTY, 1, "Controller 2 - Right Stick Y"});
    input_list.push_back({SingleInput::IT_CONTROLLER2_AXIS_TRIGGERLEFT, 1, "Controller 2 - Left Trigger"});
    input_list.push_back({SingleInput::IT_CONTROLLER2_AXIS_TRIGGERRIGHT, 1, "Controller 2 - Right Trigger"});

    input_list.push_back({SingleInput::IT_CONTROLLER3_BUTTON_A, 1, "Controller 3 - A"});
    input_list.push_back({SingleInput::IT_CONTROLLER3_BUTTON_B, 1, "Controller 3 - B"});
    input_list.push_back({SingleInput::IT_CONTROLLER3_BUTTON_X, 1, "Controller 3 - X"});
    input_list.push_back({SingleInput::IT_CONTROLLER3_BUTTON_Y, 1, "Controller 3 - Y"});
    input_list.push_back({SingleInput::IT_CONTROLLER3_BUTTON_BACK, 1, "Controller 3 - Back"});
    input_list.push_back({SingleInput::IT_CONTROLLER3_BUTTON_GUIDE, 1, "Controller 3 - Guide"});
    input_list.push_back({SingleInput::IT_CONTROLLER3_BUTTON_START, 1, "Controller 3 - Start"});
    input_list.push_back({SingleInput::IT_CONTROLLER3_BUTTON_LEFTSTICK, 1, "Controller 3 - Left Stick"});
    input_list.push_back({SingleInput::IT_CONTROLLER3_BUTTON_RIGHTSTICK, 1, "Controller 3 - Right Stick"});
    input_list.push_back({SingleInput::IT_CONTROLLER3_BUTTON_LEFTSHOULDER, 1, "Controller 3 - Left Shoulder"});
    input_list.push_back({SingleInput::IT_CONTROLLER3_BUTTON_RIGHTSHOULDER, 1, "Controller 3 - Right Shoulder"});
    input_list.push_back({SingleInput::IT_CONTROLLER3_BUTTON_DPAD_UP, 1, "Controller 3 - Dpad Up"});
    input_list.push_back({SingleInput::IT_CONTROLLER3_BUTTON_DPAD_DOWN, 1, "Controller 3 - Dpad Down"});
    input_list.push_back({SingleInput::IT_CONTROLLER3_BUTTON_DPAD_LEFT, 1, "Controller 3 - Dpad Left"});
    input_list.push_back({SingleInput::IT_CONTROLLER3_BUTTON_DPAD_RIGHT, 1, "Controller 3 - Dpad Right"});

    input_list.push_back({SingleInput::IT_CONTROLLER3_AXIS_LEFTX, 1, "Controller 3 - Left Stick X"});
    input_list.push_back({SingleInput::IT_CONTROLLER3_AXIS_LEFTY, 1, "Controller 3 - Left Stick Y"});
    input_list.push_back({SingleInput::IT_CONTROLLER3_AXIS_RIGHTX, 1, "Controller 3 - Right Stick X"});
    input_list.push_back({SingleInput::IT_CONTROLLER3_AXIS_RIGHTY, 1, "Controller 3 - Right Stick Y"});
    input_list.push_back({SingleInput::IT_CONTROLLER3_AXIS_TRIGGERLEFT, 1, "Controller 3 - Left Trigger"});
    input_list.push_back({SingleInput::IT_CONTROLLER3_AXIS_TRIGGERRIGHT, 1, "Controller 3 - Right Trigger"});

    input_list.push_back({SingleInput::IT_CONTROLLER4_BUTTON_A, 1, "Controller 4 - A"});
    input_list.push_back({SingleInput::IT_CONTROLLER4_BUTTON_B, 1, "Controller 4 - B"});
    input_list.push_back({SingleInput::IT_CONTROLLER4_BUTTON_X, 1, "Controller 4 - X"});
    input_list.push_back({SingleInput::IT_CONTROLLER4_BUTTON_Y, 1, "Controller 4 - Y"});
    input_list.push_back({SingleInput::IT_CONTROLLER4_BUTTON_BACK, 1, "Controller 4 - Back"});
    input_list.push_back({SingleInput::IT_CONTROLLER4_BUTTON_GUIDE, 1, "Controller 4 - Guide"});
    input_list.push_back({SingleInput::IT_CONTROLLER4_BUTTON_START, 1, "Controller 4 - Start"});
    input_list.push_back({SingleInput::IT_CONTROLLER4_BUTTON_LEFTSTICK, 1, "Controller 4 - Left Stick"});
    input_list.push_back({SingleInput::IT_CONTROLLER4_BUTTON_RIGHTSTICK, 1, "Controller 4 - Right Stick"});
    input_list.push_back({SingleInput::IT_CONTROLLER4_BUTTON_LEFTSHOULDER, 1, "Controller 4 - Left Shoulder"});
    input_list.push_back({SingleInput::IT_CONTROLLER4_BUTTON_RIGHTSHOULDER, 1, "Controller 4 - Right Shoulder"});
    input_list.push_back({SingleInput::IT_CONTROLLER4_BUTTON_DPAD_UP, 1, "Controller 4 - Dpad Up"});
    input_list.push_back({SingleInput::IT_CONTROLLER4_BUTTON_DPAD_DOWN, 1, "Controller 4 - Dpad Down"});
    input_list.push_back({SingleInput::IT_CONTROLLER4_BUTTON_DPAD_LEFT, 1, "Controller 4 - Dpad Left"});
    input_list.push_back({SingleInput::IT_CONTROLLER4_BUTTON_DPAD_RIGHT, 1, "Controller 4 - Dpad Right"});

    input_list.push_back({SingleInput::IT_CONTROLLER4_AXIS_LEFTX, 1, "Controller 4 - Left Stick X"});
    input_list.push_back({SingleInput::IT_CONTROLLER4_AXIS_LEFTY, 1, "Controller 4 - Left Stick Y"});
    input_list.push_back({SingleInput::IT_CONTROLLER4_AXIS_RIGHTX, 1, "Controller 4 - Right Stick X"});
    input_list.push_back({SingleInput::IT_CONTROLLER4_AXIS_RIGHTY, 1, "Controller 4 - Right Stick Y"});
    input_list.push_back({SingleInput::IT_CONTROLLER4_AXIS_TRIGGERLEFT, 1, "Controller 4 - Left Trigger"});
    input_list.push_back({SingleInput::IT_CONTROLLER4_AXIS_TRIGGERRIGHT, 1, "Controller 4 - Right Trigger"});

    /* Set default inputs */
    default_inputs();
}

std::string KeyMapping::input_description(KeySym ks)
{
    for (auto iter : input_list) {
        if (iter.type == SingleInput::IT_KEYBOARD) {
            if (iter.value == ks) {
                return iter.description;
            }
        }
    }

    return "";
}

void KeyMapping::default_hotkeys()
{
    hotkey_mapping.clear();
    for (auto iter : hotkey_list) {
        if (iter.default_input.type == SingleInput::IT_KEYBOARD) {
            hotkey_mapping[iter.default_input.value] = iter;
        }
    }
}

void KeyMapping::default_inputs()
{
    input_mapping.clear();
    for (auto iter : input_list) {
        if (iter.type == SingleInput::IT_KEYBOARD) {
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

    if (hk.default_input.type == SingleInput::IT_KEYBOARD)
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

    if (si.type == SingleInput::IT_KEYBOARD)
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

void KeyMapping::buildAllInputs(AllInputs& ai, xcb_connection_t *conn, xcb_window_t window, xcb_key_symbols_t *keysyms, SharedConfig& sc){
    int i,j;
    int keysym_i = 0;

    ai.emptyInputs();

    /* Get keyboard inputs */
    xcb_generic_error_t* error = nullptr;
    xcb_query_keymap_cookie_t keymap_cookie = xcb_query_keymap(conn);
    xcb_query_keymap_reply_t* keymap_reply = xcb_query_keymap_reply(conn, keymap_cookie, &error);

    if (error) {
        // std::cerr << "Could not get keymap, X error" << error->error_code << std::endl;
        free(keymap_reply);
        free(error);
        return;
    }

    unsigned char* keyboard_state = keymap_reply->keys;
    xcb_keysym_t modifiers = build_modifiers(keyboard_state, keysyms);

    for (i=0; i<32; i++) {
        if (keyboard_state[i] == 0)
            continue;
        for (j=0; j<8; j++) {
            if ((keyboard_state[i] >> j) & 0x1) {

                /* We got a pressed keycode */
                xcb_keycode_t kc = (i << 3) | j;
                /* Translating to keysym */
                xcb_keysym_t ks = xcb_key_symbols_get_keysym(keysyms, kc, 0);

                /* Check if we are dealing with a hotkey with or without modifiers */
                if (hotkey_mapping.find(ks) != hotkey_mapping.end()) {
                    /* Dealing with a hotkey, skipping */
                    continue;
                }

                if (modifiers) {
                    xcb_keysym_t ksm = ks | modifiers;
                    if (hotkey_mapping.find(ksm) != hotkey_mapping.end()) {
                        /* Dealing with a hotkey, skipping */
                        continue;
                    }
                }

                /* Checking the mapped input for that key */
                SingleInput si = {SingleInput::IT_NONE,0};
                if (input_mapping.find(ks) != input_mapping.end())
                    si = input_mapping[ks];

                if (si.type == SingleInput::IT_NONE) {
                    /* Key is mapped to nothing */
                    continue;
                }

                if (si.type == SingleInput::IT_KEYBOARD) {
                    /* Check if we support keyboard */
                    if (!sc.keyboard_support)
                        continue;

                    /* Checking the current number of keys */
                    if (keysym_i >= AllInputs::MAXKEYS) {
                        fprintf(stderr, "Reached maximum number of inputs (%d).", AllInputs::MAXKEYS);
                        continue;
                    }

                    /* Saving the key */
                    ai.keyboard[keysym_i++] = si.value;
                }

                if (si.type & SingleInput::IT_CONTROLLER_ID_MASK) {
                    /* Key is mapped to a game controller */

                    /* Getting Controller id
                     * Arithmetic on enums is bad, no?
                     */
                    int controller_i = ((si.type & SingleInput::IT_CONTROLLER_ID_MASK) >> SingleInput::IT_CONTROLLER_ID_SHIFT) - 1;

                    /* Check if we support this joystick */
                    if (controller_i >= sc.nb_controllers)
                        continue;

                    int controller_axis = si.type & SingleInput::IT_CONTROLLER_AXIS_MASK;
                    int controller_type = si.type & SingleInput::IT_CONTROLLER_TYPE_MASK;
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

    free(keymap_reply);

    if (sc.mouse_support && window) {
        /* Get the pointer position and mask */
        xcb_query_pointer_cookie_t pointer_cookie = xcb_query_pointer(conn, window);
        xcb_query_pointer_reply_t* pointer_reply = xcb_query_pointer_reply(conn, pointer_cookie, &error);

        if (error) {
            // std::cerr << "Could not get keymap, X error" << error->error_code << std::endl;
            free(pointer_reply);
            free(error);
            return;
        }

        ai.pointer_x = pointer_reply->win_x;
        ai.pointer_y = pointer_reply->win_y;
        /* We only care about the five mouse buttons */
        ai.pointer_mask = 0;
        if (pointer_reply->mask & XCB_BUTTON_MASK_1)
            ai.pointer_mask |= (0x1u << SingleInput::POINTER_B1);
        if (pointer_reply->mask & XCB_BUTTON_MASK_2)
            ai.pointer_mask |= (0x1u << SingleInput::POINTER_B2);
        if (pointer_reply->mask & XCB_BUTTON_MASK_3)
            ai.pointer_mask |= (0x1u << SingleInput::POINTER_B3);
        if (pointer_reply->mask & XCB_BUTTON_MASK_4)
            ai.pointer_mask |= (0x1u << SingleInput::POINTER_B4);
        if (pointer_reply->mask & XCB_BUTTON_MASK_5)
            ai.pointer_mask |= (0x1u << SingleInput::POINTER_B5);

        free(pointer_reply);

        /* TODO: Do something with off-screen pointer */
        // if (!onScreen) {
        //     ai.pointer_x = -1;
        //     ai.pointer_y = -1;
        // }
    }
}
