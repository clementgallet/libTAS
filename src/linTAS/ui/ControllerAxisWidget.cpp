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

#include "ControllerAxisWidget.h"
#include <FL/fl_draw.H>
#include <cstdint>
#include <iostream>

// static Fl_Callback start_cb;

ControllerAxisWidget::ControllerAxisWidget(int x, int y, int w, int h, const char *label) :
    Fl_Widget(x, y, w, h, label)
{
    x_axis = y_axis = 0;
    // when(FL_WHEN_CHANGED);
}


void ControllerAxisWidget::draw()
{
    // Fl_Widget::draw();
    // int w = this->w();
    // int h = this->h();

    fl_color(200, 200, 200);
    fl_rectf(x(), y(), w(), h());

    fl_color(100, 100, 100);
    fl_rect(x(), y(), w(), h());
    fl_arc(x(), y(), w(), h(), 0, 360);

    fl_color(0, 0, 0);

    fl_line(x()+ w()/2, y() + h()/2, x() + ((double)x_axis - INT16_MIN) * w() / UINT16_MAX, y() + ((double)y_axis - INT16_MIN) * h() / UINT16_MAX);

}

int ControllerAxisWidget::handle(int event)
{
    switch (event) {
        default:
            return Fl_Widget::handle(event);
        case FL_PUSH:
        case FL_DRAG:
        case FL_RELEASE:
            int x_axis_unclip = ((Fl::event_x() - x()) * UINT16_MAX / w()) + INT16_MIN;
            int y_axis_unclip = ((Fl::event_y() - y()) * UINT16_MAX / h()) + INT16_MIN;

            /* Clip values to short */
            #define clamptofullsignedrange(x,lo,hi) ((static_cast<unsigned int>((x)-(lo))<=static_cast<unsigned int>((hi)-(lo)))?(x):(((x)<0)?(lo):(hi)))

            x_axis = static_cast<short>(clamptofullsignedrange(x_axis_unclip, INT16_MIN, INT16_MAX));
            y_axis = static_cast<short>(clamptofullsignedrange(y_axis_unclip, INT16_MIN, INT16_MAX));

            // std::cout << Fl::event_x() << " - " << x_axis << std::endl;
            // std::cout << Fl::event_y() << " - " << y_axis << std::endl;
            redraw();

            do_callback();
            return 1;
    }
}
