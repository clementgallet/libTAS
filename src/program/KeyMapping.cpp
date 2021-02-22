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
    input_list[INPUTLIST_HIDDEN].push_back({SingleInput::IT_POINTER_B1, 1, "Mouse button 1"});
    input_list[INPUTLIST_HIDDEN].push_back({SingleInput::IT_POINTER_B2, 1, "Mouse button 2"});
    input_list[INPUTLIST_HIDDEN].push_back({SingleInput::IT_POINTER_B3, 1, "Mouse button 3"});
    input_list[INPUTLIST_HIDDEN].push_back({SingleInput::IT_POINTER_B4, 1, "Mouse button 4"});
    input_list[INPUTLIST_HIDDEN].push_back({SingleInput::IT_POINTER_B5, 1, "Mouse button 5"});

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
