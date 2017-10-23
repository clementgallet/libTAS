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
static Fl_Callback edit_cb;
static Fl_Callback remove_cb;

RamWatchWindow::RamWatchWindow(Context* c) : context(c)
{
    window = new Fl_Double_Window(500, 700, "Ram Watch");

    /* Table */
    watch_table = new RamWatchTable(10, 10, 480, 630, "");

    add_watch = new Fl_Button(10, 650, 120, 30, "Add Watch");
    add_watch->callback(add_cb, this);

    edit_watch = new Fl_Button(170, 650, 120, 30, "Edit Watch");
    edit_watch->callback(edit_cb, this);

    remove_watch = new Fl_Button(330, 650, 120, 30, "Remove Watch");
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
        rww->watch_table->update();
    }
}

void edit_cb(Fl_Widget* w, void* v)
{
    RamWatchWindow* rww = static_cast<RamWatchWindow*>(v);

    int r;
    for (r=0; r<rww->watch_table->rows(); r++) {
        if (rww->watch_table->row_selected(r)) {
            /* Fill the watch edit window accordingly */
            rww->edit_window->fill(rww->watch_table->ramwatches.at(r));
            break;
        }
    }

    /* If no watch was selected, return */
    if (r == rww->watch_table->rows())
        return;

    rww->edit_window->window->show();

    while (rww->edit_window->window->shown()) {
        Fl::wait();
    }

    if (rww->edit_window->ramwatch) {
        rww->edit_window->ramwatch->game_pid = rww->context->game_pid;
        /* Replace the smart pointer object */
        rww->watch_table->ramwatches[r] = std::move(rww->edit_window->ramwatch);

//        selected_watch = rww->edit_window->ramwatch;
        rww->watch_table->update();
    }
}


void remove_cb(Fl_Widget* w, void* v)
{
    RamWatchWindow* rww = static_cast<RamWatchWindow*>(v);

    for (int r=rww->watch_table->rows() - 1; r>=0; r--) {
        if (rww->watch_table->row_selected(r)) {
            rww->watch_table->ramwatches.erase(rww->watch_table->ramwatches.begin() + r);
        }
    }

    rww->watch_table->select_all_rows(0);
}
