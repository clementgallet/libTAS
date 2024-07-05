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

#include "RenderHUD.h"

#ifndef LIBTAS_RENDERHUD_BASE_MACOS_H_INCL
#define LIBTAS_RENDERHUD_BASE_MACOS_H_INCL

#include <stdint.h>

namespace libtas {
/* This class provide a method to create a surface from
 * a text, using Core Text API. It returns a standard
 * 32-bit surface using the ARGB mask, encapsulated in a SurfaceARGB object.
 */
class RenderHUD_Base_MacOS : public RenderHUD
{
    public:
        ~RenderHUD_Base_MacOS();
};
}

#endif
