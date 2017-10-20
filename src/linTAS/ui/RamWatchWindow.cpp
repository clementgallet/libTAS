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

#include "RamWatchWindow.h"
// #include "MainWindow.h"
#include <iostream>
#include <sstream>
#include <algorithm> // std::remove_if
// #include <X11/XKBlib.h>
// #include <FL/names.h>
// #include <FL/x.H>
// #include "../ramsearch/CompareEnums.h"

static Fl_Callback add_cb;
static Fl_Callback remove_cb;

RamWatchWindow::RamWatchWindow(Context* c) : context(c)
{
    window = new Fl_Double_Window(500, 700, "Ram Watch");

    /* Table */
    watch_table = new RamWatchTable(10, 10, 480, 630, "");

    /* Progress bar */
    // search_progress = new Fl_Hor_Fill_Slider(10, 650, 480, 10);
    // search_progress->hide();
    // search_progress->selection_color(FL_BLUE);
    // search_progress->box(FL_THIN_DOWN_FRAME);
    // search_progress->slider(FL_FLAT_BOX);
    //
    // watch_count = new Fl_Box(10, 670, 480, 30);
    // watch_count->box(FL_NO_BOX);
    // watch_count->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);

    add_watch = new Fl_Button(10, 650, 70, 30, "Add");
    add_watch->callback(add_cb, this);

    remove_watch = new Fl_Button(100, 650, 70, 30, "Remove");
    remove_watch->callback(remove_cb, this);

    window->end();

    edit_window = new RamWatchEditWindow();
}

void RamWatchWindow::update()
{
    watch_table->update();
}

void add_cb(Fl_Widget* w, void* v)
{
    RamWatchWindow* rww = static_cast<RamWatchWindow*>(v);
    rww->edit_window->window->show();

    while (rww->edit_window->window->shown()) {
        Fl::wait();
    }

    if (rww->edit_window->ramwatch) {
        rww->edit_window->ramwatch->game_pid = rww->context->game_pid;
        rww->watch_table->ramwatches.push_back(std::move(rww->edit_window->ramwatch));
    }
}

void remove_cb(Fl_Widget* w, void* v)
{

}
