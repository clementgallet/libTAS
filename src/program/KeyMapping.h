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
#include <map>
#include <vector>
#include <array>
#include <string>

Q_DECLARE_METATYPE(SingleInput)

/* Save the content of the struct into the stream */
QDataStream &operator<<(QDataStream &out, const SingleInput &obj);

/* Load the content of the struct from the stream */
QDataStream &operator>>(QDataStream &in, SingleInput &obj);

/* NOTE: Only append more hotkeys at the end, otherwise it breaks the config
 * hotkey mappings. */
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
    HOTKEY_TOGGLE_FASTFORWARD, // Toggle fastforward
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

/* Generic keysym type */
typedef uint32_t keysym_t;

/* Flag for modifiers. We must expose this because GUI needs to build modifiers */
enum ModifierFlag {
    XK_Shift_Flag = 0x010000,
    XK_Control_Flag = 0x040000,
    XK_Meta_Flag = 0x100000,
    XK_Alt_Flag = 0x400000,
};


class KeyMapping {
    public:
        /* Initialize hotkeys and mapping list */
        KeyMapping(void* conn);

        /* Register a keydown event */
        virtual void registerKeyDown(uint16_t kc) {}

        /* Register a keyup event */
        virtual void registerKeyUp(uint16_t kc) {}

        /* Convert a native keycode into xcb keysym */
        virtual keysym_t nativeToKeysym(int keycode) {return keycode;}

        /* Map keyboard KeySym to a single input of a keyboard or controller */
        std::map<keysym_t,SingleInput> input_mapping;

        enum {
            INPUTLIST_KEYBOARD_LATIN,
            INPUTLIST_KEYBOARD_MISC,
            INPUTLIST_FLAG,
            INPUTLIST_MOUSE,
            INPUTLIST_CONTROLLER,
            INPUTLIST_HIDDEN,
            INPUTLIST_SIZE,
        };

        /* List of inputs that can be mapped to a single key */
        std::vector<SingleInput> input_list[INPUTLIST_SIZE];

        /* Map keyboard KeySym to a hotkey */
        std::map<keysym_t,HotKey> hotkey_mapping;

        /* List of hotkeys */
        std::vector<HotKey> hotkey_list;

        /* Get the input description */
        std::string input_description(keysym_t ks);

        /* Returns if a keysym is a modifier */
        virtual bool is_modifier(keysym_t ks) = 0;

        /* Returns the list of modifiers compated into a single int from the event state */
        virtual keysym_t get_modifiers(int state) = 0;

        /* Get the input description with modifiers */
        virtual std::string input_description_mod(keysym_t ks) = 0;

        /* Set hotkeys to default values */
        void default_hotkeys();

        /* Set hotkeys to default values */
        virtual void default_inputs() = 0;

        /* Set a hotkey to default value */
        void default_hotkey(int hotkey_index);

        /* Set input to default value */
        virtual void default_input(int tab, int input_index) = 0;

        /* Assign a new key to the hotkey */
        void reassign_hotkey(int hotkey_index, keysym_t ks);
        void reassign_hotkey(HotKey hk, keysym_t ks);

        /* Assign a new key to the input */
        void reassign_input(int tab, int input_index, keysym_t ks);
        void reassign_input(SingleInput si, keysym_t ks);

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
        virtual void buildAllInputs(AllInputs& ai, uint32_t window, SharedConfig& sc, bool mouse_warp) = 0;
};

#endif // LIBTAS_KEYMAPPING_H_INCLUDED
