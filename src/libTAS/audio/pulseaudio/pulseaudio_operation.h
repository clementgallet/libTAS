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

#ifndef LIBTAS_PULSEAUDIO_OPERATION_H_INCL
#define LIBTAS_PULSEAUDIO_OPERATION_H_INCL

#include "../../global.h"

namespace libtas {

/** The state of an operation */
typedef enum pa_operation_state {
    PA_OPERATION_RUNNING,
    /**< The operation is still running */
    PA_OPERATION_DONE,
    /**< The operation has completed */
    PA_OPERATION_CANCELLED
    /**< The operation has been cancelled. Operations may get cancelled by the
     * application, or as a result of the context getting disconneted while the
     * operation is pending. */
} pa_operation_state_t;

/** An asynchronous operation object */
typedef void pa_operation;

/** Decrease the reference count by one */
OVERRIDE void pa_operation_unref(pa_operation *o);

/** Return the current status of the operation */
OVERRIDE pa_operation_state_t pa_operation_get_state(pa_operation *o);

}

#endif
