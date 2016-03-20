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

#ifndef ALLINPUTS_H_INCLUDED
#define ALLINPUTS_H_INCLUDED

#define ALLINPUTS_MAXKEY 16

#include <X11/Xlib.h> // For the KeySym type
#include <X11/keysym.h>

class AllInputs {
    public:
        KeySym keyboard[ALLINPUTS_MAXKEY];
        short controller_axes[4][6];
        unsigned short controller_buttons[4];

        void emptyInputs();
};

#endif // INPUTS_H_INCLUDED
