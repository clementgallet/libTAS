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

#include "KeyMapping.h"
#include "../external/keysymdef.h"
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

KeyMapping::KeyMapping(void* c)
{
    /* Fill hotkey list */
    hotkey_list.push_back({{SingleInput::IT_KEYBOARD, XK_Pause}, HOTKEY_PLAYPAUSE, "Play/Pause"});
    hotkey_list.push_back({{SingleInput::IT_KEYBOARD, XK_v}, HOTKEY_FRAMEADVANCE, "Frame Advance"});
    hotkey_list.push_back({{SingleInput::IT_KEYBOARD, XK_Tab}, HOTKEY_FASTFORWARD, "Fast-forward"});
    hotkey_list.push_back({{SingleInput::IT_NONE, 0}, HOTKEY_TOGGLE_FASTFORWARD, "Toggle fast-forward"});
    hotkey_list.push_back({{SingleInput::IT_NONE, 0}, HOTKEY_READWRITE, "Toggle Read/Write"});
    hotkey_list.push_back({{SingleInput::IT_KEYBOARD, XK_F1 | XK_Shift_Flag}, HOTKEY_SAVESTATE1, "Save State 1"});
    hotkey_list.push_back({{SingleInput::IT_KEYBOARD, XK_F2 | XK_Shift_Flag}, HOTKEY_SAVESTATE2, "Save State 2"});
    hotkey_list.push_back({{SingleInput::IT_KEYBOARD, XK_F3 | XK_Shift_Flag}, HOTKEY_SAVESTATE3, "Save State 3"});
    hotkey_list.push_back({{SingleInput::IT_KEYBOARD, XK_F4 | XK_Shift_Flag}, HOTKEY_SAVESTATE4, "Save State 4"});
    hotkey_list.push_back({{SingleInput::IT_KEYBOARD, XK_F5 | XK_Shift_Flag}, HOTKEY_SAVESTATE5, "Save State 5"});
    hotkey_list.push_back({{SingleInput::IT_KEYBOARD, XK_F6 | XK_Shift_Flag}, HOTKEY_SAVESTATE6, "Save State 6"});
    hotkey_list.push_back({{SingleInput::IT_KEYBOARD, XK_F7 | XK_Shift_Flag}, HOTKEY_SAVESTATE7, "Save State 7"});
    hotkey_list.push_back({{SingleInput::IT_KEYBOARD, XK_F8 | XK_Shift_Flag}, HOTKEY_SAVESTATE8, "Save State 8"});
    hotkey_list.push_back({{SingleInput::IT_KEYBOARD, XK_F9 | XK_Shift_Flag}, HOTKEY_SAVESTATE9, "Save State 9"});
    hotkey_list.push_back({{SingleInput::IT_NONE, 0}, HOTKEY_SAVESTATE_BACKTRACK, "Save Backtrack State"});
    hotkey_list.push_back({{SingleInput::IT_KEYBOARD, XK_F1}, HOTKEY_LOADSTATE1, "Load State 1"});
    hotkey_list.push_back({{SingleInput::IT_KEYBOARD, XK_F2}, HOTKEY_LOADSTATE2, "Load State 2"});
    hotkey_list.push_back({{SingleInput::IT_KEYBOARD, XK_F3}, HOTKEY_LOADSTATE3, "Load State 3"});
    hotkey_list.push_back({{SingleInput::IT_KEYBOARD, XK_F4}, HOTKEY_LOADSTATE4, "Load State 4"});
    hotkey_list.push_back({{SingleInput::IT_KEYBOARD, XK_F5}, HOTKEY_LOADSTATE5, "Load State 5"});
    hotkey_list.push_back({{SingleInput::IT_KEYBOARD, XK_F6}, HOTKEY_LOADSTATE6, "Load State 6"});
    hotkey_list.push_back({{SingleInput::IT_KEYBOARD, XK_F7}, HOTKEY_LOADSTATE7, "Load State 7"});
    hotkey_list.push_back({{SingleInput::IT_KEYBOARD, XK_F8}, HOTKEY_LOADSTATE8, "Load State 8"});
    hotkey_list.push_back({{SingleInput::IT_KEYBOARD, XK_F9}, HOTKEY_LOADSTATE9, "Load State 9"});
    hotkey_list.push_back({{SingleInput::IT_KEYBOARD, XK_F10}, HOTKEY_LOADSTATE_BACKTRACK, "Load Backtrack State"});
    hotkey_list.push_back({{SingleInput::IT_KEYBOARD, XK_F1 | XK_Control_Flag}, HOTKEY_LOADBRANCH1, "Load Branch 1"});
    hotkey_list.push_back({{SingleInput::IT_KEYBOARD, XK_F2 | XK_Control_Flag}, HOTKEY_LOADBRANCH2, "Load Branch 2"});
    hotkey_list.push_back({{SingleInput::IT_KEYBOARD, XK_F3 | XK_Control_Flag}, HOTKEY_LOADBRANCH3, "Load Branch 3"});
    hotkey_list.push_back({{SingleInput::IT_KEYBOARD, XK_F4 | XK_Control_Flag}, HOTKEY_LOADBRANCH4, "Load Branch 4"});
    hotkey_list.push_back({{SingleInput::IT_KEYBOARD, XK_F5 | XK_Control_Flag}, HOTKEY_LOADBRANCH5, "Load Branch 5"});
    hotkey_list.push_back({{SingleInput::IT_KEYBOARD, XK_F6 | XK_Control_Flag}, HOTKEY_LOADBRANCH6, "Load Branch 6"});
    hotkey_list.push_back({{SingleInput::IT_KEYBOARD, XK_F7 | XK_Control_Flag}, HOTKEY_LOADBRANCH7, "Load Branch 7"});
    hotkey_list.push_back({{SingleInput::IT_KEYBOARD, XK_F8 | XK_Control_Flag}, HOTKEY_LOADBRANCH8, "Load Branch 8"});
    hotkey_list.push_back({{SingleInput::IT_KEYBOARD, XK_F9 | XK_Control_Flag}, HOTKEY_LOADBRANCH9, "Load Branch 9"});
    hotkey_list.push_back({{SingleInput::IT_KEYBOARD, XK_F10 | XK_Control_Flag}, HOTKEY_LOADBRANCH_BACKTRACK, "Load Backtrack Branch"});
    hotkey_list.push_back({{SingleInput::IT_NONE, 0}, HOTKEY_TOGGLE_ENCODE, "Toggle encode"});

    /* Add flags mapping */
    input_list[INPUTLIST_FLAG].push_back({SingleInput::IT_FLAG, SingleInput::FLAG_RESTART, "Restart"});
    input_list[INPUTLIST_FLAG].push_back({SingleInput::IT_FLAG, SingleInput::FLAG_CONTROLLER1_ADDED, "Joy1 Added"});
    input_list[INPUTLIST_FLAG].push_back({SingleInput::IT_FLAG, SingleInput::FLAG_CONTROLLER2_ADDED, "Joy2 Added"});
    input_list[INPUTLIST_FLAG].push_back({SingleInput::IT_FLAG, SingleInput::FLAG_CONTROLLER3_ADDED, "Joy3 Added"});
    input_list[INPUTLIST_FLAG].push_back({SingleInput::IT_FLAG, SingleInput::FLAG_CONTROLLER4_ADDED, "Joy4 Added"});
    input_list[INPUTLIST_FLAG].push_back({SingleInput::IT_FLAG, SingleInput::FLAG_CONTROLLER1_REMOVED, "Joy1 Removed"});
    input_list[INPUTLIST_FLAG].push_back({SingleInput::IT_FLAG, SingleInput::FLAG_CONTROLLER2_REMOVED, "Joy2 Removed"});
    input_list[INPUTLIST_FLAG].push_back({SingleInput::IT_FLAG, SingleInput::FLAG_CONTROLLER3_REMOVED, "Joy3 Removed"});
    input_list[INPUTLIST_FLAG].push_back({SingleInput::IT_FLAG, SingleInput::FLAG_CONTROLLER4_REMOVED, "Joy4 Removed"});

    /* Add mouse button mapping */
    input_list[INPUTLIST_MOUSE].push_back({SingleInput::IT_POINTER_B1, 1, "Mouse button 1"});
    input_list[INPUTLIST_MOUSE].push_back({SingleInput::IT_POINTER_B2, 1, "Mouse button 2"});
    input_list[INPUTLIST_MOUSE].push_back({SingleInput::IT_POINTER_B3, 1, "Mouse button 3"});
    input_list[INPUTLIST_MOUSE].push_back({SingleInput::IT_POINTER_B4, 1, "Mouse button 4"});
    input_list[INPUTLIST_MOUSE].push_back({SingleInput::IT_POINTER_B5, 1, "Mouse button 5"});

    /* Add controller mapping */
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER1_BUTTON_A, 1, "Joy1 A"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER1_BUTTON_B, 1, "Joy1 B"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER1_BUTTON_X, 1, "Joy1 X"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER1_BUTTON_Y, 1, "Joy1 Y"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER1_BUTTON_BACK, 1, "Joy1 Back"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER1_BUTTON_GUIDE, 1, "Joy1 Guide"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER1_BUTTON_START, 1, "Joy1 Start"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER1_BUTTON_LEFTSTICK, 1, "Joy1 LeftStick"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER1_BUTTON_RIGHTSTICK, 1, "Joy1 RightStick"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER1_BUTTON_LEFTSHOULDER, 1, "Joy1 LeftShoulder"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER1_BUTTON_RIGHTSHOULDER, 1, "Joy1 RightShoulder"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER1_BUTTON_DPAD_UP, 1, "Joy1 Up"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER1_BUTTON_DPAD_DOWN, 1, "Joy1 Down"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER1_BUTTON_DPAD_LEFT, 1, "Joy1 Left"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER1_BUTTON_DPAD_RIGHT, 1, "Joy1 Right"});

    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER2_BUTTON_A, 1, "Joy2 A"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER2_BUTTON_B, 1, "Joy2 B"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER2_BUTTON_X, 1, "Joy2 X"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER2_BUTTON_Y, 1, "Joy2 Y"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER2_BUTTON_BACK, 1, "Joy2 Back"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER2_BUTTON_GUIDE, 1, "Joy2 Guide"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER2_BUTTON_START, 1, "Joy2 Start"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER2_BUTTON_LEFTSTICK, 1, "Joy2 LeftStick"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER2_BUTTON_RIGHTSTICK, 1, "Joy2 RightStick"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER2_BUTTON_LEFTSHOULDER, 1, "Joy2 LeftShoulder"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER2_BUTTON_RIGHTSHOULDER, 1, "Joy2 RightShoulder"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER2_BUTTON_DPAD_UP, 1, "Joy2 Up"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER2_BUTTON_DPAD_DOWN, 1, "Joy2 Down"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER2_BUTTON_DPAD_LEFT, 1, "Joy2 Left"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER2_BUTTON_DPAD_RIGHT, 1, "Joy2 Right"});

    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER3_BUTTON_A, 1, "Joy3 A"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER3_BUTTON_B, 1, "Joy3 B"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER3_BUTTON_X, 1, "Joy3 X"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER3_BUTTON_Y, 1, "Joy3 Y"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER3_BUTTON_BACK, 1, "Joy3 Back"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER3_BUTTON_GUIDE, 1, "Joy3 Guide"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER3_BUTTON_START, 1, "Joy3 Start"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER3_BUTTON_LEFTSTICK, 1, "Joy3 LeftStick"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER3_BUTTON_RIGHTSTICK, 1, "Joy3 RightStick"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER3_BUTTON_LEFTSHOULDER, 1, "Joy3 LeftShoulder"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER3_BUTTON_RIGHTSHOULDER, 1, "Joy3 RightShoulder"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER3_BUTTON_DPAD_UP, 1, "Joy3 Up"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER3_BUTTON_DPAD_DOWN, 1, "Joy3 Down"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER3_BUTTON_DPAD_LEFT, 1, "Joy3 Left"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER3_BUTTON_DPAD_RIGHT, 1, "Joy3 Right"});

    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER4_BUTTON_A, 1, "Joy4 A"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER4_BUTTON_B, 1, "Joy4 B"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER4_BUTTON_X, 1, "Joy4 X"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER4_BUTTON_Y, 1, "Joy4 Y"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER4_BUTTON_BACK, 1, "Joy4 Back"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER4_BUTTON_GUIDE, 1, "Joy4 Guide"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER4_BUTTON_START, 1, "Joy4 Start"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER4_BUTTON_LEFTSTICK, 1, "Joy4 LeftStick"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER4_BUTTON_RIGHTSTICK, 1, "Joy4 RightStick"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER4_BUTTON_LEFTSHOULDER, 1, "Joy4 LeftShoulder"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER4_BUTTON_RIGHTSHOULDER, 1, "Joy4 RightShoulder"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER4_BUTTON_DPAD_UP, 1, "Joy4 Up"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER4_BUTTON_DPAD_DOWN, 1, "Joy4 Down"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER4_BUTTON_DPAD_LEFT, 1, "Joy4 Left"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER4_BUTTON_DPAD_RIGHT, 1, "Joy4 Right"});

    /* Hidden mapping. They cannot be remapped, but we need to define them
     * to have a description for input editor */
     
    /* Mouse mapping */
    input_list[INPUTLIST_HIDDEN].push_back({SingleInput::IT_POINTER_X, 1, "Mouse X coord"});
    input_list[INPUTLIST_HIDDEN].push_back({SingleInput::IT_POINTER_Y, 1, "Mouse Y coord"});
    input_list[INPUTLIST_HIDDEN].push_back({SingleInput::IT_POINTER_MODE, 1, "Mouse rel"});

    /* Framerate mapping */
    input_list[INPUTLIST_HIDDEN].push_back({SingleInput::IT_FRAMERATE_NUM, 1, "Framerate num"});
    input_list[INPUTLIST_HIDDEN].push_back({SingleInput::IT_FRAMERATE_DEN, 1, "Framerate den"});
    
    /* Add controller mapping */
    input_list[INPUTLIST_HIDDEN].push_back({SingleInput::IT_CONTROLLER1_AXIS_LEFTX, 1, "Joy1 LeftStickX"});
    input_list[INPUTLIST_HIDDEN].push_back({SingleInput::IT_CONTROLLER1_AXIS_LEFTY, 1, "Joy1 LeftStickY"});
    input_list[INPUTLIST_HIDDEN].push_back({SingleInput::IT_CONTROLLER1_AXIS_RIGHTX, 1, "Joy1 RightStickX"});
    input_list[INPUTLIST_HIDDEN].push_back({SingleInput::IT_CONTROLLER1_AXIS_RIGHTY, 1, "Joy1 RightStickY"});
    input_list[INPUTLIST_HIDDEN].push_back({SingleInput::IT_CONTROLLER1_AXIS_TRIGGERLEFT, 1, "Joy1 LeftTrigger"});
    input_list[INPUTLIST_HIDDEN].push_back({SingleInput::IT_CONTROLLER1_AXIS_TRIGGERRIGHT, 1, "Joy1 RightTrigger"});

    input_list[INPUTLIST_HIDDEN].push_back({SingleInput::IT_CONTROLLER2_AXIS_LEFTX, 1, "Joy2 LeftStickX"});
    input_list[INPUTLIST_HIDDEN].push_back({SingleInput::IT_CONTROLLER2_AXIS_LEFTY, 1, "Joy2 LeftStickY"});
    input_list[INPUTLIST_HIDDEN].push_back({SingleInput::IT_CONTROLLER2_AXIS_RIGHTX, 1, "Joy2 RightStickX"});
    input_list[INPUTLIST_HIDDEN].push_back({SingleInput::IT_CONTROLLER2_AXIS_RIGHTY, 1, "Joy2 RightStickY"});
    input_list[INPUTLIST_HIDDEN].push_back({SingleInput::IT_CONTROLLER2_AXIS_TRIGGERLEFT, 1, "Joy2 LeftTrigger"});
    input_list[INPUTLIST_HIDDEN].push_back({SingleInput::IT_CONTROLLER2_AXIS_TRIGGERRIGHT, 1, "Joy2 RightTrigger"});

    input_list[INPUTLIST_HIDDEN].push_back({SingleInput::IT_CONTROLLER3_AXIS_LEFTX, 1, "Joy3 LeftStickX"});
    input_list[INPUTLIST_HIDDEN].push_back({SingleInput::IT_CONTROLLER3_AXIS_LEFTY, 1, "Joy3 LeftStickY"});
    input_list[INPUTLIST_HIDDEN].push_back({SingleInput::IT_CONTROLLER3_AXIS_RIGHTX, 1, "Joy3 RightStickX"});
    input_list[INPUTLIST_HIDDEN].push_back({SingleInput::IT_CONTROLLER3_AXIS_RIGHTY, 1, "Joy3 RightStickY"});
    input_list[INPUTLIST_HIDDEN].push_back({SingleInput::IT_CONTROLLER3_AXIS_TRIGGERLEFT, 1, "Joy3 LeftTrigger"});
    input_list[INPUTLIST_HIDDEN].push_back({SingleInput::IT_CONTROLLER3_AXIS_TRIGGERRIGHT, 1, "Joy3 RightTrigger"});

    input_list[INPUTLIST_HIDDEN].push_back({SingleInput::IT_CONTROLLER4_AXIS_LEFTX, 1, "Joy4 LeftStickX"});
    input_list[INPUTLIST_HIDDEN].push_back({SingleInput::IT_CONTROLLER4_AXIS_LEFTY, 1, "Joy4 LeftStickY"});
    input_list[INPUTLIST_HIDDEN].push_back({SingleInput::IT_CONTROLLER4_AXIS_RIGHTX, 1, "Joy4 RightStickX"});
    input_list[INPUTLIST_HIDDEN].push_back({SingleInput::IT_CONTROLLER4_AXIS_RIGHTY, 1, "Joy4 RightStickY"});
    input_list[INPUTLIST_HIDDEN].push_back({SingleInput::IT_CONTROLLER4_AXIS_TRIGGERLEFT, 1, "Joy4 LeftTrigger"});
    input_list[INPUTLIST_HIDDEN].push_back({SingleInput::IT_CONTROLLER4_AXIS_TRIGGERRIGHT, 1, "Joy4 RightTrigger"});    
}

std::string KeyMapping::input_description(keysym_t ks)
{
    for (auto iter : input_list[INPUTLIST_KEYBOARD_LATIN]) {
        if (iter.type == SingleInput::IT_KEYBOARD) {
            if (iter.value == ks) {
                return iter.description;
            }
        }
    }

    for (auto iter : input_list[INPUTLIST_KEYBOARD_MISC]) {
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

void KeyMapping::reassign_hotkey(int hotkey_index, keysym_t ks)
{
    /* Hotkey selected */
    HotKey hk = hotkey_list[hotkey_index];
    reassign_hotkey(hk, ks);
}

void KeyMapping::reassign_hotkey(HotKey hk, keysym_t ks)
{
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

void KeyMapping::reassign_input(int tab, int input_index, keysym_t ks)
{
    /* Input selected */
    SingleInput si = input_list[tab][input_index];
    reassign_input(si, ks);
}

void KeyMapping::reassign_input(SingleInput si, keysym_t ks)
{
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
