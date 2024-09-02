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

#include "xkbcommon.h"
#include "xkeyboardlayout.h"

#include "logging.h"
#include "GlobalState.h"

namespace libtas {

DEFINE_ORIG_POINTER(xkb_state_mod_name_is_active)

/* TODO: Duplicate in xkeyboardlayout.cpp */
static const char* Xlib_default_char = "\0\0\0\0\0\0\0\0\0\0001234567890-=\010\tqwertyuiop[]\r\0asdfghjkl;'`\0\\zxcvbnm,./\0*\0 \0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0-\0\0\0+\0\0\0\0\0\0\0<\0\0\0\0\0\0\0\0\0\r\0/\0\0\n\0\0\0\0\0\0\0\0\0\177\0\0\0\0\0=+\0\0.";
static const char* Xlib_default_char_shifted = "\0\0\0\0\0\0\0\0\0\0!@#$%^&*()_+\010\0QWERTYUIOP{}\0\0ASDFGHJKL:\"~\0|ZXCVBNM<>?\0*\0 \0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0-\0\0\0+\0\0\0\0\0\0\0>\0\0\0\0\0\0\0\0\0\r\0/\0\0\n\0\0\0\0\0\0\0\0\0\177\0\0\0\0\0=+\0\0.";

/* Override */ int xkb_state_key_get_utf8(struct xkb_state *state, xkb_keycode_t key, char *buffer, size_t size)
{
    LOGTRACE(LCF_KEYBOARD);
    if (buffer && (size > 0)) {
        char c = '\0';
        if (key < 130) {
            LINK_NAMESPACE_FULLNAME(xkb_state_mod_name_is_active, "libxkbcommon.so.0");
            int shift_pressed = orig::xkb_state_mod_name_is_active(state, XKB_MOD_NAME_SHIFT, XKB_STATE_MODS_EFFECTIVE);

            if (shift_pressed)
                c = Xlib_default_char_shifted[key];
            else
                c = Xlib_default_char[key];            
        }

        if (c == '\0') {
            return 0;
        }
        buffer[0] = c;
        return 1;
    }
    return 0;
}

/* Override */ xkb_keysym_t xkb_state_key_get_one_sym(struct xkb_state *state, xkb_keycode_t key)
{
    LOGTRACE(LCF_KEYBOARD);
    GlobalNoLog gnl; // Avoid logging on XkbKeycodeToKeysym

    /* Get shift modifier */
    LINK_NAMESPACE_FULLNAME(xkb_state_mod_name_is_active, "libxkbcommon.so.0");
    int shift_pressed = orig::xkb_state_mod_name_is_active(state, XKB_MOD_NAME_SHIFT, XKB_STATE_MODS_EFFECTIVE);

    return XkbKeycodeToKeysym(nullptr, key, 0, shift_pressed);
}

}
