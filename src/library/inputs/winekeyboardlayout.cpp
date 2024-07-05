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

#include "winekeyboardlayout.h"

#include "logging.h"

#include <X11/keysym.h>
#include <X11/XF86keysym.h>

namespace libtas {

static const KeySym wine_default_keymap[256] = {
    NoSymbol, NoSymbol, NoSymbol, NoSymbol, NoSymbol, NoSymbol, NoSymbol, NoSymbol,
    XK_BackSpace, XK_Tab, NoSymbol, NoSymbol, NoSymbol /*VK_CLEAR*/, XK_Return, NoSymbol, NoSymbol,
    XK_Shift_L, XK_Control_L, XK_Menu, XK_Pause, NoSymbol /*VK_CAPITAL*/, XK_Hiragana_Katakana, NoSymbol, NoSymbol,
    NoSymbol, XK_Hangul_Hanja, NoSymbol, XK_Escape, NoSymbol, NoSymbol, NoSymbol, NoSymbol,
    XK_space, XK_Prior, XK_Next, XK_End, XK_Home, XK_Left, XK_Up, XK_Right,
    XK_Down, NoSymbol /*VK_SELECT*/, XK_Print, NoSymbol /*VK_EXECUTE*/, NoSymbol /*VK_SNAPSHOT*/, XK_Insert, XK_Delete, XK_Help,
    XK_0, XK_1, XK_2, XK_3, XK_4, XK_5, XK_6, XK_7,
    XK_8, XK_9, NoSymbol, NoSymbol, NoSymbol, NoSymbol, NoSymbol, NoSymbol,
    NoSymbol, XK_a, XK_b, XK_c, XK_d, XK_e, XK_f, XK_g,
    XK_h, XK_i, XK_j, XK_k, XK_l, XK_m, XK_n, XK_o,
    XK_p, XK_q, XK_r, XK_s, XK_t, XK_u, XK_v, XK_w,
    XK_x, XK_y, XK_z, XK_Super_L, XK_Super_R, NoSymbol /*VK_APPS*/, NoSymbol, XF86XK_Sleep,
    XK_0, XK_1, XK_2, XK_3, XK_4, XK_5, XK_6, XK_7,
    XK_8, XK_9, XK_KP_Multiply, XK_KP_Add, XK_Linefeed, XK_KP_Subtract, XK_KP_Decimal, XK_KP_Divide,
    XK_F1, XK_F2, XK_F3, XK_F4, XK_F5, XK_F6, XK_F7, XK_F8,
    XK_F9, XK_F10, XK_F11, XK_F12, NoSymbol, NoSymbol, NoSymbol, NoSymbol,
    NoSymbol, NoSymbol, NoSymbol, NoSymbol, NoSymbol, NoSymbol, NoSymbol, NoSymbol,
    NoSymbol, NoSymbol, NoSymbol, NoSymbol, NoSymbol, NoSymbol, NoSymbol, NoSymbol,
    XK_Num_Lock, XK_Scroll_Lock, NoSymbol, NoSymbol, NoSymbol, NoSymbol, NoSymbol, NoSymbol,
    NoSymbol, NoSymbol, NoSymbol, NoSymbol, NoSymbol, NoSymbol, NoSymbol, NoSymbol,
    XK_Shift_L, XK_Shift_R, XK_Control_L, XK_Control_R, XK_Menu, XK_Menu, XF86XK_Back, XF86XK_Forward,
    XF86XK_Reload, XF86XK_Close, XF86XK_Search, XF86XK_Favorites, XF86XK_HomePage, XF86XK_AudioMute, XF86XK_AudioLowerVolume, XF86XK_AudioRaiseVolume,
    XF86XK_AudioNext, XF86XK_AudioPrev, XF86XK_AudioStop, XF86XK_AudioPlay, XF86XK_Mail, XF86XK_AudioMedia, XF86XK_Launch1, XF86XK_Launch2,
    NoSymbol, NoSymbol, XK_semicolon, XK_plusminus, XK_comma, XK_minus, XK_period, XK_slash,
    XK_grave, NoSymbol, NoSymbol, NoSymbol, NoSymbol, NoSymbol, NoSymbol, NoSymbol,
    NoSymbol, NoSymbol, NoSymbol, NoSymbol, NoSymbol, NoSymbol, NoSymbol, NoSymbol,
    NoSymbol, NoSymbol, NoSymbol, NoSymbol, NoSymbol, NoSymbol, NoSymbol, NoSymbol,
    NoSymbol, NoSymbol, NoSymbol, XK_bracketleft, XK_backslash, XK_bracketright, XK_apostrophe, NoSymbol,
    NoSymbol, NoSymbol, NoSymbol, NoSymbol, NoSymbol, NoSymbol, NoSymbol, NoSymbol,
    NoSymbol, NoSymbol, NoSymbol, NoSymbol, NoSymbol, NoSymbol, NoSymbol, NoSymbol,
    NoSymbol, NoSymbol, NoSymbol, NoSymbol, NoSymbol, NoSymbol, NoSymbol, NoSymbol,
    NoSymbol, NoSymbol, NoSymbol, NoSymbol, NoSymbol, NoSymbol, NoSymbol, NoSymbol,
};

KeySym VKeyToXKeysym(int vkey)
{
    LOG(LL_DEBUG, LCF_KEYBOARD, "%s called with vkey %d", __func__, vkey);
    KeySym sym = wine_default_keymap[vkey];
    LOG(LL_DEBUG, LCF_KEYBOARD, "   returning %d", sym);
    return sym;
}

int XKeysymToVKey(KeySym keysym)
{
    LOG(LL_DEBUG, LCF_KEYBOARD, "%s called with keysym %d", __func__, keysym);
    int vkey = 0;
    /* Translate from uppercase letter to lowercase */
    if ((keysym >= XK_A) && (keysym <= XK_Z)) {
        keysym += (XK_a - XK_A);
    }

    for (int i=0; i<256; i++) {
        if (wine_default_keymap[i] == keysym) {
            vkey = i;
            break;
        }
    }

    LOG(LL_DEBUG, LCF_KEYBOARD, "   returning %d", vkey);
    return vkey;
}

}
