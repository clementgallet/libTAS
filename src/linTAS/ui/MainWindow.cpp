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
#include <iostream>

MainWindow::MainWindow(Context &c) : context(c)
{
    main_window = new Fl_Window(600, 400);

    /* Game Executable */
    gamepath = new Fl_Input(10, 300, 500, 30, "Game Executable");
    gamepath->align(FL_ALIGN_TOP_LEFT);
    gamepath->value(context.gamepath.c_str());

    gamepathchooser = new Fl_File_Chooser(context.gamepath.c_str(), nullptr, Fl_File_Chooser::SINGLE, "Game path");
    gamepathchooser->preview(0);

    browsegamepath = new Fl_Button(520, 300, 70, 30, "Browse...");
    browsegamepath->callback((Fl_Callback*) browse_gamepath_cb, this);

    /* Movie File */
    moviepath = new Fl_Input(10, 20, 500, 30, "Movie File");
    moviepath->align(FL_ALIGN_TOP_LEFT);
    moviepath->value(context.moviefile.c_str());

    moviepathchooser = new Fl_File_Chooser(context.moviefile.c_str(), nullptr, Fl_File_Chooser::SINGLE, "Choose a movie file");
    moviepathchooser->preview(0);

    browsemoviepath = new Fl_Button(520, 20, 70, 30, "Browse...");
    browsemoviepath->callback((Fl_Callback*) browse_moviepath_cb, this);

    logicalfps = new Fl_Int_Input(160, 120, 40, 30, "Frames per second");
    std::string fpsstr = std::to_string(context.tasflags.framerate);
    logicalfps->value(fpsstr.c_str());
    logicalfps->callback((Fl_Callback*) set_fps_cb, this);

    framecount = new Fl_Output(80, 60, 60, 30, "Frames:");
    std::string framestr = std::to_string(context.framecount);
    framecount->value(framestr.c_str());

    launch = new Fl_Button(10, 350, 70, 40, "Start");
    launch->callback((Fl_Callback*) launch_cb, this);
    main_window->end();
    main_window->show();
}

void MainWindow::update_status()
{
    /* Update game status (active/inactive) */

    /* This function might be called by another thread */
    Fl::lock();

    if (quit) {
        launch->label("Start");
        launch->redraw();
        moviepath->activate();
        browsemoviepath->activate();
        gamepath->activate();
        browsegamepath->activate();
        logicalfps->activate();

    }
    else {
        launch->label("Stop");
        launch->redraw();
        moviepath->deactivate();
        browsemoviepath->deactivate();
        gamepath->deactivate();
        browsegamepath->deactivate();
        logicalfps->deactivate();
    }

    Fl::unlock();
    Fl::awake();
}

void MainWindow::update()
{
    /* This function is called by another thread */
    Fl::lock();

    /* Update frame count */
    std::string framestr = std::to_string(context.framecount);
    framecount->value(framestr.c_str());
    framecount->redraw();

    Fl::unlock();
    Fl::awake();
}

void launch_cb(Fl_Widget* w, void* v)
{
    MainWindow *mw = (MainWindow*) v;
    if (quit) { // TODO: move this quit variable elsewhere, in Context ?

        /* Check that there might be a thread from a previous game execution */
        if (mw->game_thread.joinable())
            mw->game_thread.join();

        /* Start game */
        quit = false;
        mw->game_thread = std::thread{launchGame, nullptr};
    }
    else {
        w->deactivate();
        w->redraw();
        quit = true;
        mw->game_thread.detach();
        w->activate();
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

        /* Change the movie file also */
        mw->context.moviefile = mw->context.gamepath;
        mw->context.moviefile += ".ltm";
        mw->moviepath->value(mw->context.moviefile.c_str());
    }
}

void browse_moviepath_cb(Fl_Widget* w, void* v)
{
    // TODO: Almost duplicate of browse_gamepath_cb...
    MainWindow *mw = (MainWindow*) v;
    mw->moviepathchooser->show();
    /* This is not pretty, but the Fl_File_Chooser callback
       is called when click on a file, not when you hit "Ok"
       This is a workaround.
     */
    while (mw->moviepathchooser->shown()) {
        Fl::wait();
    }

    const char* filename = mw->moviepathchooser->value(1);

    /* If the user hit "Cancel", the returned value is null */
    if (filename) {
        mw->moviepath->value(mw->moviepathchooser->value(1));
        mw->context.moviefile = std::string(mw->moviepathchooser->value(1));
    }
}

void set_fps_cb(Fl_Widget* w, void* v)
{
    MainWindow *mw = (MainWindow*) v;
    Fl_Int_Input *ii = (Fl_Int_Input*) w;
    std::string fpsstr = ii->value();
    mw->context.tasflags.framerate = std::stoi(fpsstr);
    std::cout << "Set fps to " << mw->context.tasflags.framerate << std::endl;
}
