/*
    Copyright 2015-2016 Clément Gallet <clement.gallet@ens-lyon.org>

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

#ifdef LIBTAS_ENABLE_CUSTOM_MALLOC
#ifndef LIBTAS_MALLOC_H_INCL
#define LIBTAS_MALLOC_H_INCL

#include "../global.h"
#include <cstdlib>

/* Allocate SIZE bytes of memory.  */
OVERRIDE void *malloc (size_t size) throw();

/* Allocate NMEMB elements of SIZE bytes each, all initialized to 0.  */
OVERRIDE void *calloc (size_t nmemb, size_t size) throw();

/* Re-allocate the previously allocated block in ptr, making the new
   block SIZE bytes long.  */
OVERRIDE void *realloc (void *ptr, size_t size) throw();

/* Allocate SIZE bytes on a page boundary.  */
OVERRIDE void *valloc (size_t size) throw();

/* Equivalent to valloc(minimum-page-that-holds(n)), that is, round up
   size to nearest pagesize. */
OVERRIDE void *pvalloc (size_t size) throw();

/* Allocate memory of SIZE bytes with an alignment of ALIGNMENT.  */
OVERRIDE int posix_memalign (void **memptr, size_t alignment, size_t size) throw();

/* ISO C variant of aligned allocation.  */
OVERRIDE void *aligned_alloc (size_t alignment, size_t size) throw();

/* Allocate SIZE bytes allocated to ALIGNMENT bytes.  */
OVERRIDE void *memalign (size_t alignment, size_t size) throw();

/* Free a block allocated by `malloc', `realloc' or `calloc'.  */
OVERRIDE void free (void *ptr) throw();

#endif
#endif

