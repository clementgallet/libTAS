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

#include "KeyMapping.h"
#include "../external/keysymdef.h"
#include "../shared/inputs/AllInputs.h"
#include "../shared/SharedConfig.h"

#include <cstring>
#include <iostream>

QDataStream &operator<<(QDataStream &out, const SingleInput &obj) {
    out << obj.type << obj.which;
    return out;
}

QDataStream &operator>>(QDataStream &in, SingleInput &obj) {
    in >> obj.type >> obj.which;
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
    hotkey_list.push_back({{SingleInput::IT_KEYBOARD, XK_F10 | XK_Shift_Flag}, HOTKEY_SAVESTATE10, "Save State 10"});
    hotkey_list.push_back({{SingleInput::IT_KEYBOARD, XK_F1}, HOTKEY_LOADSTATE1, "Load State 1"});
    hotkey_list.push_back({{SingleInput::IT_KEYBOARD, XK_F2}, HOTKEY_LOADSTATE2, "Load State 2"});
    hotkey_list.push_back({{SingleInput::IT_KEYBOARD, XK_F3}, HOTKEY_LOADSTATE3, "Load State 3"});
    hotkey_list.push_back({{SingleInput::IT_KEYBOARD, XK_F4}, HOTKEY_LOADSTATE4, "Load State 4"});
    hotkey_list.push_back({{SingleInput::IT_KEYBOARD, XK_F5}, HOTKEY_LOADSTATE5, "Load State 5"});
    hotkey_list.push_back({{SingleInput::IT_KEYBOARD, XK_F6}, HOTKEY_LOADSTATE6, "Load State 6"});
    hotkey_list.push_back({{SingleInput::IT_KEYBOARD, XK_F7}, HOTKEY_LOADSTATE7, "Load State 7"});
    hotkey_list.push_back({{SingleInput::IT_KEYBOARD, XK_F8}, HOTKEY_LOADSTATE8, "Load State 8"});
    hotkey_list.push_back({{SingleInput::IT_KEYBOARD, XK_F9}, HOTKEY_LOADSTATE9, "Load State 9"});
    hotkey_list.push_back({{SingleInput::IT_KEYBOARD, XK_F10}, HOTKEY_LOADSTATE10, "Load State 10"});
    hotkey_list.push_back({{SingleInput::IT_KEYBOARD, XK_F1 | XK_Control_Flag}, HOTKEY_LOADBRANCH1, "Load Branch 1"});
    hotkey_list.push_back({{SingleInput::IT_KEYBOARD, XK_F2 | XK_Control_Flag}, HOTKEY_LOADBRANCH2, "Load Branch 2"});
    hotkey_list.push_back({{SingleInput::IT_KEYBOARD, XK_F3 | XK_Control_Flag}, HOTKEY_LOADBRANCH3, "Load Branch 3"});
    hotkey_list.push_back({{SingleInput::IT_KEYBOARD, XK_F4 | XK_Control_Flag}, HOTKEY_LOADBRANCH4, "Load Branch 4"});
    hotkey_list.push_back({{SingleInput::IT_KEYBOARD, XK_F5 | XK_Control_Flag}, HOTKEY_LOADBRANCH5, "Load Branch 5"});
    hotkey_list.push_back({{SingleInput::IT_KEYBOARD, XK_F6 | XK_Control_Flag}, HOTKEY_LOADBRANCH6, "Load Branch 6"});
    hotkey_list.push_back({{SingleInput::IT_KEYBOARD, XK_F7 | XK_Control_Flag}, HOTKEY_LOADBRANCH7, "Load Branch 7"});
    hotkey_list.push_back({{SingleInput::IT_KEYBOARD, XK_F8 | XK_Control_Flag}, HOTKEY_LOADBRANCH8, "Load Branch 8"});
    hotkey_list.push_back({{SingleInput::IT_KEYBOARD, XK_F9 | XK_Control_Flag}, HOTKEY_LOADBRANCH9, "Load Branch 9"});
    hotkey_list.push_back({{SingleInput::IT_KEYBOARD, XK_F10 | XK_Control_Flag}, HOTKEY_LOADBRANCH10, "Load Branch 10"});
    hotkey_list.push_back({{SingleInput::IT_NONE, 0}, HOTKEY_SCREENSHOT, "Screenshot"});
    hotkey_list.push_back({{SingleInput::IT_NONE, 0}, HOTKEY_TOGGLE_ENCODE, "Toggle encode"});

    /* Add flags mapping */
    input_list[INPUTLIST_FLAG].push_back({SingleInput::IT_FLAG, SingleInput::FLAG_RESTART, "Restart"});
    input_list[INPUTLIST_FLAG].push_back({SingleInput::IT_FLAG, SingleInput::FLAG_CONTROLLER1_ADDED_REMOVED, "Joy1 Added/Removed"});
    input_list[INPUTLIST_FLAG].push_back({SingleInput::IT_FLAG, SingleInput::FLAG_CONTROLLER2_ADDED_REMOVED, "Joy2 Added/Removed"});
    input_list[INPUTLIST_FLAG].push_back({SingleInput::IT_FLAG, SingleInput::FLAG_CONTROLLER3_ADDED_REMOVED, "Joy3 Added/Removed"});
    input_list[INPUTLIST_FLAG].push_back({SingleInput::IT_FLAG, SingleInput::FLAG_CONTROLLER4_ADDED_REMOVED, "Joy4 Added/Removed"});
    input_list[INPUTLIST_FLAG].push_back({SingleInput::IT_FLAG, SingleInput::FLAG_FOCUS_UNFOCUS, "Focus/Unfocus window"});

    /* Add mouse button mapping */
    input_list[INPUTLIST_MOUSE].push_back({SingleInput::IT_POINTER_BUTTON, SingleInput::POINTER_B1, "Mouse button 1"});
    input_list[INPUTLIST_MOUSE].push_back({SingleInput::IT_POINTER_BUTTON, SingleInput::POINTER_B2, "Mouse button 2"});
    input_list[INPUTLIST_MOUSE].push_back({SingleInput::IT_POINTER_BUTTON, SingleInput::POINTER_B3, "Mouse button 3"});
    input_list[INPUTLIST_MOUSE].push_back({SingleInput::IT_POINTER_BUTTON, SingleInput::POINTER_B4, "Mouse button 4"});
    input_list[INPUTLIST_MOUSE].push_back({SingleInput::IT_POINTER_BUTTON, SingleInput::POINTER_B5, "Mouse button 5"});
    input_list[INPUTLIST_MOUSE].push_back({SingleInput::IT_POINTER_MODE, 1, "Mouse rel"});

    /* Add controller mapping */
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER1_BUTTON, SingleInput::BUTTON_A, "Joy1 A"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER1_BUTTON, SingleInput::BUTTON_B, "Joy1 B"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER1_BUTTON, SingleInput::BUTTON_X, "Joy1 X"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER1_BUTTON, SingleInput::BUTTON_Y, "Joy1 Y"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER1_BUTTON, SingleInput::BUTTON_BACK, "Joy1 Back"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER1_BUTTON, SingleInput::BUTTON_GUIDE, "Joy1 Guide"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER1_BUTTON, SingleInput::BUTTON_START, "Joy1 Start"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER1_BUTTON, SingleInput::BUTTON_LEFTSTICK, "Joy1 LeftStick"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER1_BUTTON, SingleInput::BUTTON_RIGHTSTICK, "Joy1 RightStick"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER1_BUTTON, SingleInput::BUTTON_LEFTSHOULDER, "Joy1 LeftShoulder"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER1_BUTTON, SingleInput::BUTTON_RIGHTSHOULDER, "Joy1 RightShoulder"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER1_BUTTON, SingleInput::BUTTON_DPAD_UP, "Joy1 Up"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER1_BUTTON, SingleInput::BUTTON_DPAD_DOWN, "Joy1 Down"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER1_BUTTON, SingleInput::BUTTON_DPAD_LEFT, "Joy1 Left"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER1_BUTTON, SingleInput::BUTTON_DPAD_RIGHT, "Joy1 Right"});

    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER2_BUTTON, SingleInput::BUTTON_A, "Joy2 A"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER2_BUTTON, SingleInput::BUTTON_B, "Joy2 B"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER2_BUTTON, SingleInput::BUTTON_X, "Joy2 X"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER2_BUTTON, SingleInput::BUTTON_Y, "Joy2 Y"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER2_BUTTON, SingleInput::BUTTON_BACK, "Joy2 Back"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER2_BUTTON, SingleInput::BUTTON_GUIDE, "Joy2 Guide"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER2_BUTTON, SingleInput::BUTTON_START, "Joy2 Start"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER2_BUTTON, SingleInput::BUTTON_LEFTSTICK, "Joy2 LeftStick"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER2_BUTTON, SingleInput::BUTTON_RIGHTSTICK, "Joy2 RightStick"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER2_BUTTON, SingleInput::BUTTON_LEFTSHOULDER, "Joy2 LeftShoulder"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER2_BUTTON, SingleInput::BUTTON_RIGHTSHOULDER, "Joy2 RightShoulder"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER2_BUTTON, SingleInput::BUTTON_DPAD_UP, "Joy2 Up"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER2_BUTTON, SingleInput::BUTTON_DPAD_DOWN, "Joy2 Down"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER2_BUTTON, SingleInput::BUTTON_DPAD_LEFT, "Joy2 Left"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER2_BUTTON, SingleInput::BUTTON_DPAD_RIGHT, "Joy2 Right"});

    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER3_BUTTON, SingleInput::BUTTON_A, "Joy3 A"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER3_BUTTON, SingleInput::BUTTON_B, "Joy3 B"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER3_BUTTON, SingleInput::BUTTON_X, "Joy3 X"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER3_BUTTON, SingleInput::BUTTON_Y, "Joy3 Y"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER3_BUTTON, SingleInput::BUTTON_BACK, "Joy3 Back"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER3_BUTTON, SingleInput::BUTTON_GUIDE, "Joy3 Guide"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER3_BUTTON, SingleInput::BUTTON_START, "Joy3 Start"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER3_BUTTON, SingleInput::BUTTON_LEFTSTICK, "Joy3 LeftStick"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER3_BUTTON, SingleInput::BUTTON_RIGHTSTICK, "Joy3 RightStick"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER3_BUTTON, SingleInput::BUTTON_LEFTSHOULDER, "Joy3 LeftShoulder"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER3_BUTTON, SingleInput::BUTTON_RIGHTSHOULDER, "Joy3 RightShoulder"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER3_BUTTON, SingleInput::BUTTON_DPAD_UP, "Joy3 Up"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER3_BUTTON, SingleInput::BUTTON_DPAD_DOWN, "Joy3 Down"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER3_BUTTON, SingleInput::BUTTON_DPAD_LEFT, "Joy3 Left"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER3_BUTTON, SingleInput::BUTTON_DPAD_RIGHT, "Joy3 Right"});

    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER4_BUTTON, SingleInput::BUTTON_A, "Joy4 A"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER4_BUTTON, SingleInput::BUTTON_B, "Joy4 B"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER4_BUTTON, SingleInput::BUTTON_X, "Joy4 X"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER4_BUTTON, SingleInput::BUTTON_Y, "Joy4 Y"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER4_BUTTON, SingleInput::BUTTON_BACK, "Joy4 Back"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER4_BUTTON, SingleInput::BUTTON_GUIDE, "Joy4 Guide"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER4_BUTTON, SingleInput::BUTTON_START, "Joy4 Start"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER4_BUTTON, SingleInput::BUTTON_LEFTSTICK, "Joy4 LeftStick"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER4_BUTTON, SingleInput::BUTTON_RIGHTSTICK, "Joy4 RightStick"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER4_BUTTON, SingleInput::BUTTON_LEFTSHOULDER, "Joy4 LeftShoulder"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER4_BUTTON, SingleInput::BUTTON_RIGHTSHOULDER, "Joy4 RightShoulder"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER4_BUTTON, SingleInput::BUTTON_DPAD_UP, "Joy4 Up"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER4_BUTTON, SingleInput::BUTTON_DPAD_DOWN, "Joy4 Down"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER4_BUTTON, SingleInput::BUTTON_DPAD_LEFT, "Joy4 Left"});
    input_list[INPUTLIST_CONTROLLER].push_back({SingleInput::IT_CONTROLLER4_BUTTON, SingleInput::BUTTON_DPAD_RIGHT, "Joy4 Right"});

    /* Analog mapping. They cannot be remapped, but we need to define them
     * to have a description for input editor */
     
    /* Mouse mapping */
    input_list[INPUTLIST_ANALOG].push_back({SingleInput::IT_POINTER_X, 1, "Mouse X coord"});
    input_list[INPUTLIST_ANALOG].push_back({SingleInput::IT_POINTER_Y, 1, "Mouse Y coord"});
    input_list[INPUTLIST_ANALOG].push_back({SingleInput::IT_POINTER_WHEEL, 1, "Mouse wheel"});

    /* Framerate mapping */
    input_list[INPUTLIST_ANALOG].push_back({SingleInput::IT_FRAMERATE_NUM, 1, "Framerate num"});
    input_list[INPUTLIST_ANALOG].push_back({SingleInput::IT_FRAMERATE_DEN, 1, "Framerate den"});
    
    /* Realtime mapping */
    input_list[INPUTLIST_ANALOG].push_back({SingleInput::IT_REALTIME_SEC, 1, "Realtime sec"});
    input_list[INPUTLIST_ANALOG].push_back({SingleInput::IT_REALTIME_NSEC, 1, "Realtime nsec"});
    
    /* Add controller mapping */
    input_list[INPUTLIST_ANALOG].push_back({SingleInput::IT_CONTROLLER1_AXIS, SingleInput::AXIS_LEFTX, "Joy1 LeftStickX"});
    input_list[INPUTLIST_ANALOG].push_back({SingleInput::IT_CONTROLLER1_AXIS, SingleInput::AXIS_LEFTY, "Joy1 LeftStickY"});
    input_list[INPUTLIST_ANALOG].push_back({SingleInput::IT_CONTROLLER1_AXIS, SingleInput::AXIS_RIGHTX, "Joy1 RightStickX"});
    input_list[INPUTLIST_ANALOG].push_back({SingleInput::IT_CONTROLLER1_AXIS, SingleInput::AXIS_RIGHTY, "Joy1 RightStickY"});
    input_list[INPUTLIST_ANALOG].push_back({SingleInput::IT_CONTROLLER1_AXIS, SingleInput::AXIS_TRIGGERLEFT, "Joy1 LeftTrigger"});
    input_list[INPUTLIST_ANALOG].push_back({SingleInput::IT_CONTROLLER1_AXIS, SingleInput::AXIS_TRIGGERRIGHT, "Joy1 RightTrigger"});

    input_list[INPUTLIST_ANALOG].push_back({SingleInput::IT_CONTROLLER2_AXIS, SingleInput::AXIS_LEFTX, "Joy2 LeftStickX"});
    input_list[INPUTLIST_ANALOG].push_back({SingleInput::IT_CONTROLLER2_AXIS, SingleInput::AXIS_LEFTY, "Joy2 LeftStickY"});
    input_list[INPUTLIST_ANALOG].push_back({SingleInput::IT_CONTROLLER2_AXIS, SingleInput::AXIS_RIGHTX, "Joy2 RightStickX"});
    input_list[INPUTLIST_ANALOG].push_back({SingleInput::IT_CONTROLLER2_AXIS, SingleInput::AXIS_RIGHTY, "Joy2 RightStickY"});
    input_list[INPUTLIST_ANALOG].push_back({SingleInput::IT_CONTROLLER2_AXIS, SingleInput::AXIS_TRIGGERLEFT, "Joy2 LeftTrigger"});
    input_list[INPUTLIST_ANALOG].push_back({SingleInput::IT_CONTROLLER2_AXIS, SingleInput::AXIS_TRIGGERRIGHT, "Joy2 RightTrigger"});

    input_list[INPUTLIST_ANALOG].push_back({SingleInput::IT_CONTROLLER3_AXIS, SingleInput::AXIS_LEFTX, "Joy3 LeftStickX"});
    input_list[INPUTLIST_ANALOG].push_back({SingleInput::IT_CONTROLLER3_AXIS, SingleInput::AXIS_LEFTY, "Joy3 LeftStickY"});
    input_list[INPUTLIST_ANALOG].push_back({SingleInput::IT_CONTROLLER3_AXIS, SingleInput::AXIS_RIGHTX, "Joy3 RightStickX"});
    input_list[INPUTLIST_ANALOG].push_back({SingleInput::IT_CONTROLLER3_AXIS, SingleInput::AXIS_RIGHTY, "Joy3 RightStickY"});
    input_list[INPUTLIST_ANALOG].push_back({SingleInput::IT_CONTROLLER3_AXIS, SingleInput::AXIS_TRIGGERLEFT, "Joy3 LeftTrigger"});
    input_list[INPUTLIST_ANALOG].push_back({SingleInput::IT_CONTROLLER3_AXIS, SingleInput::AXIS_TRIGGERRIGHT, "Joy3 RightTrigger"});

    input_list[INPUTLIST_ANALOG].push_back({SingleInput::IT_CONTROLLER4_AXIS, SingleInput::AXIS_LEFTX, "Joy4 LeftStickX"});
    input_list[INPUTLIST_ANALOG].push_back({SingleInput::IT_CONTROLLER4_AXIS, SingleInput::AXIS_LEFTY, "Joy4 LeftStickY"});
    input_list[INPUTLIST_ANALOG].push_back({SingleInput::IT_CONTROLLER4_AXIS, SingleInput::AXIS_RIGHTX, "Joy4 RightStickX"});
    input_list[INPUTLIST_ANALOG].push_back({SingleInput::IT_CONTROLLER4_AXIS, SingleInput::AXIS_RIGHTY, "Joy4 RightStickY"});
    input_list[INPUTLIST_ANALOG].push_back({SingleInput::IT_CONTROLLER4_AXIS, SingleInput::AXIS_TRIGGERLEFT, "Joy4 LeftTrigger"});
    input_list[INPUTLIST_ANALOG].push_back({SingleInput::IT_CONTROLLER4_AXIS, SingleInput::AXIS_TRIGGERRIGHT, "Joy4 RightTrigger"});    
}

std::string KeyMapping::input_description(keysym_t ks)
{
    for (auto iter : input_list[INPUTLIST_KEYBOARD_LATIN]) {
        if (iter.type == SingleInput::IT_KEYBOARD) {
            if (iter.which == ks) {
                return iter.description;
            }
        }
    }

    for (auto iter : input_list[INPUTLIST_KEYBOARD_MISC]) {
        if (iter.type == SingleInput::IT_KEYBOARD) {
            if (iter.which == ks) {
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
            hotkey_mapping[iter.default_input.which] = iter;
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
        hotkey_mapping[hk.default_input.which] = hk;

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
