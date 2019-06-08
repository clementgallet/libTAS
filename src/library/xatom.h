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

#ifndef LIBTAS_XATOM_H_INCL
#define LIBTAS_XATOM_H_INCL

#include "global.h"
#include <X11/Xlib.h>

namespace libtas {

OVERRIDE Atom XInternAtom(Display* display, const char*	atom_name, Bool only_if_exists);

/* atoms */

void initX11Atoms(Display* display);

enum x11_atoms
{
    XATOM_WM_PROTOCOLS,
    XATOM_WM_TAKE_FOCUS,
    XATOM_WM_DELETE_WINDOW,
    XATOM__NET_WM_STATE,
    XATOM__NET_WM_STATE_FULLSCREEN,
    NB_XATOMS
};

extern Atom X11Atoms[NB_XATOMS];

#define x11_atom(name) (X11Atoms[XATOM_##name])

}

#endif
