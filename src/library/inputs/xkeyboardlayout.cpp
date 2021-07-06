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

#include "xkeyboardlayout.h"

#include "../logging.h"
#include "../backtrace.h"

#include <X11/keysym.h>
#include <X11/XF86keysym.h>
// #include <X11/X.h>


namespace libtas {

static const KeySym Xlib_default_keymap[256] = {
    NoSymbol, NoSymbol, NoSymbol, NoSymbol, NoSymbol,
    NoSymbol, NoSymbol, NoSymbol, NoSymbol, XK_Escape,
    XK_1, XK_2, XK_3, XK_4, XK_5,
    XK_6, XK_7, XK_8, XK_9, XK_0,
    XK_minus, XK_equal, XK_BackSpace, XK_Tab, XK_q,
    XK_w, XK_e, XK_r, XK_t, XK_y,
    XK_u, XK_i, XK_o, XK_p, XK_bracketleft,
    XK_bracketright, XK_Return, XK_Control_L, XK_a, XK_s,
    XK_d, XK_f, XK_g, XK_h, XK_j,
    XK_k, XK_l, XK_semicolon, XK_apostrophe, XK_grave,
    XK_Shift_L, XK_backslash, XK_z, XK_x, XK_c,
    XK_v, XK_b, XK_n, XK_m, XK_comma,
    XK_period, XK_slash, XK_Shift_R, XK_KP_Multiply, XK_Alt_L,
    XK_space, XK_Caps_Lock, XK_F1, XK_F2, XK_F3,
    XK_F4, XK_F5, XK_F6, XK_F7, XK_F8,
    XK_F9, XK_F10, XK_Num_Lock, XK_Scroll_Lock, XK_KP_Home,
    XK_KP_Up, XK_KP_Prior, XK_KP_Subtract, XK_KP_Left, XK_KP_Begin,
    XK_KP_Right, XK_KP_Add, XK_KP_End, XK_KP_Down, XK_KP_Next,
    XK_KP_Insert, XK_KP_Delete, XK_ISO_Level3_Shift, NoSymbol, XK_less,
    XK_F11, XK_F12, NoSymbol, XK_Katakana, XK_Hiragana,
    XK_Henkan_Mode, XK_Hiragana_Katakana, XK_Muhenkan, NoSymbol, XK_KP_Enter,
    XK_Control_R, XK_KP_Divide, XK_Print, XK_Alt_R, XK_Linefeed,
    XK_Home, XK_Up, XK_Prior, XK_Left, XK_Right,
    XK_End, XK_Down, XK_Next, XK_Insert, XK_Delete,
    NoSymbol, XF86XK_AudioMute, XF86XK_AudioLowerVolume, XF86XK_AudioRaiseVolume, XF86XK_PowerOff,
    XK_KP_Equal, XK_plusminus, XK_Pause, XF86XK_LaunchA, XK_KP_Decimal,
    XK_Hangul, XK_Hangul_Hanja, NoSymbol, XK_Super_L, XK_Super_R,
    XK_Menu, XK_Cancel, XK_Redo, NoSymbol, XK_Undo,
    NoSymbol, XF86XK_Copy, XF86XK_Open, XF86XK_Paste, XK_Find,
    XF86XK_Cut, XK_Help, XF86XK_MenuKB, XF86XK_Calculator, NoSymbol,
    XF86XK_Sleep, XF86XK_WakeUp, XF86XK_Explorer, XF86XK_Send, NoSymbol,
    XF86XK_Xfer, XF86XK_Launch1, XF86XK_Launch2, XF86XK_WWW, XF86XK_DOS,
    XF86XK_ScreenSaver, XF86XK_RotateWindows, XF86XK_TaskPane, XF86XK_Mail, XF86XK_Favorites,
    XF86XK_MyComputer, XF86XK_Back, XF86XK_Forward, NoSymbol, XF86XK_Eject,
    XF86XK_Eject, XF86XK_AudioNext, XF86XK_AudioPlay, XF86XK_AudioPrev, XF86XK_AudioStop,
    XF86XK_AudioRecord, XF86XK_AudioRewind, XF86XK_Phone, NoSymbol, XF86XK_Tools,
    XF86XK_HomePage, XF86XK_Reload, XF86XK_Close, NoSymbol, NoSymbol,
    XF86XK_ScrollUp, XF86XK_ScrollDown, XK_parenleft, XK_parenright, XF86XK_New,
    XK_Redo, XF86XK_Tools, XF86XK_Launch5, XF86XK_Launch6, XF86XK_Launch7,
    XF86XK_Launch8, XF86XK_Launch9, NoSymbol, XF86XK_AudioMicMute, XF86XK_TouchpadToggle,
    XF86XK_TouchpadOn, XF86XK_TouchpadOff, NoSymbol, XK_Mode_switch, NoSymbol,
    NoSymbol, NoSymbol, NoSymbol, XF86XK_AudioPlay, XF86XK_AudioPause,
    XF86XK_Launch3, XF86XK_Launch4, XF86XK_LaunchB, XF86XK_Suspend, XF86XK_Close,
    XF86XK_AudioPlay, XF86XK_AudioForward, NoSymbol, XK_Print, NoSymbol,
    XF86XK_WebCam, NoSymbol, NoSymbol, XF86XK_Mail, XF86XK_Messenger,
    XF86XK_Search, XF86XK_Go, XF86XK_Finance, XF86XK_Game, XF86XK_Shop,
    NoSymbol, XK_Cancel, XF86XK_MonBrightnessDown, XF86XK_MonBrightnessUp, XF86XK_AudioMedia,
    XF86XK_Display, XF86XK_KbdLightOnOff, XF86XK_KbdBrightnessDown, XF86XK_KbdBrightnessUp, XF86XK_Send,
    XF86XK_Reply, XF86XK_MailForward, XF86XK_Save, XF86XK_Documents, XF86XK_Battery,
    XF86XK_Bluetooth, XF86XK_WLAN, NoSymbol, NoSymbol, NoSymbol,
    NoSymbol, NoSymbol, NoSymbol, NoSymbol, NoSymbol,
    NoSymbol
};

static const char Xlib_default_char[256] = {
    '\0', '\0', '\0', '\0', '\0',
    '\0', '\0', '\0', '\0', '\0',
    '1', '2', '3', '4', '5',
    '6', '7', '8', '9', '0',
    '-', '=', '\0', '\t', 'q',
    'w', 'e', 'r', 't', 'y',
    'u', 'i', 'o', 'p', '[',
    ']', '\r', '\0', 'a', 's',
    'd', 'f', 'g', 'h', 'j',
    'k', 'l', ';', '\'', '`',
    '\0', '\\', 'z', 'x', 'c',
    'v', 'b', 'n', 'm', ',',
    '.', '/', '\0', '*', '\0',
    ' ', '\0', '\0', '\0', '\0',
    '\0', '\0', '\0', '\0', '\0',
    '\0', '\0', '\0', '\0', '\0',
    '\0', '\0', '-', '\0', '\0',
    '\0', '+', '\0', '\0', '\0',
    '\0', '\0', '\0', '\0', '<',
    '\0', '\0', '\0', '\0', '\0',
    '\0', '\0', '\0', '\0', '\r',
    '\0', '/', '\0', '\0', '\n',
    '\0', '\0', '\0', '\0', '\0',
    '\0', '\0', '\0', '\0', '\0',
    '\0', '\0', '\0', '\0', '\0',
    '=', '+', '\0', '\0', '.',
    '\0', '\0', '\0', '\0', '\0',
    '\0', '\0', '\0', '\0', '\0',
    '\0', '\0', '\0', '\0', '\0',
    '\0', '\0', '\0', '\0', '\0',
    '\0', '\0', '\0', '\0', '\0',
    '\0', '\0', '\0', '\0', '\0',
    '\0', '\0', '\0', '\0', '\0',
    '\0', '\0', '\0', '\0', '\0',
    '\0', '\0', '\0', '\0', '\0',
    '\0', '\0', '\0', '\0', '\0',
    '\0', '\0', '\0', '\0', '\0',
    '\0', '\0', '\0', '\0', '\0',
    '\0', '\0', '\0', '\0', '\0',
    '\0', '\0', '\0', '\0', '\0',
    '\0', '\0', '\0', '\0', '\0',
    '\0', '\0', '\0', '\0', '\0',
    '\0', '\0', '\0', '\0', '\0',
    '\0', '\0', '\0', '\0', '\0',
    '\0', '\0', '\0', '\0', '\0',
    '\0', '\0', '\0', '\0', '\0',
    '\0', '\0', '\0', '\0', '\0',
    '\0', '\0', '\0', '\0', '\0',
    '\0', '\0', '\0', '\0', '\0',
    '\0', '\0', '\0', '\0', '\0',
    '\0', '\0', '\0', '\0', '\0',
    '\0'
};

/* Override */ KeySym XKeycodeToKeysym(Display* display, KeyCode keycode, int index)
{
    debuglogstdio(LCF_KEYBOARD, "%s called with keycode %d", __func__, (int)keycode);
    KeySym sym = Xlib_default_keymap[keycode];
    debuglogstdio(LCF_KEYBOARD, "   returning %d", sym);
    return sym;
}

/* Override */ KeySym XkbKeycodeToKeysym(Display *dpy, KeyCode kc, int group, int level)
{
    debuglogstdio(LCF_KEYBOARD, "%s called with keycode %d", __func__, (int)kc);
    KeySym sym = Xlib_default_keymap[kc];
    debuglogstdio(LCF_KEYBOARD, "   returning %d", sym);
    return sym;
}

/* Override */ KeyCode XKeysymToKeycode( Display* display, KeySym keysym)
{
    debuglogstdio(LCF_KEYBOARD, "%s called with keysym %d", __func__, keysym);
    KeyCode kc = 0;
    /* Translate from uppercase letter to lowercase */
    if ((keysym >= XK_A) && (keysym <= XK_Z)) {
        keysym += (XK_a - XK_A);
    }

    for (int i=0; i<256; i++) {
        if (Xlib_default_keymap[i] == keysym) {
            kc = static_cast<KeyCode>(i);
            break;
        }
    }

    debuglogstdio(LCF_KEYBOARD, "   returning %d", (int)kc);
    return kc;
}

/* Override */ int XLookupString(XKeyEvent *event_struct, char *buffer_return, int bytes_buffer, KeySym *keysym_return, void *status_in_out)
{
    debuglogstdio(LCF_KEYBOARD, "%s called with keycode %d", __func__, event_struct->keycode);
    // printBacktrace();
    KeyCode keycode = event_struct->keycode;
    if (keysym_return) {
        *keysym_return = Xlib_default_keymap[keycode];
    }
    if (buffer_return && (bytes_buffer > 0)) {
        char c = Xlib_default_char[keycode];
        if (c == '\0') {
            return 0;
        }
        buffer_return[0] = c;
        return 1;
    }
    return 0;
}

/* Override */ int XmbLookupString(XIC ic, XKeyPressedEvent *event, char *buffer_return, int bytes_buffer, KeySym *keysym_return, Status *status_return)
{
    debuglogstdio(LCF_KEYBOARD, "%s called with keycode %d", __func__, event->keycode);
    KeyCode keycode = event->keycode;
    KeySym keysym = Xlib_default_keymap[keycode];

    /* Return if no associated keysym */
    if (keysym == NoSymbol) {
        *status_return = XLookupNone;
        return 0;
    }

    if (keysym_return)
        *keysym_return = keysym;

    char c = Xlib_default_char[keycode];

    /* Return if no associated string */
    if (c == '\0') {
        *status_return = keysym_return ? XLookupKeySym : XLookupNone;
        return 0;
    }

    *status_return = keysym_return ? XLookupBoth : XLookupChars;
    if (buffer_return && (bytes_buffer > 0)) {
        buffer_return[0] = c;
        return 1;
    }
    return 0;
}

/* Override */ int XwcLookupString(XIC ic, XKeyPressedEvent *event, wchar_t *buffer_return, int wchars_buffer, KeySym *keysym_return, Status *status_return)
{
    debuglogstdio(LCF_KEYBOARD, "%s called with keycode %d", __func__, event->keycode);
    KeyCode keycode = event->keycode;
    KeySym keysym = Xlib_default_keymap[keycode];

    /* Return if no associated keysym */
    if (keysym == NoSymbol) {
        *status_return = XLookupNone;
        return 0;
    }

    if (keysym_return) {
        *keysym_return = keysym;
    }

    char c = Xlib_default_char[keycode];

    /* Return if no associated string */
    if (c == '\0') {
        *status_return = keysym_return ? XLookupKeySym : XLookupNone;
        return 0;
    }

    *status_return = keysym_return ? XLookupBoth : XLookupChars;
    if (buffer_return && (wchars_buffer > 0)) {
        buffer_return[0] = c;
        return 1;
    }
    return 0;
}

/* Override */ int Xutf8LookupString(XIC ic, XKeyPressedEvent *event, char *buffer_return, int bytes_buffer, KeySym *keysym_return, Status *status_return)
{
    debuglogstdio(LCF_KEYBOARD, "%s called with keycode %d", __func__, event->keycode);
    KeyCode keycode = event->keycode;
    KeySym keysym = Xlib_default_keymap[keycode];

    /* Return if no associated keysym */
    if (keysym == NoSymbol) {
        *status_return = XLookupNone;
        return 0;
    }
    
    if (keysym_return)
        *keysym_return = keysym;

    char c = Xlib_default_char[keycode];

    /* Return if no associated string */
    if (c == '\0') {
        *status_return = keysym_return ? XLookupKeySym : XLookupNone;
        return 0;
    }

    *status_return = keysym_return ? XLookupBoth : XLookupChars;
    if (buffer_return && (bytes_buffer > 0)) {
        buffer_return[0] = c;
        return 1;
    }
    return 0;
}

/* Override */ KeySym *XGetKeyboardMapping(Display *display, KeyCode first_keycode, int keycode_count, int *keysyms_per_keycode_return)
{
    debuglogstdio(LCF_KEYBOARD, "%s called with keycode_count %d", __func__, keycode_count);
    *keysyms_per_keycode_return = 1;
    KeySym *keysyms = static_cast<KeySym*>(malloc(keycode_count*(*keysyms_per_keycode_return)*sizeof(KeySym)));
    for (int c=0; c<keycode_count; c++) {
        keysyms[c] = Xlib_default_keymap[c+first_keycode];
    }
    return keysyms;
}


}
