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

#ifndef LIBTAS_XKEYBOARDLAYOUT_H_INCL
#define LIBTAS_XKEYBOARDLAYOUT_H_INCL

#include <X11/X.h>
#include <X11/Xlib.h>

#include "../hook.h"

namespace libtas {

OVERRIDE KeySym XKeycodeToKeysym(Display* display, KeyCode keycode, int index);
// OVERRIDE KeySym XLookupKeysym( XKeyEvent* key_event, int index);
OVERRIDE KeyCode XKeysymToKeycode( Display* display, KeySym keysym);

OVERRIDE KeySym XkbKeycodeToKeysym(Display *dpy, KeyCode kc, int group, int level);

OVERRIDE int XLookupString(XKeyEvent *event_struct, char *buffer_return, int bytes_buffer, KeySym *keysym_return, void *status_in_out);

OVERRIDE int XmbLookupString(XIC ic, XKeyPressedEvent *event, char *buffer_return, int bytes_buffer, KeySym *keysym_return, Status *status_return);

OVERRIDE int XwcLookupString(XIC ic, XKeyPressedEvent *event, wchar_t *buffer_return, int wchars_buffer, KeySym *keysym_return, Status *status_return);

OVERRIDE int Xutf8LookupString(XIC ic, XKeyPressedEvent *event, char *buffer_return, int bytes_buffer, KeySym *keysym_return, Status *status_return); 

OVERRIDE KeySym *XGetKeyboardMapping(Display *display, KeyCode first_keycode, int keycode_count, int *keysyms_per_keycode_return);

}

#endif
