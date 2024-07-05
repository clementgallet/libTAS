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

#ifndef LIBTAS_XCBWINDOWS_H_INCL
#define LIBTAS_XCBWINDOWS_H_INCL

#include "hook.h"

#include <xcb/xcb.h>

namespace libtas {

OVERRIDE xcb_void_cookie_t
xcb_create_window_checked (xcb_connection_t *c,
                           uint8_t           depth,
                           xcb_window_t      wid,
                           xcb_window_t      parent,
                           int16_t           x,
                           int16_t           y,
                           uint16_t          width,
                           uint16_t          height,
                           uint16_t          border_width,
                           uint16_t          _class,
                           xcb_visualid_t    visual,
                           uint32_t          value_mask,
                           const void       *value_list);

OVERRIDE xcb_void_cookie_t
xcb_create_window (xcb_connection_t *c,
                   uint8_t           depth,
                   xcb_window_t      wid,
                   xcb_window_t      parent,
                   int16_t           x,
                   int16_t           y,
                   uint16_t          width,
                   uint16_t          height,
                   uint16_t          border_width,
                   uint16_t          _class,
                   xcb_visualid_t    visual,
                   uint32_t          value_mask,
                   const void       *value_list);

OVERRIDE xcb_void_cookie_t
xcb_create_window_aux_checked (xcb_connection_t                     *c,
                               uint8_t                               depth,
                               xcb_window_t                          wid,
                               xcb_window_t                          parent,
                               int16_t                               x,
                               int16_t                               y,
                               uint16_t                              width,
                               uint16_t                              height,
                               uint16_t                              border_width,
                               uint16_t                              _class,
                               xcb_visualid_t                        visual,
                               uint32_t                              value_mask,
                               const xcb_create_window_value_list_t *value_list);

OVERRIDE xcb_void_cookie_t
xcb_create_window_aux (xcb_connection_t                     *c,
                       uint8_t                               depth,
                       xcb_window_t                          wid,
                       xcb_window_t                          parent,
                       int16_t                               x,
                       int16_t                               y,
                       uint16_t                              width,
                       uint16_t                              height,
                       uint16_t                              border_width,
                       uint16_t                              _class,
                       xcb_visualid_t                        visual,
                       uint32_t                              value_mask,
                       const xcb_create_window_value_list_t *value_list);

OVERRIDE xcb_void_cookie_t xcb_destroy_window_checked (xcb_connection_t *c, xcb_window_t window);

OVERRIDE xcb_void_cookie_t xcb_destroy_window (xcb_connection_t *c, xcb_window_t window);

OVERRIDE xcb_void_cookie_t xcb_map_window_checked (xcb_connection_t *c, xcb_window_t window);

OVERRIDE xcb_void_cookie_t xcb_map_window (xcb_connection_t *c, xcb_window_t window);

OVERRIDE xcb_void_cookie_t xcb_unmap_window_checked (xcb_connection_t *c, xcb_window_t window);

OVERRIDE xcb_void_cookie_t xcb_unmap_window (xcb_connection_t *c, xcb_window_t window);

OVERRIDE xcb_void_cookie_t xcb_configure_window_checked (xcb_connection_t *c, xcb_window_t window, uint16_t value_mask, const void *value_list);

OVERRIDE xcb_void_cookie_t xcb_configure_window (xcb_connection_t *c, xcb_window_t window, uint16_t value_mask, const void *value_list);

}

#endif
