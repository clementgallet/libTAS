/*
    Copyright 2015-2023 Clément Gallet <clement.gallet@ens-lyon.org>

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

#include "xatom.h"
#include "../hook.h"
#include "../logging.h"

namespace libtas {

DEFINE_ORIG_POINTER(XInternAtom)

Atom XInternAtom(Display* display, const char* atom_name, Bool only_if_exists)
{
    debuglogstdio(LCF_WINDOW, "%s call with atom %s", __func__, atom_name);
    LINK_NAMESPACE_GLOBAL(XInternAtom);
    return orig::XInternAtom(display, atom_name, only_if_exists);
}

static const char * const atom_names[NB_XATOMS] =
{
    "WM_PROTOCOLS",
    "WM_TAKE_FOCUS",
    "WM_DELETE_WINDOW",
    "_NET_WM_STATE",
    "_NET_WM_STATE_FULLSCREEN",
    "_NET_WM_STATE_ABOVE",
    "_NET_WM_PING",
    "_MOTIF_WM_HINTS",
    "_NET_WM_NAME",
    "UTF8_STRING",
};

Atom X11Atoms[NB_XATOMS];

void initX11Atoms(Display* display)
{
    static bool atoms_inited = false;
    if (!atoms_inited) {
        XInternAtoms( display, (char **)atom_names, NB_XATOMS, False, X11Atoms );
        atoms_inited = true;
    }
}

}
