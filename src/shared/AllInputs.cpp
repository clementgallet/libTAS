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

#include "AllInputs.h"

void AllInputs::emptyInputs() {
    int i,j;
    for (i=0; i<MAXKEYS; i++)
        keyboard[i] = XK_VoidSymbol;

    pointer_x = 0;
    pointer_y = 0;
    pointer_mask = 0;

    for (i=0; i<MAXJOYS; i++) {
        for (j=0; j<MAXAXES; j++)
            controller_axes[i][j] = 0;
        controller_buttons[i] = 0;
    }
}
