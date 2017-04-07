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
    main_window = new Fl_Window(600, 400);
    gamepath = new Fl_Input(10, 20, 500, 30, "Game path");
    gamepath->align(FL_ALIGN_TOP_LEFT);
    gamepath->value(c.gamepath.c_str());

    gamepathchooser = new Fl_File_Chooser(c.gamepath.c_str(), nullptr, Fl_File_Chooser::SINGLE, "Game path");
    gamepathchooser->preview(0);
//    gamepathchooser->callback(confirm_gamepath_cb, this);

    browsegamepath = new Fl_Button(520, 20, 70, 30, "Browse");
    browsegamepath->callback((Fl_Callback*) browse_gamepath_cb, this);

    framecount = new Fl_Output(80, 60, 60, 30, "Frames:");
    framestr = std::to_string(context.framecount);
    framecount->value(framestr.c_str());

    launch = new Fl_Button(10, 350, 70, 40, "Start");
    launch->callback((Fl_Callback*) launch_cb, this);
    main_window->end();
    main_window->show();
}

void MainWindow::update()
{
    /* This function is called by a child thread so we need to protect
       the calls to Fltk widgets */
    Fl::lock();
    /* Update frame count */
    framestr = std::to_string(context.framecount);
    framecount->value(framestr.c_str());
    framecount->redraw();

    /* Update game status (running/stopped) */
    if (quit) {
        launch->label("Start");
        launch->redraw();
    }

    Fl::unlock();
    Fl::awake();
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

void browse_gamepath_cb(Fl_Widget* w, void* v)
{
    MainWindow *mw = (MainWindow*) v;
    mw->gamepathchooser->show();
    /* This is not pretty, but the Fl_File_Chooser callback
       is called when click on a file, not when you hit "Ok"
       This is a workaround.
     */
    while (mw->gamepathchooser->shown()) {
        Fl::wait();
    }

    const char* filename = mw->gamepathchooser->value(1);

    /* If the user hit "Cancel", the returned value is null */
    if (filename) {
        mw->gamepath->value(mw->gamepathchooser->value(1));
        mw->context.gamepath = std::string(mw->gamepathchooser->value(1));
    }
}
