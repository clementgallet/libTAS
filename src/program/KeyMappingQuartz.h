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

#ifndef LIBTAS_KEYMAPPING_QUARTZ_H_INCLUDED
#define LIBTAS_KEYMAPPING_QUARTZ_H_INCLUDED

#include "../shared/AllInputs.h"
#include "../shared/SharedConfig.h"
#include "KeyMapping.h"
#include <string>

class KeyMappingQuartz : public KeyMapping {
    public:
        /* Initialize hotkeys and mapping list */
        KeyMappingQuartz(void* conn);

        /* Register a keydown event */
        void registerKeyDown(keysym_t ks);

        /* Register a keyup event */
        void registerKeyUp(keysym_t ks);

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
        /* Up-to-date keyboard state */
        uint8_t keyboard_state[16];

        /* Returns the list of currently pressed modifiers */
        keysym_t build_modifiers();

};

#endif // LIBTAS_KEYMAPPING_QUARTZ_H_INCLUDED
