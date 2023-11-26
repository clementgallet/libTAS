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

#ifndef LIBTAS_OPENALSOFT_H_INCL
#define LIBTAS_OPENALSOFT_H_INCL

#include "../../hook.h"
#include "../../global.h"

namespace libtas {

/** Check if OpenAL Soft is available */
bool check_al_soft_available();

/** Helper macro to link against OpenAL Soft functions */
#define LINK_NAMESPACE_ALSOFT(func) do { \
    link_function((void**)&orig::func, #func, "libopenal-soft.so.1"); \
    link_function((void**)&orig::func, #func, "libopenal.so.1"); \
} while (0)

/** Helper macro to use OpenAL Soft functions if preferred and available */
#define CHECK_USE_ALSOFT_FUNCTION(func, ...) \
    if (Global::shared_config.openal_soft && check_al_soft_available()) { \
        LINK_NAMESPACE_ALSOFT(func); \
        return orig::func(__VA_ARGS__); \
    }

}

#endif
