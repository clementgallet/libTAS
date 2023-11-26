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

#ifndef LIBTAS_GENERALIO_H_INCLUDED
#define LIBTAS_GENERALIO_H_INCLUDED

#include "hook.h"

#include <cstdio>

namespace libtas {

/* Rename file OLD to NEW.  */
OVERRIDE int rename (const char *oldf, const char *newf) __THROW;

/* Remove file FILENAME.  */
OVERRIDE int remove (const char *filename) __THROW;

/* Remove the link NAME.  */
OVERRIDE int unlink (const char *name) __THROW;
}

#endif
