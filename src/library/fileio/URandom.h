/*
    Copyright 2015-2023 Clément Gallet <clement.gallet@ens-lyon.org>

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

#ifndef LIBTAS_URANDOM_H_INCLUDED
#define LIBTAS_URANDOM_H_INCLUDED

#include <cstdio>

namespace libtas {

/* Creates a fd implementing our own deterministic /dev/urandom */
int urandom_create_fd();

/* Returns that fd, or -1 if not created */
int urandom_get_fd();

/* Creates a FILE* stream containing the above fd */
FILE* urandom_create_file();

/* Returns the /dev/urandom FILE* stream */
FILE* urandom_get_file();

/* Disable the signal handling */
void urandom_disable_handler();

/* Restore the signal handling */
void urandom_enable_handler();

}

#endif
