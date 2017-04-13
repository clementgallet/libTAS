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

#include "InputWindow.h"
//#include "../main.h"
#include <iostream>

InputWindow::InputWindow(Context* c) : context(c)
{
    window = new Fl_Double_Window(600, 500);

    /* Browsers */
    hotkey_browser = new Fl_Multi_Browser(10, 10, 250, 400, "Hotkeys");
    input_browser = new Fl_Multi_Browser(300, 10, 250, 400, "Inputs");

    /* Set two columns */
    static int col_width[] = {150, 100, 0};
    hotkey_browser->column_widths(col_width);
    hotkey_browser->column_char('\t');
    input_browser->column_widths(col_width);
    input_browser->column_char('\t');

    /* Fill hotkey list */
    for (auto iter : context->km.hotkey_list) {
        std::string linestr(iter.description);
        /* Check if a key is mapped to this hotkey */
        for (auto itermap : context->km.hotkey_mapping) {
            if (itermap.second == iter) {
                linestr += '\t';
                linestr += XKeysymToString(itermap.first);
                break;
            }
        }
        /* Add the line in the browser */
        hotkey_browser->add(linestr.c_str());
    }

    // start = new Fl_Button(400, 120, 70, 30, "Ok");
    // start->callback((Fl_Callback*) start_cb, this);
    //
    // cancel = new Fl_Button(500, 120, 70, 30, "Cancel");
    // cancel->callback((Fl_Callback*) cancel_cb, this);

    window->end();
}

// void start_cb(Fl_Widget* w, void* v)
// {
//     EncodeWindow* ew = (EncodeWindow*) v;
//
//     /* Fill encode filename */
//     const char* filename = ew->encodepath->value();
//     std::string ext = EncodeWindow::container_items[ew->containerchoice->value()].label();
//     ew->context->dumpfile = std::string(filename) + ext;
//
//     /* TODO: Set video and audio codec */
//
//     /* Close window */
//     ew->window->hide();
// }
//
// void cancel_cb(Fl_Widget* w, void* v)
// {
//     EncodeWindow* ew = (EncodeWindow*) v;
//
//     /* Close window */
//     ew->window->hide();
// }
