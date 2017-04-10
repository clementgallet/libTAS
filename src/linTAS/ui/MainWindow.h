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

#ifndef LINTAS_MAINWINDOW_H_INCLUDED
#define LINTAS_MAINWINDOW_H_INCLUDED

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Int_Input.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/Fl_Output.H>
#include <thread>

#include "../Context.h"

class MainWindow {
    public:
        MainWindow(Context &c);
        std::thread game_thread;
        Context &context;
        Fl_Window *main_window;
        Fl_Button *launch;
        Fl_Input *gamepath;
        Fl_Button *browsegamepath;
        Fl_File_Chooser *gamepathchooser;

        Fl_Input *moviepath;
        Fl_Button *browsemoviepath;
        Fl_File_Chooser *moviepathchooser;

        Fl_Int_Input *logicalfps;

        Fl_Output *framecount;

        void update_status();
        void update();
};

void launch_cb(Fl_Widget*, void*);
void browse_gamepath_cb(Fl_Widget*, void*);
void browse_moviepath_cb(Fl_Widget*, void*);
void set_fps_cb(Fl_Widget*, void*);

#endif
