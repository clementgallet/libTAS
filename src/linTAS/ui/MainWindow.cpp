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

#include "MainWindow.h"
#include "../main.h"

MainWindow::MainWindow(Context &c) : context(c)
{
    main_window = new Fl_Window(300, 200);
    launch = new Fl_Button(10, 70, 90, 25, "Start");
    launch->callback((Fl_Callback*) launch_cb, this);
    main_window->end();
    main_window->show();
}

void launch_cb(Fl_Widget* w, void* v)
{
    MainWindow *mw = (MainWindow*) v;
    if (quit) { // TODO: move this quit variable elsewhere, in Context ?
        /* Start game */
        mw->game_thread = std::thread{launchGame, nullptr};
        w->label("Stop");
        w->redraw();
    }
    else {
        w->deactivate();
        w->redraw();
        quit = true;
        mw->game_thread.join();
        w->activate();
        w->label("Start");
        w->redraw();
    }

}
