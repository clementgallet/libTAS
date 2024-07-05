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

#ifndef LIBTAS_XCBREQUESTS_H_INCL
#define LIBTAS_XCBREQUESTS_H_INCL

#include "hook.h"

#include <xcb/xcb.h>
#include <xcb/xcbext.h>

namespace libtas {

OVERRIDE unsigned int xcb_send_request(xcb_connection_t *c, int flags, struct iovec *vector, const xcb_protocol_request_t *req);
OVERRIDE unsigned int xcb_send_request_with_fds(xcb_connection_t *c, int flags, struct iovec *vector, const xcb_protocol_request_t *req, unsigned int num_fds, int *fds);
OVERRIDE uint64_t xcb_send_request64(xcb_connection_t *c, int flags, struct iovec *vector, const xcb_protocol_request_t *req);
OVERRIDE uint64_t xcb_send_request_with_fds64(xcb_connection_t *c, int flags, struct iovec *vector, const xcb_protocol_request_t *req, unsigned int num_fds, int *fds);

}

#endif
