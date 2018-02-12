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

#ifndef LINTAS_RAMWATCHWINDOW_H_INCLUDED
#define LINTAS_RAMWATCHWINDOW_H_INCLUDED

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Radio_Round_Button.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Float_Input.H>
#include <FL/Fl_Hor_Fill_Slider.H>

#include "RamWatchTable.h"
#include "RamWatchEditWindow.h"

// #include <X11/Xlib.h>
#include "../Context.h"
// #include "../ramsearch/RamSearch.h"
#include <memory>

class RamWatchWindow {
    public:
        RamWatchWindow(Context *c);
        Context *context;
        // RamSearch ram_search;

        Fl_Double_Window *window;
        RamWatchEditWindow *edit_window;

        RamWatchTable *watch_table;

        Fl_Button *add_watch;
        Fl_Button *edit_watch;
        Fl_Button *remove_watch;

        void update();
};

#endif
