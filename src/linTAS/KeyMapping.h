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

#ifndef LINTAS_KEYMAPPING_H_INCLUDED
#define LINTAS_KEYMAPPING_H_INCLUDED

#include "../shared/AllInputs.h"
#include "../shared/SharedConfig.h"
#include <xcb/xcb.h>
// #include <X11/Xlib.h>
#include <X11/keysym.h>
#include <map>
#include <vector>
#include <array>
#include <string>

#define IT_CONTROLLER_TYPE_MASK  0x7F
#define IT_CONTROLLER_ID_SHIFT 8
#define IT_CONTROLLER_ID_MASK  0xF00
#define IT_CONTROLLER_AXIS_MASK 0x80

/* Some structures to represent single inputs */
typedef int InputType; enum {
    IT_NONE = -1, /* No input */
    IT_KEYBOARD = 0, /* Keyboard */

    /* Controller 1 */
    IT_CONTROLLER1_BUTTON_A = (1 << IT_CONTROLLER_ID_SHIFT) + AllInputs::BUTTON_A,
    IT_CONTROLLER1_BUTTON_B = (1 << IT_CONTROLLER_ID_SHIFT) + AllInputs::BUTTON_B,
    IT_CONTROLLER1_BUTTON_X = (1 << IT_CONTROLLER_ID_SHIFT) + AllInputs::BUTTON_X,
    IT_CONTROLLER1_BUTTON_Y = (1 << IT_CONTROLLER_ID_SHIFT) + AllInputs::BUTTON_Y,
    IT_CONTROLLER1_BUTTON_BACK = (1 << IT_CONTROLLER_ID_SHIFT) + AllInputs::BUTTON_BACK,
    IT_CONTROLLER1_BUTTON_GUIDE = (1 << IT_CONTROLLER_ID_SHIFT) + AllInputs::BUTTON_GUIDE,
    IT_CONTROLLER1_BUTTON_START = (1 << IT_CONTROLLER_ID_SHIFT) + AllInputs::BUTTON_START,
    IT_CONTROLLER1_BUTTON_LEFTSTICK = (1 << IT_CONTROLLER_ID_SHIFT) + AllInputs::BUTTON_LEFTSTICK,
    IT_CONTROLLER1_BUTTON_RIGHTSTICK = (1 << IT_CONTROLLER_ID_SHIFT) + AllInputs::BUTTON_RIGHTSTICK,
    IT_CONTROLLER1_BUTTON_LEFTSHOULDER = (1 << IT_CONTROLLER_ID_SHIFT) + AllInputs::BUTTON_LEFTSHOULDER,
    IT_CONTROLLER1_BUTTON_RIGHTSHOULDER = (1 << IT_CONTROLLER_ID_SHIFT) + AllInputs::BUTTON_RIGHTSHOULDER,
    IT_CONTROLLER1_BUTTON_DPAD_UP = (1 << IT_CONTROLLER_ID_SHIFT) + AllInputs::BUTTON_DPAD_UP,
    IT_CONTROLLER1_BUTTON_DPAD_DOWN = (1 << IT_CONTROLLER_ID_SHIFT) + AllInputs::BUTTON_DPAD_DOWN,
    IT_CONTROLLER1_BUTTON_DPAD_LEFT = (1 << IT_CONTROLLER_ID_SHIFT) + AllInputs::BUTTON_DPAD_LEFT,
    IT_CONTROLLER1_BUTTON_DPAD_RIGHT = (1 << IT_CONTROLLER_ID_SHIFT) + AllInputs::BUTTON_DPAD_RIGHT,
    IT_CONTROLLER1_AXIS_LEFTX = ((1 << IT_CONTROLLER_ID_SHIFT) | IT_CONTROLLER_AXIS_MASK) + AllInputs::AXIS_LEFTX,
    IT_CONTROLLER1_AXIS_LEFTY = ((1 << IT_CONTROLLER_ID_SHIFT) | IT_CONTROLLER_AXIS_MASK) + AllInputs::AXIS_LEFTY,
    IT_CONTROLLER1_AXIS_RIGHTX = ((1 << IT_CONTROLLER_ID_SHIFT) | IT_CONTROLLER_AXIS_MASK) + AllInputs::AXIS_RIGHTX,
    IT_CONTROLLER1_AXIS_RIGHTY = ((1 << IT_CONTROLLER_ID_SHIFT) | IT_CONTROLLER_AXIS_MASK) + AllInputs::AXIS_RIGHTY,
    IT_CONTROLLER1_AXIS_TRIGGERLEFT = ((1 << IT_CONTROLLER_ID_SHIFT) | IT_CONTROLLER_AXIS_MASK) + AllInputs::AXIS_TRIGGERLEFT,
    IT_CONTROLLER1_AXIS_TRIGGERRIGHT = ((1 << IT_CONTROLLER_ID_SHIFT) | IT_CONTROLLER_AXIS_MASK) + AllInputs::AXIS_TRIGGERRIGHT,

    /* Controller 2 */
    IT_CONTROLLER2_BUTTON_A = (2 << IT_CONTROLLER_ID_SHIFT) + AllInputs::BUTTON_A,
    IT_CONTROLLER2_BUTTON_B = (2 << IT_CONTROLLER_ID_SHIFT) + AllInputs::BUTTON_B,
    IT_CONTROLLER2_BUTTON_X = (2 << IT_CONTROLLER_ID_SHIFT) + AllInputs::BUTTON_X,
    IT_CONTROLLER2_BUTTON_Y = (2 << IT_CONTROLLER_ID_SHIFT) + AllInputs::BUTTON_Y,
    IT_CONTROLLER2_BUTTON_BACK = (2 << IT_CONTROLLER_ID_SHIFT) + AllInputs::BUTTON_BACK,
    IT_CONTROLLER2_BUTTON_GUIDE = (2 << IT_CONTROLLER_ID_SHIFT) + AllInputs::BUTTON_GUIDE,
    IT_CONTROLLER2_BUTTON_START = (2 << IT_CONTROLLER_ID_SHIFT) + AllInputs::BUTTON_START,
    IT_CONTROLLER2_BUTTON_LEFTSTICK = (2 << IT_CONTROLLER_ID_SHIFT) + AllInputs::BUTTON_LEFTSTICK,
    IT_CONTROLLER2_BUTTON_RIGHTSTICK = (2 << IT_CONTROLLER_ID_SHIFT) + AllInputs::BUTTON_RIGHTSTICK,
    IT_CONTROLLER2_BUTTON_LEFTSHOULDER = (2 << IT_CONTROLLER_ID_SHIFT) + AllInputs::BUTTON_LEFTSHOULDER,
    IT_CONTROLLER2_BUTTON_RIGHTSHOULDER = (2 << IT_CONTROLLER_ID_SHIFT) + AllInputs::BUTTON_RIGHTSHOULDER,
    IT_CONTROLLER2_BUTTON_DPAD_UP = (2 << IT_CONTROLLER_ID_SHIFT) + AllInputs::BUTTON_DPAD_UP,
    IT_CONTROLLER2_BUTTON_DPAD_DOWN = (2 << IT_CONTROLLER_ID_SHIFT) + AllInputs::BUTTON_DPAD_DOWN,
    IT_CONTROLLER2_BUTTON_DPAD_LEFT = (2 << IT_CONTROLLER_ID_SHIFT) + AllInputs::BUTTON_DPAD_LEFT,
    IT_CONTROLLER2_BUTTON_DPAD_RIGHT = (2 << IT_CONTROLLER_ID_SHIFT) + AllInputs::BUTTON_DPAD_RIGHT,
    IT_CONTROLLER2_AXIS_LEFTX = ((2 << IT_CONTROLLER_ID_SHIFT) | IT_CONTROLLER_AXIS_MASK) + AllInputs::AXIS_LEFTX,
    IT_CONTROLLER2_AXIS_LEFTY = ((2 << IT_CONTROLLER_ID_SHIFT) | IT_CONTROLLER_AXIS_MASK) + AllInputs::AXIS_LEFTY,
    IT_CONTROLLER2_AXIS_RIGHTX = ((2 << IT_CONTROLLER_ID_SHIFT) | IT_CONTROLLER_AXIS_MASK) + AllInputs::AXIS_RIGHTX,
    IT_CONTROLLER2_AXIS_RIGHTY = ((2 << IT_CONTROLLER_ID_SHIFT) | IT_CONTROLLER_AXIS_MASK) + AllInputs::AXIS_RIGHTY,
    IT_CONTROLLER2_AXIS_TRIGGERLEFT = ((2 << IT_CONTROLLER_ID_SHIFT) | IT_CONTROLLER_AXIS_MASK) + AllInputs::AXIS_TRIGGERLEFT,
    IT_CONTROLLER2_AXIS_TRIGGERRIGHT = ((2 << IT_CONTROLLER_ID_SHIFT) | IT_CONTROLLER_AXIS_MASK) + AllInputs::AXIS_TRIGGERRIGHT,

    /* Controller 3 */
    IT_CONTROLLER3_BUTTON_A = (3 << IT_CONTROLLER_ID_SHIFT) + AllInputs::BUTTON_A,
    IT_CONTROLLER3_BUTTON_B = (3 << IT_CONTROLLER_ID_SHIFT) + AllInputs::BUTTON_B,
    IT_CONTROLLER3_BUTTON_X = (3 << IT_CONTROLLER_ID_SHIFT) + AllInputs::BUTTON_X,
    IT_CONTROLLER3_BUTTON_Y = (3 << IT_CONTROLLER_ID_SHIFT) + AllInputs::BUTTON_Y,
    IT_CONTROLLER3_BUTTON_BACK = (3 << IT_CONTROLLER_ID_SHIFT) + AllInputs::BUTTON_BACK,
    IT_CONTROLLER3_BUTTON_GUIDE = (3 << IT_CONTROLLER_ID_SHIFT) + AllInputs::BUTTON_GUIDE,
    IT_CONTROLLER3_BUTTON_START = (3 << IT_CONTROLLER_ID_SHIFT) + AllInputs::BUTTON_START,
    IT_CONTROLLER3_BUTTON_LEFTSTICK = (3 << IT_CONTROLLER_ID_SHIFT) + AllInputs::BUTTON_LEFTSTICK,
    IT_CONTROLLER3_BUTTON_RIGHTSTICK = (3 << IT_CONTROLLER_ID_SHIFT) + AllInputs::BUTTON_RIGHTSTICK,
    IT_CONTROLLER3_BUTTON_LEFTSHOULDER = (3 << IT_CONTROLLER_ID_SHIFT) + AllInputs::BUTTON_LEFTSHOULDER,
    IT_CONTROLLER3_BUTTON_RIGHTSHOULDER = (3 << IT_CONTROLLER_ID_SHIFT) + AllInputs::BUTTON_RIGHTSHOULDER,
    IT_CONTROLLER3_BUTTON_DPAD_UP = (3 << IT_CONTROLLER_ID_SHIFT) + AllInputs::BUTTON_DPAD_UP,
    IT_CONTROLLER3_BUTTON_DPAD_DOWN = (3 << IT_CONTROLLER_ID_SHIFT) + AllInputs::BUTTON_DPAD_DOWN,
    IT_CONTROLLER3_BUTTON_DPAD_LEFT = (3 << IT_CONTROLLER_ID_SHIFT) + AllInputs::BUTTON_DPAD_LEFT,
    IT_CONTROLLER3_BUTTON_DPAD_RIGHT = (3 << IT_CONTROLLER_ID_SHIFT) + AllInputs::BUTTON_DPAD_RIGHT,
    IT_CONTROLLER3_AXIS_LEFTX = ((3 << IT_CONTROLLER_ID_SHIFT) | IT_CONTROLLER_AXIS_MASK) + AllInputs::AXIS_LEFTX,
    IT_CONTROLLER3_AXIS_LEFTY = ((3 << IT_CONTROLLER_ID_SHIFT) | IT_CONTROLLER_AXIS_MASK) + AllInputs::AXIS_LEFTY,
    IT_CONTROLLER3_AXIS_RIGHTX = ((3 << IT_CONTROLLER_ID_SHIFT) | IT_CONTROLLER_AXIS_MASK) + AllInputs::AXIS_RIGHTX,
    IT_CONTROLLER3_AXIS_RIGHTY = ((3 << IT_CONTROLLER_ID_SHIFT) | IT_CONTROLLER_AXIS_MASK) + AllInputs::AXIS_RIGHTY,
    IT_CONTROLLER3_AXIS_TRIGGERLEFT = ((3 << IT_CONTROLLER_ID_SHIFT) | IT_CONTROLLER_AXIS_MASK) + AllInputs::AXIS_TRIGGERLEFT,
    IT_CONTROLLER3_AXIS_TRIGGERRIGHT = ((3 << IT_CONTROLLER_ID_SHIFT) | IT_CONTROLLER_AXIS_MASK) + AllInputs::AXIS_TRIGGERRIGHT,

    /* Controller 4 */
    IT_CONTROLLER4_BUTTON_A = (4 << IT_CONTROLLER_ID_SHIFT) + AllInputs::BUTTON_A,
    IT_CONTROLLER4_BUTTON_B = (4 << IT_CONTROLLER_ID_SHIFT) + AllInputs::BUTTON_B,
    IT_CONTROLLER4_BUTTON_X = (4 << IT_CONTROLLER_ID_SHIFT) + AllInputs::BUTTON_X,
    IT_CONTROLLER4_BUTTON_Y = (4 << IT_CONTROLLER_ID_SHIFT) + AllInputs::BUTTON_Y,
    IT_CONTROLLER4_BUTTON_BACK = (4 << IT_CONTROLLER_ID_SHIFT) + AllInputs::BUTTON_BACK,
    IT_CONTROLLER4_BUTTON_GUIDE = (4 << IT_CONTROLLER_ID_SHIFT) + AllInputs::BUTTON_GUIDE,
    IT_CONTROLLER4_BUTTON_START = (4 << IT_CONTROLLER_ID_SHIFT) + AllInputs::BUTTON_START,
    IT_CONTROLLER4_BUTTON_LEFTSTICK = (4 << IT_CONTROLLER_ID_SHIFT) + AllInputs::BUTTON_LEFTSTICK,
    IT_CONTROLLER4_BUTTON_RIGHTSTICK = (4 << IT_CONTROLLER_ID_SHIFT) + AllInputs::BUTTON_RIGHTSTICK,
    IT_CONTROLLER4_BUTTON_LEFTSHOULDER = (4 << IT_CONTROLLER_ID_SHIFT) + AllInputs::BUTTON_LEFTSHOULDER,
    IT_CONTROLLER4_BUTTON_RIGHTSHOULDER = (4 << IT_CONTROLLER_ID_SHIFT) + AllInputs::BUTTON_RIGHTSHOULDER,
    IT_CONTROLLER4_BUTTON_DPAD_UP = (4 << IT_CONTROLLER_ID_SHIFT) + AllInputs::BUTTON_DPAD_UP,
    IT_CONTROLLER4_BUTTON_DPAD_DOWN = (4 << IT_CONTROLLER_ID_SHIFT) + AllInputs::BUTTON_DPAD_DOWN,
    IT_CONTROLLER4_BUTTON_DPAD_LEFT = (4 << IT_CONTROLLER_ID_SHIFT) + AllInputs::BUTTON_DPAD_LEFT,
    IT_CONTROLLER4_BUTTON_DPAD_RIGHT = (4 << IT_CONTROLLER_ID_SHIFT) + AllInputs::BUTTON_DPAD_RIGHT,
    IT_CONTROLLER4_AXIS_LEFTX = ((4 << IT_CONTROLLER_ID_SHIFT) | IT_CONTROLLER_AXIS_MASK) + AllInputs::AXIS_LEFTX,
    IT_CONTROLLER4_AXIS_LEFTY = ((4 << IT_CONTROLLER_ID_SHIFT) | IT_CONTROLLER_AXIS_MASK) + AllInputs::AXIS_LEFTY,
    IT_CONTROLLER4_AXIS_RIGHTX = ((4 << IT_CONTROLLER_ID_SHIFT) | IT_CONTROLLER_AXIS_MASK) + AllInputs::AXIS_RIGHTX,
    IT_CONTROLLER4_AXIS_RIGHTY = ((4 << IT_CONTROLLER_ID_SHIFT) | IT_CONTROLLER_AXIS_MASK) + AllInputs::AXIS_RIGHTY,
    IT_CONTROLLER4_AXIS_TRIGGERLEFT = ((4 << IT_CONTROLLER_ID_SHIFT) | IT_CONTROLLER_AXIS_MASK) + AllInputs::AXIS_TRIGGERLEFT,
    IT_CONTROLLER4_AXIS_TRIGGERRIGHT = ((4 << IT_CONTROLLER_ID_SHIFT) | IT_CONTROLLER_AXIS_MASK) + AllInputs::AXIS_TRIGGERRIGHT,
};

