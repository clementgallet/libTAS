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

#include "pulseaudio_operation.h"
#include "../../logging.h"
#include "../../GlobalState.h"
#include "../../hook.h"

namespace libtas {

namespace orig {
    static void (*pa_operation_unref)(pa_operation *o);
    static pa_operation_state_t (*pa_operation_get_state)(pa_operation *o);
}

void pa_operation_unref(pa_operation *o)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE(pa_operation_unref, "libpulse.so");
        return orig::pa_operation_unref(o);
    }

    DEBUGLOGCALL(LCF_SOUND);
}

pa_operation_state_t pa_operation_get_state(pa_operation *o)
{
    if (GlobalState::isNative()) {
        LINK_NAMESPACE(pa_operation_get_state, "libpulse.so");
        return orig::pa_operation_get_state(o);
    }

    DEBUGLOGCALL(LCF_SOUND);
    return PA_OPERATION_DONE;
}

}
