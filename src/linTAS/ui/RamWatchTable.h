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

#ifndef LINTAS_RAMWATCHTABLE_H_INCLUDED
#define LINTAS_RAMWATCHTABLE_H_INCLUDED

#include "../ramsearch/IRamWatchDetailed.h"

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Table_Row.H>
#include <vector>
#include <memory>

class RamWatchTable : public Fl_Table_Row {
    public:
        /* A reference to the vector of addresses to watch */
        std::vector<std::unique_ptr<IRamWatchDetailed>> ramwatches;

        RamWatchTable(int X, int Y, int W, int H, const char *l=0);
        void draw_cell (TableContext context, int R=0, int C=0, int X=0, int Y=0, int W=0, int H=0);
        void update();
};

#endif
