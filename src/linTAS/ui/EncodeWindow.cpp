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
#include "MainWindow.h"
//#include "../main.h"
#include <iostream>

EncodeWindow::EncodeWindow(Context* c) : context(c)
{
    window = new Fl_Double_Window(600, 200);

    /* Game Executable */
    encodepath = new Fl_Output(10, 30, 400, 30, "Encode file path");
    encodepath->align(FL_ALIGN_TOP_LEFT);
    encodepath->color(FL_LIGHT1);
    encodepath->value(context->gamepath.c_str());

    browseencodepath = new Fl_Button(520, 30, 70, 30, "Browse...");
    browseencodepath->callback(browse_encodepath_cb, this);

    encodepathchooser = new Fl_Native_File_Chooser(Fl_Native_File_Chooser::BROWSE_SAVE_FILE);
    encodepathchooser->title("Choose an encode filename");
    encodepathchooser->preset_file(encodepath->value());
    encodepathchooser->options(Fl_Native_File_Chooser::SAVEAS_CONFIRM);

    containerchoice = new Fl_Choice(420, 30, 80, 30);
    containerchoice->menu(container_items);

    videochoice = new Fl_Choice(10, 100, 100, 30, "Video codec");
    videochoice->align(FL_ALIGN_TOP_LEFT);
    videochoice->menu(video_items);
    videochoice->callback(vcodec_cb, this);

    videobitrate = new Fl_Output(10, 160, 100, 30, "Video bitrate");
    videobitrate->align(FL_ALIGN_TOP_LEFT);
    std::string vbstr = std::to_string(context->config.sc.video_bitrate);
    videobitrate->value(vbstr.c_str());

    audiochoice = new Fl_Choice(150, 100, 100, 30, "Audio codec");
    audiochoice->align(FL_ALIGN_TOP_LEFT);
    audiochoice->menu(audio_items);
    audiochoice->callback(acodec_cb, this);

    audiobitrate = new Fl_Output(150, 160, 100, 30, "Audio bitrate");
    audiobitrate->align(FL_ALIGN_TOP_LEFT);
    std::string abstr = std::to_string(context->config.sc.audio_bitrate);
    audiobitrate->value(abstr.c_str());

    start = new Fl_Button(400, 160, 70, 30, "Ok");
    start->callback(start_cb, this);

    cancel = new Fl_Button(500, 160, 70, 30, "Cancel");
    cancel->callback(cancel_cb, this);

    window->end();
}

Fl_Menu_Item EncodeWindow::container_items[] = {
    {".mkv"},
    {".avi"},
    {nullptr}
};

Fl_Menu_Item EncodeWindow::video_items[] = {
    {"h264"},
    {"ffv1"},
    {"raw"},
    {nullptr}
};

Fl_Menu_Item EncodeWindow::audio_items[] = {
    {"vorbis"},
    {"opus"},
    {"flac"},
    {"pcm"},
    {nullptr}
};

void start_cb(Fl_Widget* w, void* v)
{
    EncodeWindow* ew = (EncodeWindow*) v;

    /* Fill encode filename */
    const char* filename = ew->encodepath->value();
    std::string ext = EncodeWindow::container_items[ew->containerchoice->value()].label();
    ew->context->config.dumpfile = std::string(filename) + ext;

    /* Set video codec and bitrate */
    /* TODO: this switch/case is ugly, we should put them in userdata
     * of each choice item */
    switch(ew->videochoice->value()) {
        case 0:
            ew->context->config.sc.video_codec = AV_CODEC_ID_H264;
            break;
        case 1:
            ew->context->config.sc.video_codec = AV_CODEC_ID_FFV1;
            break;
        case 2:
            ew->context->config.sc.video_codec = AV_CODEC_ID_RAWVIDEO;
            break;
        default:
            ew->context->config.sc.video_codec = AV_CODEC_ID_H264;
    }

    ew->context->config.sc.video_bitrate = std::stoi(ew->videobitrate->value());

    /* Set audio codec and bitrate */
    switch(ew->audiochoice->value()) {
        case 0:
            ew->context->config.sc.audio_codec = AV_CODEC_ID_VORBIS;
            break;
        case 1:
            ew->context->config.sc.audio_codec = AV_CODEC_ID_OPUS;
            break;
        case 2:
            ew->context->config.sc.audio_codec = AV_CODEC_ID_FLAC;
            break;
        case 3:
            ew->context->config.sc.audio_codec = AV_CODEC_ID_PCM_S16LE;
            break;
        default:
            ew->context->config.sc.audio_codec = AV_CODEC_ID_VORBIS;
    }

    ew->context->config.sc.audio_bitrate = std::stoi(ew->audiobitrate->value());
    ew->context->config.sc_modified = true;

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

void vcodec_cb(Fl_Widget* w, void* v)
{
    EncodeWindow* ew = (EncodeWindow*) v;

    /* Enable/disable video bitrate */
    switch(ew->videochoice->value()) {
        case 1:
        case 2:
            ew->videobitrate->deactivate();
            break;
        case 0:
        default:
            ew->videobitrate->activate();
    }
}

void acodec_cb(Fl_Widget* w, void* v)
{
    EncodeWindow* ew = (EncodeWindow*) v;

    /* Enable/disable video bitrate */
    switch(ew->audiochoice->value()) {
        case 2:
        case 3:
            ew->audiobitrate->deactivate();
            break;
        case 0:
        case 1:
        default:
            ew->audiobitrate->activate();
    }

    if (ew->audiochoice->value() == 1) {
        /* For Opus codec, only some frequencies are supported */
        int freq = ew->context->config.sc.audio_frequency;
        if ((freq % 4000) || (freq == 32000)) {
            /* If we are not runnning, change the frequency */
            if (ew->context->status == Context::INACTIVE) {
                fl_alert("The sound frequency %d Hz is not compatible with Opus. It is changed to 48000 Hz", freq);
                ew->context->config.sc.audio_frequency = 48000;
                MainWindow& mw = MainWindow::getInstance();
                Fl_Menu_Item* freq_item = const_cast<Fl_Menu_Item*>(mw.menu_bar->find_item("Sound/Format/48000 Hz"));
                if (freq_item) freq_item->setonly();
            }
            else {
                fl_alert("The sound frequency %d Hz is not compatible with Opus. You must quit the game and then change the frequency", freq);
                ew->audiochoice->value(0);
            }
        }
    }
}
