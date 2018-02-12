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

#ifndef LINTAS_RAMSEARCHWINDOW_H_INCLUDED
#define LINTAS_RAMSEARCHWINDOW_H_INCLUDED

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

#include "RamSearchTable.h"

// #include <X11/Xlib.h>
#include "../Context.h"
#include "../ramsearch/RamSearch.h"
#include <memory>

class RamSearchWindow {
    public:
        RamSearchWindow(Context *c);
        Context *context;
        RamSearch ram_search;

        Fl_Double_Window *window;

        RamSearchTable *address_table;
        Fl_Box *watch_count;
        Fl_Hor_Fill_Slider *search_progress;

        Fl_Pack *config_pack;

        Fl_Pack *mem_pack;
        Fl_Pack *mem_col1;
        Fl_Pack *mem_col2;

        Fl_Check_Button *mem_text;
        Fl_Check_Button *mem_data_ro;
        Fl_Check_Button *mem_data_rw;
        Fl_Check_Button *mem_bss;
        Fl_Check_Button *mem_heap;
        Fl_Check_Button *mem_file_mapping;
        Fl_Check_Button *mem_anonymous_mapping_ro;
        Fl_Check_Button *mem_anonymous_mapping_rw;
        Fl_Check_Button *mem_stack;
        Fl_Check_Button *mem_special;

        Fl_Pack *compare_pack;
        Fl_Radio_Round_Button *compare_previous;
        Fl_Radio_Round_Button *compare_value;
        Fl_Float_Input *comparing_value;

        Fl_Pack *operator_pack;
        Fl_Radio_Round_Button *operator_equal;
        Fl_Radio_Round_Button *operator_not_equal;
        Fl_Radio_Round_Button *operator_less;
        Fl_Radio_Round_Button *operator_greater;
        Fl_Radio_Round_Button *operator_less_equal;
        Fl_Radio_Round_Button *operator_greater_equal;

        Fl_Pack *type_pack;
        Fl_Choice *type_choice;
        static Fl_Menu_Item type_items[];
        Fl_Choice *display_choice;
        static Fl_Menu_Item display_items[];

        Fl_Button *new_button;
        Fl_Button *search_button;
        Fl_Button *add_button;

        void update();
};

#endif
