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

#ifndef LIBTAS_XCBKEYBOARD_H_INCL
#define LIBTAS_XCBKEYBOARD_H_INCL

#include "hook.h"

#include <xcb/xcb.h>
#include <xcb/xcb_keysyms.h>
#include <xcb/xproto.h>

namespace libtas {

OVERRIDE xcb_query_keymap_cookie_t xcb_query_keymap(xcb_connection_t *conn);
OVERRIDE xcb_query_keymap_cookie_t xcb_query_keymap_unchecked(xcb_connection_t *conn);
OVERRIDE xcb_query_keymap_reply_t* xcb_query_keymap_reply(xcb_connection_t *conn, xcb_query_keymap_cookie_t cookie, xcb_generic_error_t** error);

OVERRIDE xcb_grab_keyboard_cookie_t
xcb_grab_keyboard (xcb_connection_t *c,
                   uint8_t           owner_events,
                   xcb_window_t      grab_window,
                   xcb_timestamp_t   time,
                   uint8_t           pointer_mode,
                   uint8_t           keyboard_mode);

OVERRIDE xcb_grab_keyboard_cookie_t
xcb_grab_keyboard_unchecked (xcb_connection_t *c,
                             uint8_t           owner_events,
                             xcb_window_t      grab_window,
                             xcb_timestamp_t   time,
                             uint8_t           pointer_mode,
                             uint8_t           keyboard_mode);

OVERRIDE xcb_grab_keyboard_reply_t *
xcb_grab_keyboard_reply (xcb_connection_t            *c,
                         xcb_grab_keyboard_cookie_t   cookie  /**< */,
                         xcb_generic_error_t        **e);

OVERRIDE xcb_void_cookie_t
xcb_ungrab_keyboard_checked (xcb_connection_t *c,
                             xcb_timestamp_t   time);

OVERRIDE xcb_void_cookie_t
xcb_ungrab_keyboard (xcb_connection_t *c,
                     xcb_timestamp_t   time);

OVERRIDE xcb_void_cookie_t
xcb_grab_key_checked (xcb_connection_t *c,
                      uint8_t           owner_events,
                      xcb_window_t      grab_window,
                      uint16_t          modifiers,
                      xcb_keycode_t     key,
                      uint8_t           pointer_mode,
                      uint8_t           keyboard_mode);

OVERRIDE xcb_void_cookie_t
xcb_grab_key (xcb_connection_t *c,
              uint8_t           owner_events,
              xcb_window_t      grab_window,
              uint16_t          modifiers,
              xcb_keycode_t     key,
              uint8_t           pointer_mode,
              uint8_t           keyboard_mode);

OVERRIDE xcb_void_cookie_t
xcb_ungrab_key_checked (xcb_connection_t *c,
                        xcb_keycode_t     key,
                        xcb_window_t      grab_window,
                        uint16_t          modifiers);

OVERRIDE xcb_void_cookie_t
xcb_ungrab_key (xcb_connection_t *c,
                xcb_keycode_t     key,
                xcb_window_t      grab_window,
                uint16_t          modifiers);


}

#endif
