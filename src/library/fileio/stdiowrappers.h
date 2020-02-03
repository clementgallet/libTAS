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

#ifndef LIBTAS_STDIO_H_INCLUDED
#define LIBTAS_STDIO_H_INCLUDED

#include "../global.h"
#include <cstdio> // FILE

namespace libtas {

/* Open a file and create a new stream for it. */
OVERRIDE FILE *fopen (const char *filename, const char *modes);
OVERRIDE FILE *fopen64 (const char *filename, const char *modes);

/* Close STREAM. */
OVERRIDE int fclose (FILE *stream);

/* Return the system file descriptor for STREAM.  */
// OVERRIDE int fileno (FILE *stream) throw();

}

#endif
