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

#include "ExecutableWindow.h"
#include "MainWindow.h"
#include <iostream>

static Fl_Callback confirm_cb;
static Fl_Callback cancel_cb;
static Fl_Callback browse_runpath_cb;
static Fl_Callback browse_libpath_cb;
static Fl_Callback clear_runpath_cb;
static Fl_Callback clear_libpath_cb;

ExecutableWindow::ExecutableWindow(Context* c) : context(c)
{
    window = new Fl_Double_Window(680, 160, "Executable Options");

    /* Run path */
    runpath = new Fl_Output(10, 30, 500, 30, "Run path");
    runpath->align(FL_ALIGN_TOP_LEFT);
    runpath->color(FL_LIGHT1);

    browserunpath = new Fl_Button(520, 30, 70, 30, "Browse...");
    browserunpath->callback(browse_runpath_cb, this);

    runpathchooser = new Fl_Native_File_Chooser(Fl_Native_File_Chooser::BROWSE_DIRECTORY);
    runpathchooser->title("Choose an run directory");

    clear_runpath = new Fl_Button(600, 30, 70, 30, "Clear");
    clear_runpath->callback(clear_runpath_cb, this);

    /* Lib path */
    libpath = new Fl_Output(10, 80, 500, 30, "Library path");
    libpath->align(FL_ALIGN_TOP_LEFT);
    libpath->color(FL_LIGHT1);

    browselibpath = new Fl_Button(520, 80, 70, 30, "Browse...");
    browselibpath->callback(browse_libpath_cb, this);

    libpathchooser = new Fl_Native_File_Chooser(Fl_Native_File_Chooser::BROWSE_DIRECTORY);
    libpathchooser->title("Choose an lib directory");

    clear_libpath = new Fl_Button(600, 80, 70, 30, "Clear");
    clear_libpath->callback(clear_libpath_cb, this);

    /* Buttons */

    confirm = new Fl_Button(400, 120, 70, 30, "Ok");
    confirm->callback(confirm_cb, this);

    cancel = new Fl_Button(500, 120, 70, 30, "Cancel");
    cancel->callback(cancel_cb, this);

    update_config();
    window->end();
}

void ExecutableWindow::update_config()
{
    runpath->value(context->config.rundir.c_str());
    if (!context->config.rundir.empty())
        runpathchooser->preset_file(runpath->value());
    else
        runpathchooser->preset_file(context->gamepath.c_str());

    libpath->value(context->config.libdir.c_str());
    if (!context->config.libdir.empty())
        libpathchooser->preset_file(libpath->value());
    else
        libpathchooser->preset_file(context->gamepath.c_str());
}

void confirm_cb(Fl_Widget* w, void* v)
{
    ExecutableWindow* ew = (ExecutableWindow*) v;

    ew->context->config.rundir = ew->runpath->value();
    ew->context->config.libdir = ew->libpath->value();

    /* Close window */
    ew->window->hide();
}

void cancel_cb(Fl_Widget* w, void* v)
{
    ExecutableWindow* ew = (ExecutableWindow*) v;

    /* Close window */
    ew->window->hide();
}

void browse_runpath_cb(Fl_Widget* w, void* v)
{
    ExecutableWindow* ew = (ExecutableWindow*) v;
    int ret = ew->runpathchooser->show();

    const char* filename = ew->runpathchooser->filename();

    /* If the user picked a file */
    if (ret == 0) {
        ew->runpath->value(filename);
    }
}

void browse_libpath_cb(Fl_Widget* w, void* v)
{
    ExecutableWindow* ew = (ExecutableWindow*) v;
    int ret = ew->libpathchooser->show();

    const char* filename = ew->libpathchooser->filename();

    /* If the user picked a file */
    if (ret == 0) {
        ew->libpath->value(filename);
    }
}

void clear_runpath_cb(Fl_Widget* w, void* v)
{
    ExecutableWindow* ew = (ExecutableWindow*) v;
    ew->runpath->value("");
}

void clear_libpath_cb(Fl_Widget* w, void* v)
{
    ExecutableWindow* ew = (ExecutableWindow*) v;
    ew->libpath->value("");
}
