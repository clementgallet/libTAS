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

#ifndef LINTAS_RAMWATCHEDITWINDOW_H_INCLUDED
#define LINTAS_RAMWATCHEDITWINDOW_H_INCLUDED

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Pack.H>
#include <Fl/Fl_Choice.H>
#include <Fl/Fl_Box.H>
#include <Fl/Fl_Input.H>
#include <Fl/Fl_Int_Input.H>

// #include <X11/Xlib.h>
// #include "../Context.h"
#include "../ramsearch/IRamWatchDetailed.h"
#include <memory>

class RamWatchEditWindow {
    public:
        RamWatchEditWindow();
        // Context *context;
        // RamSearch ram_search;

        std::unique_ptr<IRamWatchDetailed> ramwatch;

        Fl_Double_Window *window;

        Fl_Pack *edit_pack;

        Fl_Int_Input *address_input;
        Fl_Input *label_input;

        Fl_Choice *type_choice;
        static Fl_Menu_Item type_items[];
        Fl_Choice *display_choice;
        static Fl_Menu_Item display_items[];

        Fl_Pack *button_pack;

        Fl_Button *save_button;
        Fl_Button *cancel_button;

        void fill(std::unique_ptr<IRamWatchDetailed> &watch);
        void update();
};

#endif
