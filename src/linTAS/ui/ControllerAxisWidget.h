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

#ifndef LINTAS_CONTROLLERAXISWIDGET_H_INCLUDED
#define LINTAS_CONTROLLERAXISWIDGET_H_INCLUDED

#include <FL/Fl.H>
#include <FL/Fl_Widget.H>

class ControllerAxisWidget : public Fl_Widget {
    public:
        ControllerAxisWidget(int x, int y, int w, int h, const char *label=0L);

        void draw();

        int handle(int event);

        short x_axis, y_axis;
};

#endif
