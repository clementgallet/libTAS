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

#ifndef LIBTAS_TLS_H_INCL
#define LIBTAS_TLS_H_INCL

#include "hook.h"
#include "global.h"
#include <pthread.h> // pthread_key_t

namespace libtas {

/* Clear all key values and call destructors if needed for the calling thread */
void clear_pthread_keys();

/* Create a key value identifying a location in the thread-specific
   data area.  Each thread maintains a distinct thread-specific data
   area.  DESTR_FUNCTION, if non-NULL, is called with the value
   associated to that key when the key is destroyed.
   DESTR_FUNCTION is not called if the value associated is NULL when
   the key is destroyed.  */
OVERRIDE int pthread_key_create (pthread_key_t *key, void (*destr_function) (void *)) throw();

/* Destroy KEY.  */
OVERRIDE int pthread_key_delete (pthread_key_t key) throw();

/* Return current value of the thread-specific data slot identified by KEY.  */
OVERRIDE void *pthread_getspecific (pthread_key_t key) throw();

/* Store POINTER in the thread-specific data slot identified by KEY. */
OVERRIDE int pthread_setspecific (pthread_key_t key, const void *pointer) throw();

}

#endif
