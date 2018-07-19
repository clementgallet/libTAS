/*
    Copyright 2015-2018 Clément Gallet <clement.gallet@ens-lyon.org>

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

#include "xcbkeyboard.h"
#include "../logging.h"
#include "inputs.h"
#include "xkeyboardlayout.h"
// #include <X11/XKBlib.h>
#include <cstring> // memset
#include "../../shared/AllInputs.h"

namespace libtas {

/* Override */ xcb_query_keymap_cookie_t xcb_query_keymap(xcb_connection_t *conn)
{
    DEBUGLOGCALL(LCF_KEYBOARD);
    xcb_query_keymap_cookie_t keymap_cookie;
    return keymap_cookie;
}

/* Override */ xcb_query_keymap_cookie_t xcb_query_keymap_unchecked(xcb_connection_t *conn)
{
    DEBUGLOGCALL(LCF_KEYBOARD);
    xcb_query_keymap_cookie_t keymap_cookie;
    return keymap_cookie;
}

/* Override */ xcb_query_keymap_reply_t* xcb_query_keymap_reply(xcb_connection_t *conn, xcb_query_keymap_cookie_t cookie, xcb_generic_error_t** error)
{
    DEBUGLOGCALL(LCF_KEYBOARD);

    /* The reply is allocated and the caller is in charge of freeing it */
    xcb_query_keymap_reply_t *reply = new xcb_query_keymap_reply_t;
    reply->response_type = XCB_QUERY_KEYMAP;
    /* Let's hope the caller does not care about the sequence */
    reply->sequence = 0;

    memset(reply->keys, 0, 32);
    GlobalNoLog gnl; // Avoid logging on XkbKeycodeToKeysym
    for (int kc=0; kc<256; kc++) {
        KeySym ks = XkbKeycodeToKeysym(nullptr, (KeyCode)kc, 0, 0);
        for (int i=0; i<AllInputs::MAXKEYS; i++) {
            if (ks == game_ai.keyboard[i]) {
                reply->keys[kc>>3] |= (1 << kc&0x7);
                break;
            }
        }
    }

    return reply;
}

/* Override */ xcb_grab_keyboard_cookie_t
xcb_grab_keyboard (xcb_connection_t *c,
                   uint8_t           owner_events,
                   xcb_window_t      grab_window,
                   xcb_timestamp_t   time,
                   uint8_t           pointer_mode,
                   uint8_t           keyboard_mode)
{
    DEBUGLOGCALL(LCF_KEYBOARD);
    xcb_grab_keyboard_cookie_t keyboard_grab_cookie;
    return keyboard_grab_cookie;
}

/* Override */ xcb_grab_keyboard_cookie_t
xcb_grab_keyboard_unchecked (xcb_connection_t *c,
                             uint8_t           owner_events,
                             xcb_window_t      grab_window,
                             xcb_timestamp_t   time,
                             uint8_t           pointer_mode,
                             uint8_t           keyboard_mode)
{
    DEBUGLOGCALL(LCF_KEYBOARD);
    xcb_grab_keyboard_cookie_t keyboard_grab_cookie;
    return keyboard_grab_cookie;
}

/* Override */ xcb_grab_keyboard_reply_t *
xcb_grab_keyboard_reply (xcb_connection_t            *c,
                         xcb_grab_keyboard_cookie_t   cookie  /**< */,
                         xcb_generic_error_t        **e)
{
    DEBUGLOGCALL(LCF_KEYBOARD);
    xcb_grab_keyboard_reply_t *reply = new xcb_grab_keyboard_reply_t;
    reply->response_type = XCB_GRAB_KEYBOARD;
    reply->sequence = 0;
    reply->status = XCB_GRAB_STATUS_SUCCESS;

    return reply;
}

/* Override */ xcb_void_cookie_t
xcb_ungrab_keyboard_checked (xcb_connection_t *c,
                             xcb_timestamp_t   time)
{
    DEBUGLOGCALL(LCF_KEYBOARD);
    xcb_void_cookie_t keyboard_ungrab_cookie;
    return keyboard_ungrab_cookie;
}

/* Override */ xcb_void_cookie_t
xcb_ungrab_keyboard (xcb_connection_t *c,
                     xcb_timestamp_t   time)
{
    DEBUGLOGCALL(LCF_KEYBOARD);
    xcb_void_cookie_t keyboard_ungrab_cookie;
    return keyboard_ungrab_cookie;
}

/* Override */ xcb_void_cookie_t
xcb_grab_key_checked (xcb_connection_t *c,
                      uint8_t           owner_events,
                      xcb_window_t      grab_window,
                      uint16_t          modifiers,
                      xcb_keycode_t     key,
                      uint8_t           pointer_mode,
                      uint8_t           keyboard_mode)
{
    DEBUGLOGCALL(LCF_KEYBOARD);
    xcb_void_cookie_t key_grab_cookie;
    return key_grab_cookie;
}

/* Override */ xcb_void_cookie_t
xcb_grab_key (xcb_connection_t *c,
              uint8_t           owner_events,
              xcb_window_t      grab_window,
              uint16_t          modifiers,
              xcb_keycode_t     key,
              uint8_t           pointer_mode,
              uint8_t           keyboard_mode)
{
    DEBUGLOGCALL(LCF_KEYBOARD);
    xcb_void_cookie_t key_grab_cookie;
    return key_grab_cookie;
}

/* Override */ xcb_void_cookie_t
xcb_ungrab_key_checked (xcb_connection_t *c,
                        xcb_keycode_t     key,
                        xcb_window_t      grab_window,
                        uint16_t          modifiers)
{
    DEBUGLOGCALL(LCF_KEYBOARD);
    xcb_void_cookie_t key_ungrab_cookie;
    return key_ungrab_cookie;
}

/* Override */ xcb_void_cookie_t
xcb_ungrab_key (xcb_connection_t *c,
                xcb_keycode_t     key,
                xcb_window_t      grab_window,
                uint16_t          modifiers)
{
    DEBUGLOGCALL(LCF_KEYBOARD);
    xcb_void_cookie_t key_ungrab_cookie;
    return key_ungrab_cookie;
}

}