class SingleInput {
public:
    InputType type;
    unsigned int value;
    std::string description;

    bool operator==( const SingleInput &si ) const {
        return (type == si.type) && (value == si.value);
    }

    bool operator<( const SingleInput &si ) const {
        return ((type < si.type) || ((type == si.type) && (value < si.value)));
    }

    /* Pack the content of the struct into the data array */
    void pack(char* data);

    /* Unpack data the content of the struct into the data array */
    void unpack(const char* data);
};

typedef int HotKeyType; enum
{
    HOTKEY_PLAYPAUSE, // Switch from play to pause of the game
    HOTKEY_FRAMEADVANCE, // Advance one frame, also pause the game if playing
    HOTKEY_FASTFORWARD, // Enable fastforward when pressed
    HOTKEY_READWRITE, // Switch from read-only recording to write
    HOTKEY_SAVESTATE1, // Save the entire state of the game
    HOTKEY_SAVESTATE2,
    HOTKEY_SAVESTATE3,
    HOTKEY_SAVESTATE4,
    HOTKEY_SAVESTATE5,
    HOTKEY_SAVESTATE6,
    HOTKEY_SAVESTATE7,
    HOTKEY_SAVESTATE8,
    HOTKEY_SAVESTATE9,
    HOTKEY_LOADSTATE1, // Load the entire state of the game
    HOTKEY_LOADSTATE2,
    HOTKEY_LOADSTATE3,
    HOTKEY_LOADSTATE4,
    HOTKEY_LOADSTATE5,
    HOTKEY_LOADSTATE6,
    HOTKEY_LOADSTATE7,
    HOTKEY_LOADSTATE8,
    HOTKEY_LOADSTATE9,
    HOTKEY_TOGGLE_ENCODE, // Start/stop audio/video encoding
    HOTKEY_LEN
};

