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

#ifndef LIBTAS_KEYMAPPING_XCB_H_INCLUDED
#define LIBTAS_KEYMAPPING_XCB_H_INCLUDED

#include "../shared/AllInputs.h"
#include "../shared/SharedConfig.h"
#include "KeyMapping.h"
#include <string>
#include <xcb/xcb.h>
#include <xcb/xcb_keysyms.h>

class KeyMappingXcb : public KeyMapping {
    public:
        /* Initialize hotkeys and mapping list */
        KeyMappingXcb(void* conn);

        /* Returns if a keysym is a modifier */
        bool is_modifier(keysym_t ks);

        /* Returns the list of modifiers compated into a single int from the event state */
        keysym_t get_modifiers(int state);

        /* Get the input description with modifiers */
        std::string input_description_mod(keysym_t ks);

        /* Set hotkeys to default values */
        void default_inputs();

        /* Set input to default value */
        void default_input(int tab, int input_index);

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
        void buildAllInputs(AllInputs& ai, uint32_t window, SharedConfig& sc, bool mouse_warp);

    private:
        /* Connection to the X11 server */
        xcb_connection_t *conn;

        /* Connection to the keyboard layout */
        xcb_key_symbols_t *keysyms;

        /* Returns the list of modifiers from the keyboard state */
        keysym_t build_modifiers(unsigned char keyboard_state[]);

};

#endif // LIBTAS_KEYMAPPING_XCB_H_INCLUDED
