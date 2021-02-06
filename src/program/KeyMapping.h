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

#ifndef LIBTAS_KEYMAPPING_H_INCLUDED
#define LIBTAS_KEYMAPPING_H_INCLUDED

#include <QtCore/QDataStream>
#include "../shared/SingleInput.h"
#include "../shared/AllInputs.h"
#include "../shared/SharedConfig.h"
#include <xcb/xcb.h>
#include <xcb/xcb_keysyms.h>
#include <X11/keysym.h>
#include <map>
#include <vector>
#include <array>
#include <string>

Q_DECLARE_METATYPE(SingleInput)

/* Save the content of the struct into the stream */
QDataStream &operator<<(QDataStream &out, const SingleInput &obj);

/* Load the content of the struct from the stream */
QDataStream &operator>>(QDataStream &in, SingleInput &obj);

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
    HOTKEY_SAVESTATE_BACKTRACK,
    HOTKEY_LOADSTATE1, // Load the entire state of the game
    HOTKEY_LOADSTATE2,
    HOTKEY_LOADSTATE3,
    HOTKEY_LOADSTATE4,
    HOTKEY_LOADSTATE5,
    HOTKEY_LOADSTATE6,
    HOTKEY_LOADSTATE7,
    HOTKEY_LOADSTATE8,
    HOTKEY_LOADSTATE9,
    HOTKEY_LOADSTATE_BACKTRACK,
    HOTKEY_TOGGLE_ENCODE, // Start/stop audio/video encoding
    HOTKEY_CALIBRATE_MOUSE, // Calibrate mouse cursor position
    HOTKEY_LOADBRANCH1, // Load state and movie
    HOTKEY_LOADBRANCH2,
    HOTKEY_LOADBRANCH3,
    HOTKEY_LOADBRANCH4,
    HOTKEY_LOADBRANCH5,
    HOTKEY_LOADBRANCH6,
    HOTKEY_LOADBRANCH7,
    HOTKEY_LOADBRANCH8,
    HOTKEY_LOADBRANCH9,
    HOTKEY_LOADBRANCH_BACKTRACK,
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
}; Q_DECLARE_METATYPE(HotKey)

/* Save the content of the struct into the stream */
QDataStream &operator<<(QDataStream &out, const HotKey &obj);

/* Load the content of the struct from the stream */
QDataStream &operator>>(QDataStream &in, HotKey &obj);

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

xcb_keysym_t build_modifiers(unsigned char keyboard_state[], xcb_key_symbols_t *keysyms);

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

        /* Get the input description */
        std::string input_description(xcb_keysym_t ks);

        /* Set hotkeys to default values */
        void default_hotkeys();

        /* Set hotkeys to default values */
        void default_inputs();

        /* Set a hotkey to default value */
        void default_hotkey(int hotkey_index);

        /* Set input to default value */
        void default_input(int input_index);

        /* Assign a new key to the hotkey */
        void reassign_hotkey(int hotkey_index, xcb_keysym_t ks);
        void reassign_hotkey(HotKey hk, xcb_keysym_t ks);

        /* Assign a new key to the input */
        void reassign_input(int input_index, xcb_keysym_t ks);
        void reassign_input(SingleInput si, xcb_keysym_t ks);

        /*
         * We are building the whole AllInputs structure,
         * that will be passed to the game and saved.
         * We will be doing the following steps:
         * - Get the raw keyboard state using XQueryKeymap
         * - Convert keyboard keycodes (physical keys) to keysyms (key meaning)
         * - Check if the keysym is mapped to a hotkey. If so, we skip it
         * - Check if the key is mapped to another input and fill the AllInputs struct accordingly
         * - Get the mouse state
         * - Warp mouse pointer if needed
         */
        void buildAllInputs(AllInputs& ai, xcb_window_t window, xcb_key_symbols_t *keysyms, SharedConfig& sc, bool mouse_warp);

    private:
        /* Connection to the X11 server */
        xcb_connection_t *conn;

        /* Connection to the keyboard layout */
        xcb_key_symbols_t *keysyms;

};

#endif // LIBTAS_KEYMAPPING_H_INCLUDED
