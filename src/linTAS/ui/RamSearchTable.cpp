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

#include "RamSearchTable.h"
#include <FL/fl_draw.H>
#include <inttypes.h> // PRIxPTR

RamSearchTable::RamSearchTable(std::vector<std::unique_ptr<IRamWatch>> *rw, int X, int Y, int W, int H, const char *l) : Fl_Table_Row(X, Y, W, H, l)
{
    ramwatches = rw;

    selection_color(FL_BLUE);

    rows(0);
    row_header(0);
    row_height_all(20);
    row_resize(0);

    cols(3);
    col_header(1);
    col_width(0, 110);
    col_width(1, 180);
    col_width(2, 180);
    col_resize(1);

    end();
}

void RamSearchTable::draw_cell (TableContext context, int R, int C, int X, int Y, int W, int H)
{
    static char addr[50];

    switch (context) {
    case CONTEXT_STARTPAGE:             // Fl_Table telling us it's starting to draw page
        fl_font(FL_COURIER, 16);
        return;

    case CONTEXT_ROW_HEADER:
        return;

    case CONTEXT_COL_HEADER:
        fl_push_clip(X, Y, W, H);
        {
            fl_draw_box(FL_THIN_UP_BOX, X, Y, W, H, color());
            fl_color(FL_BLACK);
            switch(C) {
                case 0:
                    fl_draw("Address", X, Y, W, H, FL_ALIGN_CENTER);
                    break;
                case 1:
                    fl_draw("Value", X, Y, W, H, FL_ALIGN_CENTER);
                    break;
                case 2:
                    fl_draw("Previous Value", X, Y, W, H, FL_ALIGN_CENTER);
                    break;
                default:
                    break;
            }
        }
        fl_pop_clip();
        return;

    case CONTEXT_CELL:
        if (R >= ramwatches->size())
            return;
        if (R < 0)
            return;

        fl_push_clip(X, Y, W, H);
        {
            std::unique_ptr<IRamWatch> &watch = ramwatches->at(R);

            // BG COLOR
            fl_color( row_selected(R) ? selection_color() : watch->check_no_update(compare_type, compare_operator, compare_value_db) ? FL_RED : FL_WHITE);
            fl_rectf(X, Y, W, H);

            // TEXT
            fl_color(FL_BLACK);
            switch(C) {
                case 0:
                    snprintf(addr, 50, "%" PRIxPTR, watch->address);
                    fl_draw(addr, X, Y, W, H, FL_ALIGN_CENTER);
                    break;
                case 1:
                    fl_draw(watch->tostring_current(hex), X, Y, W, H, FL_ALIGN_CENTER);
                    break;
                case 2:
                    fl_draw(watch->tostring(hex), X, Y, W, H, FL_ALIGN_CENTER);
                    break;
                default:
                    break;
            }

            // BORDER
            // fl_color(FL_LIGHT2);
            // fl_rect(X, Y, W, H);
        }
        fl_pop_clip();
        return;

    default:
        return;
    }
}
