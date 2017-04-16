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
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Int_Input.H>
#include <FL/Fl_Native_File_Chooser.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Check_Button.H>
#include <Fl/Fl_Menu_Bar.H>
#include <Fl/Fl_Menu_Item.H>
#include <Fl/Fl_Pack.H>
#include <Fl/Fl_Radio_Round_Button.H>
#include <thread>

#include "../Context.h"
#include "EncodeWindow.h"
#include "InputWindow.h"

class MainWindow {
    public:
        /* Implement a Singleton pattern */
        static MainWindow& getInstance()
        {
            static MainWindow instance;
            return instance;
        }

        MainWindow(MainWindow const&) = delete;
        void operator=(MainWindow const&) = delete;

        void build(Context* c);

        std::thread game_thread;
        Context *context;
        Fl_Double_Window *window;

        Fl_Menu_Bar *menu_bar;
        static Fl_Menu_Item menu_items[];

        EncodeWindow* encode_window;
        InputWindow* input_window;

        Fl_Button *launch;

        Fl_Output *gamepath;
        Fl_Button *browsegamepath;
        Fl_Native_File_Chooser *gamepathchooser;

        Fl_Output *moviepath;
        Fl_Button *browsemoviepath;
        Fl_Native_File_Chooser *moviepathchooser;

        Fl_Pack *moviepack;
        Fl_Radio_Round_Button *movie_norec;
        Fl_Radio_Round_Button *movie_w;
        Fl_Radio_Round_Button *movie_rw;
        Fl_Radio_Round_Button *movie_ro;

        Fl_Int_Input *logicalfps;

        Fl_Check_Button *pausecheck;
        Fl_Check_Button *fastforwardcheck;

        Fl_Output *framecount;

        void update_status();
        void update(bool status);

    private:
        MainWindow() {}
};

static Fl_Callback0 launch_cb;
static Fl_Callback browse_gamepath_cb;
static Fl_Callback browse_moviepath_cb;
static Fl_Callback0 set_fps_cb;
static Fl_Callback0 pause_cb;
static Fl_Callback0 fastforward_cb;
static Fl_Callback0 recording_cb;
static Fl_Callback0 encode_cb;
static Fl_Callback config_encode_cb;
static Fl_Callback toggle_encode_cb;
static Fl_Callback config_input_cb;
static Fl_Callback sound_frequency_cb;
static Fl_Callback sound_bitdepth_cb;
static Fl_Callback sound_channel_cb;
static Fl_Callback mute_sound_cb;

#endif
