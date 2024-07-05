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

#ifndef LIBTAS_ALLINPUTSFLAT_H_INCLUDED
#define LIBTAS_ALLINPUTSFLAT_H_INCLUDED

#include "SingleInput.h"
#include "AllInputs.h"

#include <array>
#include <set>
#include <cstdint>
#include <memory>

/* Input structure identical to AllInputs except that it has only all members. 
 * Used by the game. See AllInputs and other classes for details of the members.
 */
struct AllInputsFlat {
    static const int MAXKEYS = AllInputs::MAXKEYS;
    static const int MAXJOYS = AllInputs::MAXJOYS;

    std::array<uint32_t,MAXKEYS> keyboard;

    MouseInputs pointer;
    std::array<ControllerInputs,MAXJOYS> controllers;
    MiscInputs misc;

    /* Clear all fields */
    void clear();

    /* Receive inputs through the socket excluding the first message MSGN_ALL_INPUTS */
    void recv();
};

#endif