struct HotKey {
    SingleInput default_input;
    HotKeyType type;
    std::string description;

    bool operator==( const HotKey &hk ) const {
        return type == hk.type;
    }

    bool operator<( const HotKey &hk ) const {
        return type < hk.type;
    }

    /* Pack the content of the struct into the data array */
    void pack(char* data);

    /* Unpack data the content of the struct into the data array */
    void unpack(const char* data);
};

struct ModifierKey {
    xcb_keysym_t ks;
    xcb_keysym_t flag;
    std::string description;
};

enum ModifierFlag {
    XK_Shift_L_Flag = 0x010000,
    XK_Shift_R_Flag = 0x020000,
    XK_Control_L_Flag = 0x040000,
    XK_Control_R_Flag = 0x080000,
    XK_Meta_L_Flag = 0x100000,
    XK_Meta_R_Flag = 0x200000,
    XK_Alt_L_Flag = 0x400000,
    XK_Alt_R_Flag = 0x800000
};

static std::array<ModifierKey, 8> modifier_list {{
    {XK_Shift_L, XK_Shift_L_Flag, "Shift_L"},
    {XK_Shift_R, XK_Shift_R_Flag, "Shift_R"},
    {XK_Control_L, XK_Control_L_Flag, "Control_L"},
    {XK_Control_R, XK_Control_R_Flag, "Control_R"},
    {XK_Meta_L, XK_Meta_L_Flag, "Meta_L"},
    {XK_Meta_R, XK_Meta_R_Flag, "Meta_R"},
    {XK_Alt_L, XK_Alt_L_Flag, "Alt_L"},
    {XK_Alt_R, XK_Alt_R_Flag, "Alt_R"},
}};

