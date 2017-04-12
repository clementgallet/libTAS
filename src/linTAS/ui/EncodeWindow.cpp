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

#include "EncodeWindow.h"
//#include "../main.h"
#include <iostream>

EncodeWindow::EncodeWindow(Context* c) : context(c)
{
    window = new Fl_Double_Window(600, 180);

    /* Game Executable */
    encodepath = new Fl_Output(10, 30, 400, 30, "Encode file path");
    encodepath->align(FL_ALIGN_TOP_LEFT);
    encodepath->color(FL_LIGHT1);
    encodepath->value(context->gamepath.c_str());

    browseencodepath = new Fl_Button(520, 30, 70, 30, "Browse...");
    browseencodepath->callback((Fl_Callback*) browse_encodepath_cb, this);

    encodepathchooser = new Fl_Native_File_Chooser(Fl_Native_File_Chooser::BROWSE_SAVE_FILE);
    encodepathchooser->title("Choose an encode filename");
    encodepathchooser->preset_file(encodepath->value());
    encodepathchooser->options(Fl_Native_File_Chooser::SAVEAS_CONFIRM);

    containerchoice = new Fl_Choice(420, 30, 80, 30);
    containerchoice->menu(container_items);

    videochoice = new Fl_Choice(10, 120, 100, 30, "Video codec");
    videochoice->align(FL_ALIGN_TOP_LEFT);
    videochoice->menu(video_items);

    audiochoice = new Fl_Choice(150, 120, 100, 30, "Audio codec");
    audiochoice->align(FL_ALIGN_TOP_LEFT);
    audiochoice->menu(audio_items);

    start = new Fl_Button(400, 120, 70, 30, "Start");
    start->callback((Fl_Callback*) start_cb, this);

    cancel = new Fl_Button(500, 120, 70, 30, "Cancel");
    cancel->callback((Fl_Callback*) cancel_cb, this);

    window->end();
}

Fl_Menu_Item EncodeWindow::container_items[] = {
    {".mkv", 0, nullptr, nullptr},
    {nullptr}
};

Fl_Menu_Item EncodeWindow::video_items[] = {
    {"h264", 0, nullptr, nullptr},
    {nullptr}
};

Fl_Menu_Item EncodeWindow::audio_items[] = {
    {"ogg", 0, nullptr, nullptr},
    {nullptr}
};

void start_cb(Fl_Widget* w, void* v)
{
    EncodeWindow* ew = (EncodeWindow*) v;

    /* Fill encode filename */
    const char* filename = ew->encodepathchooser->filename();
    std::string ext = EncodeWindow::container_items[ew->containerchoice->value()].label();
    ew->context->dumpfile = std::string(filename) + ext;

    /* TODO: Set video and audio codec */

    /* Set encode status */
    ew->context->tasflags.av_dumping = 1;

    /* Close window */
    ew->window->hide();
}

void cancel_cb(Fl_Widget* w, void* v)
{
    EncodeWindow* ew = (EncodeWindow*) v;

    /* Close window */
    ew->window->hide();
}

void browse_encodepath_cb(Fl_Widget* w, void* v)
{
    EncodeWindow* ew = (EncodeWindow*) v;
    int ret = ew->encodepathchooser->show();

    const char* filename = ew->encodepathchooser->filename();

    /* If the user picked a file */
    if (ret == 0) {
        ew->encodepath->value(filename);
    }
}
