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

#ifndef LIBTAS_CONFIG_H_INCLUDED
#define LIBTAS_CONFIG_H_INCLUDED

#include <map>
#include <vector>
#include <string>
#include <X11/Xlib.h>
#include <X11/keysym.h>

enum 
{
    HOTKEY_PLAYPAUSE, // Switch from play to pause of the game
    HOTKEY_FRAMEADVANCE, // Advance one frame, also pause the game if playing
    HOTKEY_FASTFORWARD, // Enable fastforward when pressed
    HOTKEY_READWRITE, // Switch from read-only recording to write
    HOTKEY_SAVESTATE, // Save the entire state of the game
    HOTKEY_LOADSTATE, // Load the entire state of the game
    HOTKEY_LEN
};

/* Some structures to represent single inputs */
typedef int InputType; enum {
    IT_NONE = -1, /* No input */
    IT_ID = 0, /* Same input */
    IT_KEYBOARD, /* Keyboard */

    /* SDL Controller 1 */
    /* WARNING: Changing the values can break the code
     * Always make IT_CONTROLLER[x]_AXIS_LEFTX = x * IT_CONTROLLER1_AXIS_LEFTX
     */
    IT_CONTROLLER1_AXIS_LEFTX = 0x100,
    IT_CONTROLLER1_AXIS_LEFTY,
    IT_CONTROLLER1_AXIS_RIGHTX,
    IT_CONTROLLER1_AXIS_RIGHTY,
    IT_CONTROLLER1_AXIS_TRIGGERLEFT,
    IT_CONTROLLER1_AXIS_TRIGGERRIGHT,
    IT_CONTROLLER1_BUTTON_A = 0x180,
    IT_CONTROLLER1_BUTTON_B,
    IT_CONTROLLER1_BUTTON_X,
    IT_CONTROLLER1_BUTTON_Y,
    IT_CONTROLLER1_BUTTON_BACK,
    IT_CONTROLLER1_BUTTON_GUIDE,
    IT_CONTROLLER1_BUTTON_START,
    IT_CONTROLLER1_BUTTON_LEFTSTICK,
    IT_CONTROLLER1_BUTTON_RIGHTSTICK,
    IT_CONTROLLER1_BUTTON_LEFTSHOULDER,
    IT_CONTROLLER1_BUTTON_RIGHTSHOULDER,
    IT_CONTROLLER1_BUTTON_DPAD_UP,
    IT_CONTROLLER1_BUTTON_DPAD_DOWN,
    IT_CONTROLLER1_BUTTON_DPAD_LEFT,
    IT_CONTROLLER1_BUTTON_DPAD_RIGHT,

    /* SDL Controller 2 */
    IT_CONTROLLER2_AXIS_LEFTX = 0x200,
    IT_CONTROLLER2_AXIS_LEFTY,
    IT_CONTROLLER2_AXIS_RIGHTX,
    IT_CONTROLLER2_AXIS_RIGHTY,
    IT_CONTROLLER2_AXIS_TRIGGERLEFT,
    IT_CONTROLLER2_AXIS_TRIGGERRIGHT,
    IT_CONTROLLER2_BUTTON_A = 0x280,
    IT_CONTROLLER2_BUTTON_B,
    IT_CONTROLLER2_BUTTON_X,
    IT_CONTROLLER2_BUTTON_Y,
    IT_CONTROLLER2_BUTTON_BACK,
    IT_CONTROLLER2_BUTTON_GUIDE,
    IT_CONTROLLER2_BUTTON_START,
    IT_CONTROLLER2_BUTTON_LEFTSTICK,
    IT_CONTROLLER2_BUTTON_RIGHTSTICK,
    IT_CONTROLLER2_BUTTON_LEFTSHOULDER,
    IT_CONTROLLER2_BUTTON_RIGHTSHOULDER,
    IT_CONTROLLER2_BUTTON_DPAD_UP,
    IT_CONTROLLER2_BUTTON_DPAD_DOWN,
    IT_CONTROLLER2_BUTTON_DPAD_LEFT,
    IT_CONTROLLER2_BUTTON_DPAD_RIGHT,

    /* SDL Controller 3 */
    IT_CONTROLLER3_AXIS_LEFTX = 0x300,
    IT_CONTROLLER3_AXIS_LEFTY,
    IT_CONTROLLER3_AXIS_RIGHTX,
    IT_CONTROLLER3_AXIS_RIGHTY,
    IT_CONTROLLER3_AXIS_TRIGGERLEFT,
    IT_CONTROLLER3_AXIS_TRIGGERRIGHT,
    IT_CONTROLLER3_BUTTON_A = 0x380,
    IT_CONTROLLER3_BUTTON_B,
    IT_CONTROLLER3_BUTTON_X,
    IT_CONTROLLER3_BUTTON_Y,
    IT_CONTROLLER3_BUTTON_BACK,
    IT_CONTROLLER3_BUTTON_GUIDE,
    IT_CONTROLLER3_BUTTON_START,
    IT_CONTROLLER3_BUTTON_LEFTSTICK,
    IT_CONTROLLER3_BUTTON_RIGHTSTICK,
    IT_CONTROLLER3_BUTTON_LEFTSHOULDER,
    IT_CONTROLLER3_BUTTON_RIGHTSHOULDER,
    IT_CONTROLLER3_BUTTON_DPAD_UP,
    IT_CONTROLLER3_BUTTON_DPAD_DOWN,
    IT_CONTROLLER3_BUTTON_DPAD_LEFT,
    IT_CONTROLLER3_BUTTON_DPAD_RIGHT,

    /* SDL Controller 4 */
    IT_CONTROLLER4_AXIS_LEFTX = 0x400,
    IT_CONTROLLER4_AXIS_LEFTY,
    IT_CONTROLLER4_AXIS_RIGHTX,
    IT_CONTROLLER4_AXIS_RIGHTY,
    IT_CONTROLLER4_AXIS_TRIGGERLEFT,
    IT_CONTROLLER4_AXIS_TRIGGERRIGHT,
    IT_CONTROLLER4_BUTTON_A = 0x480,
    IT_CONTROLLER4_BUTTON_B,
    IT_CONTROLLER4_BUTTON_X,
    IT_CONTROLLER4_BUTTON_Y,
    IT_CONTROLLER4_BUTTON_BACK,
    IT_CONTROLLER4_BUTTON_GUIDE,
    IT_CONTROLLER4_BUTTON_START,
    IT_CONTROLLER4_BUTTON_LEFTSTICK,
    IT_CONTROLLER4_BUTTON_RIGHTSTICK,
    IT_CONTROLLER4_BUTTON_LEFTSHOULDER,
    IT_CONTROLLER4_BUTTON_RIGHTSHOULDER,
    IT_CONTROLLER4_BUTTON_DPAD_UP,
    IT_CONTROLLER4_BUTTON_DPAD_DOWN,
    IT_CONTROLLER4_BUTTON_DPAD_LEFT,
    IT_CONTROLLER4_BUTTON_DPAD_RIGHT,
};

struct SingleInput {
    InputType type;
    int value;
    std::string description;

    bool operator==( const SingleInput &si ) const {
        return (type == si.type) && (value == si.value);
    }

    bool operator<( const SingleInput &si ) const {
        return ((type < si.type) || ((type == si.type) && (value < si.value)));
    }
};

/* Structure holding program configuration that is saved in a file */

class Config {
    public:
        /* Display frame count in the HUD */
        bool hud_framecount;

        /* Display inputs in the HUD */
        bool hud_inputs;

        /* Prevent the game to write into savefiles */
        bool prevent_savefiles;

        /* Map keyboard KeySym to a single input of a keyboard or controller */
        std::map<KeySym,SingleInput> input_mapping;

        /* List of inputs that can be mapped to a single key */
        std::vector<SingleInput> input_list;

        /* Mapping of hotkeys */
        KeySym hotkeys[HOTKEY_LEN];

        /* Initialize hotkeys and mapping list */
        void default_hotkeys();

        void save_config();

        void load_config();

};

extern struct Config config;

#endif