bool is_modifier(xcb_keysym_t ks);

xcb_keysym_t build_modifiers(unsigned char keyboard_state[], xcb_connection_t *conn);

class KeyMapping {
    public:
        /* Initialize hotkeys and mapping list */
        void init(xcb_connection_t* conn);

        /* Map keyboard KeySym to a single input of a keyboard or controller */
        std::map<xcb_keysym_t,SingleInput> input_mapping;

        /* List of inputs that can be mapped to a single key */
        std::vector<SingleInput> input_list;

        /* Map keyboard KeySym to a hotkey */
        std::map<xcb_keysym_t,HotKey> hotkey_mapping;

        /* List of hotkeys */
        std::vector<HotKey> hotkey_list;

        /* Set hotkeys to default values */
        void default_hotkeys();

        /* Set hotkeys to default values */
        void default_inputs();

        /* Set a hotkey to default value */
        void default_hotkey(int hotkey_index);

        /* Set input to default value */
        void default_input(int input_index);

        /* Assign a new key to the hotkey */
        void reassign_hotkey(int hotkey_index, KeySym ks);

        /* Assign a new key to the input */
        void reassign_input(int input_index, KeySym ks);

        /*
         * We are building the whole AllInputs structure,
         * that will be passed to the game and saved.
         * We will be doing the following steps:
         * - Get the raw keyboard state using XQueryKeymap
         * - Convert keyboard keycodes (physical keys) to keysyms (key meaning)
         * - Check if the keysym is mapped to a hotkey. If so, we skip it
         * - Check if the key is mapped to another input and fill the AllInputs struct accordingly
         * - Get the mouse state
         */
        void buildAllInputs(AllInputs& ai, xcb_connection_t *conn, xcb_window_t window, SharedConfig& sc);
};

#endif // KEYMAPPING_H_INCLUDED
